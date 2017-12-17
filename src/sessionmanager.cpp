/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2009-2014 by Andrea Diamantini <adjam7 at gmail dot com>
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

// Local Includes
#include "application.h"
#include "sessiondialog.h"

#include "rekonqwindow.h"
#include "tabbar.h"

#include "webwindow.h"
#include "webpage.h"

// KDE Includes
#include <KLocalizedString>

// Qt Includes
#include <QDomDocument>
#include <QFile>
#include <QPointer>
#include <QStandardPaths>
#include <QUrl>


// Only used internally
bool readSessionDocument(QDomDocument & document, const QString & sessionFilePath)
{
    QFile sessionFile(sessionFilePath);

    if (!sessionFile.exists())
        return false;

    if (!sessionFile.open(QFile::ReadOnly))
    {
        qDebug() << "Unable to open session file" << sessionFile.fileName();
        return false;
    }

    if (!document.setContent(&sessionFile, false))
    {
        qDebug() << "Unable to parse session file" << sessionFile.fileName();
        return false;
    }

    return true;
}


int loadTabs(RekonqWindow *tw, QDomElement & window, bool useFirstTab, bool justThePinnedOnes = false)
{
    int currentTab = 0;

    for (int tabNo = 0; tabNo < window.elementsByTagName( QL1S("tab") ).length(); tabNo++)
    {
        QDomElement tab = window.elementsByTagName( QL1S("tab") ).at(tabNo).toElement();
        bool tabIsPinned = tab.hasAttribute( QL1S("pinned") );
        qDebug() << "Tab #" << tabNo <<  " is pinned? " << tabIsPinned;

        if (!justThePinnedOnes || tabIsPinned)
        {
            if (tab.hasAttribute( QL1S("currentTab") ))
                currentTab = tabNo;

            QUrl u = QUrl(tab.attribute( QL1S("url") ));

            TabHistory tabHistory;
            tabHistory.title = tab.attribute( QL1S("title") );
            tabHistory.url = tab.attribute( QL1S("url") );
            QDomCDATASection historySection = tab.firstChild().toCDATASection();
            tabHistory.history = QByteArray::fromBase64( historySection.data().toLatin1() );

            if (tabNo == 0 && useFirstTab)
            {
                tw->loadUrl(u, Rekonq::CurrentTab, &tabHistory);
            }
            else
            {
                tw->loadUrl(u, Rekonq::NewTab, &tabHistory);
            }

            if (tabIsPinned)
            {
                tw->tabBar()->setTabData(tabNo, true);
                if (tw->tabBar()->tabButton(tabNo, QTabBar::RightSide))
                    tw->tabBar()->tabButton(tabNo, QTabBar::RightSide)->hide(); // NOTE: this is not good here: where is its proper place?
            }
        }
    }

    return currentTab;
}


bool areTherePinnedTabs(QDomElement & window)
{
    bool b = false;

    for (int tabNo = 0; tabNo < window.elementsByTagName( QL1S("tab") ).length(); tabNo++)
    {
        QDomElement tab = window.elementsByTagName( QL1S("tab") ).at(tabNo).toElement();
        b = tab.hasAttribute( QL1S("pinned") );
        if (b)
            return true;
    }

    return b;
}


// -------------------------------------------------------------------------------------------------


QPointer<SessionManager> SessionManager::s_sessionManager;


SessionManager *SessionManager::self()
{
    if (s_sessionManager.isNull())
    {
        s_sessionManager = new SessionManager(qApp);
    }
    return s_sessionManager.data();
}


// ----------------------------------------------------------------------------------------------


SessionManager::SessionManager(QObject *parent)
    : QObject(parent)
    , m_safe(true)
    , m_isSessionEnabled(false)
{
    m_sessionFilePath = QStandardPaths::writableLocation(QStandardPaths::DataLocation) + QL1S("/session");
}


void SessionManager::saveSession()
{
    if (!m_isSessionEnabled || !m_safe)
        return;

    m_safe = false;

    qDebug() << "SAVING SESSION...";

    QFile sessionFile(m_sessionFilePath);
    if (!sessionFile.open(QFile::WriteOnly | QFile::Truncate))
    {
        qDebug() << "Unable to open session file" << sessionFile.fileName();
        return;
    }
    RekonqWindowList wl = rApp->rekonqWindowList();
    QDomDocument document( QL1S("session") );
    QDomElement session = document.createElement( QL1S("session") );
    document.appendChild(session);

    Q_FOREACH(const QPointer<RekonqWindow> &w, wl)
    {
        if (w.data()->isPrivateBrowsingMode())
            continue;
        
        QDomElement window = document.createElement( QL1S("window") );
        int tabInserted = 0;

        window.setAttribute( QL1S("name"), w.data()->objectName());
        
        TabWidget *tw = w.data()->tabWidget();
        for (signed int tabNo = 0; tabNo < tw->count(); tabNo++)
        {
            QUrl u = tw->webWindow(tabNo)->url();

            tabInserted++;
            QDomElement tab = document.createElement( QL1S("tab") );
            tab.setAttribute( QL1S("title"), tw->webWindow(tabNo)->title()); // redundant, but needed for closedSites()
            // as there's not way to read out the historyData
            tab.setAttribute( QL1S("url"), u.url());
            if (tw->currentIndex() == tabNo)
            {
                tab.setAttribute( QL1S("currentTab"), 1);
            }
            if (tw->tabBar()->tabData(tabNo).toBool()) // pinned tab info
            {
                tab.setAttribute( QL1S("pinned"), 1);
            }
            QByteArray history;
            QDataStream historyStream(&history, QIODevice::ReadWrite);
            historyStream << *(tw->webWindow(tabNo)->page()->history());
            QDomCDATASection historySection = document.createCDATASection( QL1S(history) );

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
    QDomDocument document( QL1S("session") );

    if (!readSessionDocument(document, m_sessionFilePath))
        return false;

    for (int winNo = 0; winNo < document.elementsByTagName( QL1S("window") ).length(); winNo++)
    {
        QDomElement window = document.elementsByTagName( QL1S("window") ).at(winNo).toElement();

        RekonqWindow *tw = rApp->newWindow();

        int currentTab = loadTabs(tw, window, true, false);

        tw->tabWidget()->setCurrentIndex(currentTab);
    }

    return true;
}


bool SessionManager::restoreJustThePinnedTabs()
{
    QDomDocument document( QL1S("session") );

    if (!readSessionDocument(document, m_sessionFilePath))
        return false;

    bool done = false;
    for (int winNo = 0; winNo < document.elementsByTagName( QL1S("window") ).length(); winNo++)
    {
        QDomElement window = document.elementsByTagName( QL1S("window") ).at(winNo).toElement();

        if (!areTherePinnedTabs(window))
            continue;

        done = true;
        RekonqWindow *tw = rApp->newWindow(false);

        int currentTab = loadTabs(tw, window, false, true);

        tw->tabWidget()->setCurrentIndex(currentTab);
    }

    return done;
}


void SessionManager::restoreCrashedSession()
{
    QDomDocument document( QL1S("session") );

    if (!readSessionDocument(document, m_sessionFilePath))
        return;

    for (int winNo = 0; winNo < document.elementsByTagName( QL1S("window") ).length(); winNo++)
    {
        QDomElement window = document.elementsByTagName( QL1S("window") ).at(winNo).toElement();

        RekonqWindow *tw = (winNo == 0)
                        ? rApp->rekonqWindow()
                        : rApp->newWindow();

        QUrl u = tw->currentWebWindow()->url();
        bool useCurrentTab = (u.isEmpty() || u.scheme() == QL1S("rekonq"));
        int currentTab = loadTabs(tw, window, useCurrentTab);

        tw->tabWidget()->setCurrentIndex(currentTab);
    }

    setSessionManagementEnabled(true);
}


bool SessionManager::restoreWindow(RekonqWindow* window)
{
    QDomDocument document( QL1S("session") );

    if (!readSessionDocument(document, m_sessionFilePath))
        return false;

    for (int winNo = 0; winNo < document.elementsByTagName( QL1S("window") ).length(); winNo++)
    {
        QDomElement savedWindowElement = document.elementsByTagName( QL1S("window") ).at(winNo).toElement();

        if (window->objectName() != savedWindowElement.attribute( QL1S("name"), QL1S("") ))
            continue;

        int currentTab = loadTabs(window, savedWindowElement, false);

        window->tabWidget()->setCurrentIndex(currentTab);

        return true;
    }

    return false;
}


QList<TabHistory> SessionManager::closedSitesForWindow(const QString &windowName)
{
    QList<TabHistory> list;
    QDomDocument document( QL1S("session") );

    if (!readSessionDocument(document, m_sessionFilePath))
        return list;

    for (int winNo = 0; winNo < document.elementsByTagName( QL1S("tab") ).length(); winNo++)
    {
        QDomElement windowElement = document.elementsByTagName( QL1S("window") ).at(winNo).toElement();

        if (windowName != windowElement.attribute( QL1S("name"), QL1S("")))
            continue;
        
        for (int tabNo = 0; tabNo < windowElement.elementsByTagName( QL1S("tab") ).length(); tabNo++)
        {
            QDomElement tab = windowElement.elementsByTagName( QL1S("tab") ).at(tabNo).toElement();

            TabHistory tabHistory;

            tabHistory.title = tab.attribute( QL1S("title") );
            tabHistory.url = tab.attribute( QL1S("url") );

            QDomCDATASection historySection = tab.firstChild().toCDATASection();
            tabHistory.history = QByteArray::fromBase64(historySection.data().toLatin1());

            list << tabHistory;
        }
        
        return list;
    }

    return list;
}


// -------------------------------------------------------------------------------------------------------


bool SessionManager::saveYourSession(int index)
{
    qDebug() << "SAVING YOUR OWN SESSION...";
    
    const QString & sessionPath = QStandardPaths::writableLocation(QStandardPaths::DataLocation) + QL1S("/usersessions/");
    const QString & sessionName = QL1S("ses") + QString::number(index);
    
    QFile sessionFile(sessionPath + sessionName);
    if (!sessionFile.open(QFile::WriteOnly | QFile::Truncate))
    {
        qDebug() << "Unable to open session file" << sessionFile.fileName();
        return false;
    }
    
    RekonqWindowList wl = rApp->rekonqWindowList();
    QDomDocument document( QL1S("session") );
    QDomElement session = document.createElement( QL1S("session") );
    document.appendChild(session);

    Q_FOREACH(const QPointer<RekonqWindow> &w, wl)
    {
        if (w.data()->isPrivateBrowsingMode())
            continue;
        
        QDomElement window = document.createElement( QL1S("window") );
        int tabInserted = 0;

        window.setAttribute( QL1S("name"), w.data()->objectName());
        
        TabWidget *tw = w.data()->tabWidget();
        for (signed int tabNo = 0; tabNo < tw->count(); tabNo++)
        {
            QUrl u = tw->webWindow(tabNo)->url();

            tabInserted++;
            QDomElement tab = document.createElement( QL1S("tab") );
            tab.setAttribute( QL1S("title"), tw->webWindow(tabNo)->title()); // redundant, but needed for closedSites()
            // as there's not way to read out the historyData
            tab.setAttribute( QL1S("url"), u.url());
            if (tw->currentIndex() == tabNo)
            {
                tab.setAttribute( QL1S("currentTab"), 1);
            }
            if (tw->tabBar()->tabData(tabNo).toBool()) // pinned tab info
            {
                tab.setAttribute( QL1S("pinned"), 1);
            }
            QByteArray history;
            QDataStream historyStream(&history, QIODevice::ReadWrite);
            historyStream << *(tw->webWindow(tabNo)->page()->history());
            QDomCDATASection historySection = document.createCDATASection( QL1S(history) );

            tab.appendChild(historySection);
            window.appendChild(tab);
        }
        
        if (tabInserted > 0)
            session.appendChild(window);
    }

    QTextStream TextStream(&sessionFile);
    document.save(TextStream, 2);
    sessionFile.close();

    return true;
}

    
bool SessionManager::restoreYourSession(int index)
{
    const QString & sessionPath = QStandardPaths::writableLocation(QStandardPaths::DataLocation) + QL1S("/usersessions/");
    const QString & sessionName = QL1S("ses") + QString::number(index);
    
    QDomDocument document( QL1S("session") );

    if (!readSessionDocument(document,sessionPath + sessionName))
        return false;

    // trace the windows to delete
    RekonqWindowList wList = rApp->rekonqWindowList();
    
    for (int winNo = 0; winNo < document.elementsByTagName( QL1S("window") ).length(); winNo++)
    {
        QDomElement window = document.elementsByTagName( QL1S("window") ).at(winNo).toElement();

        RekonqWindow *tw = rApp->newWindow();

        int currentTab = loadTabs(tw, window, true, false);

        tw->tabWidget()->setCurrentIndex(currentTab);
    }
    
    Q_FOREACH(const QPointer<RekonqWindow> &w, wList)
    {
        if (!w.isNull())
            w.data()->close();
    }
    
    return true;
}


void SessionManager::manageSessions()
{
    qDebug() << "OK, manage session..";
    
    QPointer<SessionDialog> dialog = new SessionDialog();
    auto result = dialog->exec();
    
    dialog->deleteLater();

    if (result == QDialog::Rejected && rApp->rekonqWindowList().isEmpty()) {
        qDebug() << "session dialog rejected without any windows open: quitting";
        setSessionManagementEnabled(false);
        rApp->quit();
    }
}
