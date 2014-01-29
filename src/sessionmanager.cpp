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
#include "tabhistory.h"
#include "sessionwidget.h"

#include "rekonqwindow.h"
#include "tabbar.h"

#include "webwindow.h"
#include "webpage.h"

// KDE Includes
#include <KDialog>
#include <KPushButton>

// Qt Includes
#include <QStandardPaths>
#include <QFile>
#include <QDomDocument>
#include <QPointer>
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

    for (unsigned int tabNo = 0; tabNo < window.elementsByTagName("tab").length(); tabNo++)
    {
        QDomElement tab = window.elementsByTagName("tab").at(tabNo).toElement();
        bool tabIsPinned = tab.hasAttribute("pinned");
        qDebug() << "Tab #" << tabNo <<  " is pinned? " << tabIsPinned;

        if (!justThePinnedOnes || tabIsPinned)
        {
            if (tab.hasAttribute("currentTab"))
                currentTab = tabNo;

            QUrl u = QUrl(tab.attribute("url"));

            TabHistory tabHistory;
            tabHistory.title = tab.attribute("title");
            tabHistory.url = tab.attribute("url");
            QDomCDATASection historySection = tab.firstChild().toCDATASection();
            tabHistory.history = QByteArray::fromBase64(historySection.data().toAscii());

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

    for (unsigned int tabNo = 0; tabNo < window.elementsByTagName("tab").length(); tabNo++)
    {
        QDomElement tab = window.elementsByTagName("tab").at(tabNo).toElement();
        b = tab.hasAttribute("pinned");
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
    QDomDocument document("session");
    QDomElement session = document.createElement("session");
    document.appendChild(session);

    Q_FOREACH(const QPointer<RekonqWindow> &w, wl)
    {
        if (w.data()->isPrivateBrowsingMode())
            continue;
        
        QDomElement window = document.createElement("window");
        int tabInserted = 0;

        window.setAttribute("name", w.data()->objectName());
        
        TabWidget *tw = w.data()->tabWidget();
        for (signed int tabNo = 0; tabNo < tw->count(); tabNo++)
        {
            QUrl u = tw->webWindow(tabNo)->url();

            tabInserted++;
            QDomElement tab = document.createElement("tab");
            tab.setAttribute("title", tw->webWindow(tabNo)->title()); // redundant, but needed for closedSites()
            // as there's not way to read out the historyData
            tab.setAttribute("url", u.url());
            if (tw->currentIndex() == tabNo)
            {
                tab.setAttribute("currentTab", 1);
            }
            if (tw->tabBar()->tabData(tabNo).toBool()) // pinned tab info
            {
                tab.setAttribute("pinned", 1);
            }
            QByteArray history;
            QDataStream historyStream(&history, QIODevice::ReadWrite);
            historyStream << *(tw->webWindow(tabNo)->page()->history());
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

        RekonqWindow *tw = rApp->newWindow();

        int currentTab = loadTabs(tw, window, true, false);

        tw->tabWidget()->setCurrentIndex(currentTab);
    }

    return true;
}


bool SessionManager::restoreJustThePinnedTabs()
{
    QDomDocument document("session");

    if (!readSessionDocument(document, m_sessionFilePath))
        return false;

    bool done = false;
    for (unsigned int winNo = 0; winNo < document.elementsByTagName("window").length(); winNo++)
    {
        QDomElement window = document.elementsByTagName("window").at(winNo).toElement();

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
    QDomDocument document("session");

    if (!readSessionDocument(document, m_sessionFilePath))
        return;

    for (unsigned int winNo = 0; winNo < document.elementsByTagName("window").length(); winNo++)
    {
        QDomElement window = document.elementsByTagName("window").at(winNo).toElement();

        RekonqWindow *tw = (winNo == 0)
                        ? rApp->rekonqWindow()
                        : rApp->newWindow();

        QUrl u = tw->currentWebWindow()->url();
        bool useCurrentTab = (u.isEmpty() || u.protocol() == QL1S("rekonq"));
        int currentTab = loadTabs(tw, window, useCurrentTab);

        tw->tabWidget()->setCurrentIndex(currentTab);
    }

    setSessionManagementEnabled(true);
}


bool SessionManager::restoreWindow(RekonqWindow* window)
{
    QDomDocument document("session");

    if (!readSessionDocument(document, m_sessionFilePath))
        return false;

    unsigned int winNo;

    for (winNo = 0; winNo < document.elementsByTagName("window").length(); winNo++)
    {
        QDomElement savedWindowElement = document.elementsByTagName("window").at(winNo).toElement();

        if (window->objectName() != savedWindowElement.attribute("name", ""))
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
    QDomDocument document("session");

    if (!readSessionDocument(document, m_sessionFilePath))
        return list;

    for (unsigned int winNo = 0; winNo < document.elementsByTagName("tab").length(); winNo++)
    {
        QDomElement windowElement = document.elementsByTagName("window").at(winNo).toElement();

        if (windowName != windowElement.attribute("name", ""))
            continue;
        
        for (unsigned int tabNo = 0; tabNo < windowElement.elementsByTagName("tab").length(); tabNo++)
        {
            QDomElement tab = windowElement.elementsByTagName("tab").at(tabNo).toElement();

            TabHistory tabHistory;

            tabHistory.title = tab.attribute("title");
            tabHistory.url = tab.attribute("url");

            QDomCDATASection historySection = tab.firstChild().toCDATASection();
            tabHistory.history = QByteArray::fromBase64(historySection.data().toAscii());

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
    QDomDocument document("session");
    QDomElement session = document.createElement("session");
    document.appendChild(session);

    Q_FOREACH(const QPointer<RekonqWindow> &w, wl)
    {
        if (w.data()->isPrivateBrowsingMode())
            continue;
        
        QDomElement window = document.createElement("window");
        int tabInserted = 0;

        window.setAttribute("name", w.data()->objectName());
        
        TabWidget *tw = w.data()->tabWidget();
        for (signed int tabNo = 0; tabNo < tw->count(); tabNo++)
        {
            QUrl u = tw->webWindow(tabNo)->url();

            tabInserted++;
            QDomElement tab = document.createElement("tab");
            tab.setAttribute("title", tw->webWindow(tabNo)->title()); // redundant, but needed for closedSites()
            // as there's not way to read out the historyData
            tab.setAttribute("url", u.url());
            if (tw->currentIndex() == tabNo)
            {
                tab.setAttribute("currentTab", 1);
            }
            if (tw->tabBar()->tabData(tabNo).toBool()) // pinned tab info
            {
                tab.setAttribute("pinned", 1);
            }
            QByteArray history;
            QDataStream historyStream(&history, QIODevice::ReadWrite);
            historyStream << *(tw->webWindow(tabNo)->page()->history());
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

    return true;
}

    
bool SessionManager::restoreYourSession(int index)
{
    const QString & sessionPath = QStandardPaths::writableLocation(QStandardPaths::DataLocation) + QL1S("/usersessions/");
    const QString & sessionName = QL1S("ses") + QString::number(index);
    
    QDomDocument document("session");

    if (!readSessionDocument(document,sessionPath + sessionName))
        return false;

    // trace the windows to delete
    RekonqWindowList wList = rApp->rekonqWindowList();
    
    for (unsigned int winNo = 0; winNo < document.elementsByTagName("window").length(); winNo++)
    {
        QDomElement window = document.elementsByTagName("window").at(winNo).toElement();

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
    qDebug() << "OK ,manage session..";
    
    QPointer<KDialog> dialog = new KDialog();
    dialog->setCaption(i18nc("@title:window", "Manage Session"));
    dialog->setButtons(KDialog::Ok | KDialog::Close);

    dialog->button(KDialog::Ok)->setIcon(QIcon::fromTheme("system-run"));
    dialog->button(KDialog::Ok)->setText(i18n("Load"));

    SessionWidget widg;
    dialog->setMainWidget(&widg);
    
    connect(dialog, SIGNAL(okClicked()), &widg, SLOT(loadSession()));
    dialog->exec();
}
