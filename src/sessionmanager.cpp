/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2009 by Andrea Diamantini <adjam7 at gmail dot com>
* Copyright (C) 2009 by Yoram Bar-Haim <<yoram.b at zend dot com>
* Copyright (C) 2009 by Lionel Chauvin <megabigbug@yahoo.fr>
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

// KDE Includes
#include <KStandardDirs>
#include <KDebug>

// Qt Includes
#include <QFile>
#include <QDataStream>


SessionManager::SessionManager(QObject *parent)
    : QObject(parent)
{
    m_sessionFilePath = KStandardDirs::locateLocal("appdata" , "session");
}


SessionManager::~SessionManager()
{
}


void SessionManager::saveSession()
{
    QFile sessionFile(m_sessionFilePath);
    if (!sessionFile.open(QFile::WriteOnly | QFile::Truncate))
    {
        kWarning() << "Unable to open session file" << sessionFile.fileName();
        return;
    }
    QDataStream out(&sessionFile);
    MainWindowList wl = Application::instance()->mainWindowList();
    Q_FOREACH(QPointer<MainWindow> w, wl)
    {
        out << QByteArray("window");
        MainView *mv = w->mainView();
        for (int i = 0 ; i < mv->count() ; i++)
        {
            out << mv->webView(i)->url().toEncoded();
        }
    }
    return;
}


bool SessionManager::restoreSession()
{
    QFile sessionFile(m_sessionFilePath);
    if (!sessionFile.exists())
        return false;
    if (!sessionFile.open(QFile::ReadOnly))
    {
        kWarning() << "Unable to open session file" << sessionFile.fileName();
        return false;
    }

    QDataStream in(&sessionFile);
    int i = 0;
    while (!sessionFile.atEnd())
    {
        ++i;
        QByteArray data;
        in >> data;
        if(data == QByteArray("window"))
        {
            Application::instance()->newMainWindow();
        }
        else
        {
            Application::instance()->loadUrl(data,Rekonq::NewCurrentTab);
        }
    }

    return true;
}
