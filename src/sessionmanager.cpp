/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2009-2012 by Andrea Diamantini <adjam7 at gmail dot com>
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

// Only used internally
bool readSessionDocument(QDomDocument & document, const QString & sessionFilePath)
{
    QFile sessionFile(sessionFilePath);

    if (!sessionFile.exists())
        return false;

    if (!sessionFile.open(QFile::ReadOnly))
    {
        kDebug() << "Unable to open session file" << sessionFile.fileName();
        return false;
    }

    if (!document.setContent(&sessionFile, false))
    {
        kDebug() << "Unable to parse session file" << sessionFile.fileName();
        return false;
    }

    return true;
}

SessionManager::SessionManager(QObject *parent)
    : QObject(parent)
    , m_safe(true)
    , m_isSessionEnabled(false)
{
    m_sessionFilePath = KStandardDirs::locateLocal("appdata" , "session");
}


void SessionManager::saveSession()
{
    if (!m_isSessionEnabled || !m_safe)
        return;

    m_safe = false;

    kDebug() << "SAVING SESSION...";

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
        int tabInserted = 0;

        window.setAttribute("name", w.data()->objectName());

        for (signed int tabNo = 0; tabNo < mv->count(); tabNo++)
        {
            // IGNORE about urls
            KUrl u = mv->webTab(tabNo)->url();
            if (u.protocol() == QL1S("about"))
                continue;

            tabInserted++;
            QDomElement tab = document.createElement("tab");
            tab.setAttribute("title", mv->webTab(tabNo)->view()->title()); // redundant, but needed for closedSites()
            // as there's not way to read out the historyData
            tab.setAttribute("url", u.url());
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
        if (tabInserted > 0)
            session.appendChild(window);
    }

    QTextStream TextStream(&sessionFile);
    document.save(TextStream, 2);
    sessionFile.close();

    m_safe = true;
    return;
}


bool SessionManager::restoreSessionFromScratch()
{
    QDomDocument document("session");

    if (!readSessionDocument(document, m_sessionFilePath))
        return false;

    for (unsigned int winNo = 0; winNo < document.elementsByTagName("window").length(); winNo++)
    {
        QDomElement window = document.elementsByTagName("window").at(winNo).toElement();
        int currentTab = 0;

        MainView *mv = rApp->newMainWindow(false)->mainView();

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

            // IGNORE "eventual" about urls
            KUrl u = KUrl(tab.attribute("url"));
            if (u.protocol() == QL1S("about"))
                continue;

            // This is needed for particular URLs, eg pdfs
            view->load(u);
        }

        mv->tabBar()->setCurrentIndex(currentTab);
    }

    return true;
}


void SessionManager::restoreCrashedSession()
{
    QDomDocument document("session");

    if (!readSessionDocument(document, m_sessionFilePath))
        return;

    for (unsigned int winNo = 0; winNo < document.elementsByTagName("window").length(); winNo++)
    {
        QDomElement window = document.elementsByTagName("window").at(winNo).toElement();
        int currentTab = 0;

        MainView *mv = (winNo == 0) ? rApp->mainWindow()->mainView() : rApp->newMainWindow()->mainView();

        for (unsigned int tabNo = 0; tabNo < window.elementsByTagName("tab").length(); tabNo++)
        {
            QDomElement tab = window.elementsByTagName("tab").at(tabNo).toElement();
            if (tab.hasAttribute("currentTab"))
                currentTab = tabNo;

            WebView *view = (tabNo == 0) ? mv->webTab(0)->view() : mv->newWebTab()->view();

            QDomCDATASection historySection = tab.firstChild().toCDATASection();
            QByteArray history = QByteArray::fromBase64(historySection.data().toAscii());

            QDataStream readingStream(&history, QIODevice::ReadOnly);
            readingStream >> *(view->history());

            // Get sure about urls and/or pdf are loaded
            KUrl u = KUrl(tab.attribute("url"));
            view->load(u);
        }
        mv->tabBar()->setCurrentIndex(currentTab);
    }

    setSessionManagementEnabled(true);
}


int SessionManager::restoreSavedSession()
{
    QDomDocument document("session");

    if (!readSessionDocument(document, m_sessionFilePath))
        return 0;

    unsigned int winNo;

    for (winNo = 0; winNo < document.elementsByTagName("window").length(); winNo++)
    {
        QDomElement window = document.elementsByTagName("window").at(winNo).toElement();
        int currentTab = 0;

        MainView *mv = rApp->newMainWindow()->mainView();

        for (unsigned int tabNo = 0; tabNo < window.elementsByTagName("tab").length(); tabNo++)
        {
            QDomElement tab = window.elementsByTagName("tab").at(tabNo).toElement();
            if (tab.hasAttribute("currentTab"))
                currentTab = tabNo;

            WebView *view = (tabNo == 0) ? mv->webTab(0)->view() : mv->newWebTab()->view();

            QDomCDATASection historySection = tab.firstChild().toCDATASection();
            QByteArray history = QByteArray::fromBase64(historySection.data().toAscii());

            QDataStream readingStream(&history, QIODevice::ReadOnly);
            readingStream >> *(view->history());

            // Get sure about urls and/or pdfs are loaded
            KUrl u = KUrl(tab.attribute("url"));
            view->load(u);
        }
        mv->tabBar()->setCurrentIndex(currentTab);
    }

    return winNo;
}

bool SessionManager::restoreMainWindow(MainWindow* window)
{
    QDomDocument document("session");

    if (!readSessionDocument(document, m_sessionFilePath))
        return false;

    unsigned int winNo;

    for (winNo = 0; winNo < document.elementsByTagName("window").length(); winNo++)
    {
        QDomElement savedWindowElement = document.elementsByTagName("window").at(winNo).toElement();
        int currentTab = 0;

        if (window->objectName() != savedWindowElement.attribute("name", ""))
            continue;

        MainView *mv = window->mainView();

        for (unsigned int tabNo = 0; tabNo < savedWindowElement.elementsByTagName("tab").length(); tabNo++)
        {
            QDomElement tab = savedWindowElement.elementsByTagName("tab").at(tabNo).toElement();
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

        return true;
    }

    return false;
}

QList<TabHistory> SessionManager::closedSites()
{
    QList<TabHistory> list;
    QDomDocument document("session");

    if (!readSessionDocument(document, m_sessionFilePath))
        return list;

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
