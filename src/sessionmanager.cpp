/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2009-2011 by Andrea Diamantini <adjam7 at gmail dot com>
* Copyright (C) 2009 by Yoram Bar-Haim <<yoram.b at zend dot com>
* Copyright (C) 2009-2011 by Lionel Chauvin <megabigbug@yahoo.fr>
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
#include "mainview.h"
#include "mainwindow.h"
#include "tabbar.h"
#include "webtab.h"

// KDE Includes
#include <KStandardDirs>

// Qt Includes
#include <QtCore/QFile>


SessionManager::SessionManager(QObject *parent)
    : QObject(parent)
    , m_safe(false)
{
    m_sessionFilePath = KStandardDirs::locateLocal("appdata" , "session");
}


void SessionManager::saveSession()
{
    if (!m_safe || QWebSettings::globalSettings()->testAttribute(QWebSettings::PrivateBrowsingEnabled))
        return;
    m_safe = false;


    QFile sessionFile(m_sessionFilePath);
    if (!sessionFile.open(QFile::WriteOnly | QFile::Truncate))
    {
        kDebug() << "Unable to open session file" << sessionFile.fileName();
        return;
    }
    MainWindowList wl = rApp->mainWindowList();
    QDomDocument document("session");
    QDomElement session = document.createElement("session");
    document.appendChild(session);

    Q_FOREACH(const QWeakPointer<MainWindow> &w, wl)
    {
        MainView *mv = w.data()->mainView();
        QDomElement window = document.createElement("window");
        for (signed int tabNo = 0; tabNo < mv->count(); tabNo++)
        {
            QDomElement tab = document.createElement("tab");
            tab.setAttribute("title", mv->webTab(tabNo)->view()->title()); // redundant, but needed for closedSites()
            // as there's not way to read out the historyData
            tab.setAttribute("url", mv->webTab(tabNo)->url().url()); // Use WebTab's instead of WebView's url() to fix about links
            if (mv->tabBar()->currentIndex() == tabNo)
            {
                tab.setAttribute("currentTab", 1);
            }
            QByteArray history;
            QDataStream historyStream(&history, QIODevice::ReadWrite);
            historyStream << *(mv->webTab(tabNo)->view()->history());
            QDomCDATASection historySection = document.createCDATASection(history.toBase64());

            tab.appendChild(historySection);
            window.appendChild(tab);
        }
        session.appendChild(window);
    }
    QTextStream TextStream(&sessionFile);
    document.save(TextStream, 2);
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

    bool windowAlreadyOpen = rApp->mainWindowList().count();
    MainWindowList wl;

    QDomDocument document("session");
    if (!document.setContent(&sessionFile, false))
    {
        kDebug() << "Unable to parse session file" << sessionFile.fileName();
        return false;
    }

    for (unsigned int winNo = 0; winNo < document.elementsByTagName("window").length(); winNo++)
    {
        QDomElement window = document.elementsByTagName("window").at(winNo).toElement();
        int currentTab = 0;

        if (windowAlreadyOpen)
            windowAlreadyOpen = false;
        else
            rApp->newMainWindow(false);

        wl = rApp->mainWindowList(); //get the latest windowlist
        if (wl.count() == 0)
            continue;
        MainView *mv = wl.at(0).data()->mainView(); //last mainwindow created will be first one in mainwindow list

        for (unsigned int tabNo = 0; tabNo < window.elementsByTagName("tab").length(); tabNo++)
        {
            QDomElement tab = window.elementsByTagName("tab").at(tabNo).toElement();
            if (tab.hasAttribute("currentTab"))
                currentTab = tabNo;

            WebView *view = mv->newWebTab()->view();

            QDomCDATASection historySection = tab.firstChild().toCDATASection();
            QByteArray history = QByteArray::fromBase64(historySection.data().toAscii());

            QDataStream readingStream(&history, QIODevice::ReadOnly);
            readingStream >> *(view->history());

            // Get sure about urls are loaded
            KUrl u = KUrl(tab.attribute("url"));
            if (u.protocol() == QL1S("about"))
                view->load(u);
        }

        mv->tabBar()->setCurrentIndex(currentTab);
    }

    return true;
}


QList<TabHistory> SessionManager::closedSites()
{
    QList<TabHistory> list;

    QFile sessionFile(m_sessionFilePath);
    if (!sessionFile.exists())
        return list;
    if (!sessionFile.open(QFile::ReadOnly))
    {
        kDebug() << "Unable to open session file" << sessionFile.fileName();
        return list;
    }

    QDomDocument document("session");
    if (!document.setContent(&sessionFile, false))
    {
        kDebug() << "Unable to parse session file" << sessionFile.fileName();
        return list;
    }

    for (unsigned int tabNo = 0; tabNo < document.elementsByTagName("tab").length(); tabNo++)
    {
        QDomElement tab = document.elementsByTagName("tab").at(tabNo).toElement();

        TabHistory tabHistory;

        tabHistory.title = tab.attribute("title");
        tabHistory.url = tab.attribute("url");

        QDomCDATASection historySection = tab.firstChild().toCDATASection();
        tabHistory.history = QByteArray::fromBase64(historySection.data().toAscii());

        list << tabHistory;
    }

    return list;
}
