/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2009-2010 by Andrea Diamantini <adjam7 at gmail dot com>
* Copyright (C) 2009 by Yoram Bar-Haim <<yoram.b at zend dot com>
* Copyright (C) 2009-2010 by Lionel Chauvin <megabigbug@yahoo.fr>
*
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License as
* published by the Free Software Foundation; either version 2 of
* the License or (at your option) version 3 or any later version
* accepted by the membership of KDE e.V. (or its successor approved
* by the membership of KDE e.V.), which shall act as a proxy
* defined in Section 14 of version 3 of the license.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
* ============================================================ */


// Self Includes
#include "sessionmanager.h"
#include "sessionmanager.moc"

// Local Includes
#include "application.h"
#include "mainwindow.h"
#include "mainview.h"
#include "webtab.h"

// KDE Includes
#include <KStandardDirs>

// Qt Includes
#include <QFile>
#include <QTextStream>


SessionManager::SessionManager(QObject *parent)
        : QObject(parent)
        , m_safe(true)
{
    m_sessionFilePath = KStandardDirs::locateLocal("appdata" , "session");
}


SessionManager::~SessionManager()
{
}


void SessionManager::saveSession()
{
    if (!m_safe)
        return;
    m_safe = false;
    QFile sessionFile(m_sessionFilePath);
    if (!sessionFile.open(QFile::WriteOnly | QFile::Truncate))
    {
        kDebug() << "Unable to open session file" << sessionFile.fileName();
        return;
    }
    QTextStream out(&sessionFile);
    MainWindowList wl = Application::instance()->mainWindowList();
    Q_FOREACH(QWeakPointer<MainWindow> w, wl)
    {
        out << "window\n";
        MainView *mv = w.data()->mainView();
        for (int i = 0 ; i < mv->count() ; i++)
        {
            out << mv->webTab(i)->url().toEncoded() << "\n";
        }
    }
    sessionFile.close();
    m_safe = true;
    return;
}


bool SessionManager::restoreSession()
{
    QFile sessionFile(m_sessionFilePath);
    if (!sessionFile.exists())
        return false;
    if (!sessionFile.open(QFile::ReadOnly))
    {
        kDebug() << "Unable to open session file" << sessionFile.fileName();
        return false;
    }

    QTextStream in(&sessionFile);
    QString line;
    do
    {
        line = in.readLine();
        if (line == QString("window"))
        {
            line = in.readLine();
            kDebug() << "New Window line: " << line;
            Application::instance()->loadUrl(line, Rekonq::NewWindow);
        }
        else
        {
            kDebug() << "New Current Tab line: " << line;
            Application::instance()->loadUrl(line, Rekonq::NewCurrentTab);
        }
    }
    while (!line.isEmpty());

    return true;
}
