/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2010 by Nikhil Marathe <nsm.nikhil@gmail.com>
* Copyright (C) 2012 by Andrea Diamantini <adjam7 at gmail dot com>
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
#include "api_tabs.h"
#include "api_tabs.moc"

// Auto Includes
#include "rekonq.h"

// Local Includes
#include "application.h"
#include "rekonqwindow.h"
#include "tabwidget.h"
#include "webwindow.h"


Tabs::Tabs(QObject *parent)
    : QObject(parent)
{
    setObjectName("tabs");    
}


WebWindow *Tabs::tabFor(TabId tabId)
{
    RekonqWindowList wList = rApp->rekonqWindowList();
    Q_FOREACH(const QWeakPointer<RekonqWindow> ptr, wList)
    {
        RekonqWindow *w = ptr.data();
        int tabsCount = w->tabWidget()->count();
        for(int i = 0; i < tabsCount; ++i)
        {
            WebWindow *tab = w->tabWidget()->webWindow(i);
            if (tab->id() == tabId)
                return tab;
        }        
    }
    
    return 0;
}


RekonqWindow *Tabs::windowFor(int winId)
{
    // with negative numbers, return current window
    if (winId < 0) 
    {
        return Application::instance()->rekonqWindow();
    }
 
    WindowId wId = qAbs(winId);
    
    RekonqWindowList wList = rApp->rekonqWindowList();
    Q_FOREACH(const QWeakPointer<RekonqWindow> ptr, wList)
    {
        RekonqWindow *w = ptr.data();
        if (w->id() == wId)
            return w;
    }
    
    return 0;
}


// -------------------------- CHROME.TABS API ----------------------------


QVariantMap Tabs::get(TabId tabId /*, function callback */ )
{
    WebWindow *theTab = tabFor(tabId);
    
    if (!theTab)
        return QVariantMap();

    QVariantMap map;

    map["id"] = theTab->id();

    TabWidget *view = qobject_cast<TabWidget*>(theTab->parent()->parent());
    Q_ASSERT(view);
    map["index"] = view->indexOf(theTab);

    RekonqWindow *window = qobject_cast<RekonqWindow*>(view->parent()->parent());
    Q_ASSERT(window);
    map["windowId"] = window->id();

    map["selected"] = (view->currentWebWindow() == theTab);

    map["url"] = theTab->url().prettyUrl();

    map["title"] = theTab->title();

    // TODO
    // map["favIconUrl"]

    map["status"] = theTab->isLoading() ? "loading" : "complete";

    map["incognito"] = theTab->isPrivateBrowsing();

    return map;
}


QVariantMap Tabs::getCurrent(/* function callback */ )
{
    RekonqWindow *window = rApp->rekonqWindow();
    if (!window)
        return QVariantMap();
    
    return get(window->currentWebWindow()->id());
}


// Chrome return value type is "Runtime.Port"
void Tabs::connect(TabId tabId, QVariantMap connectInfo)
{
    kDebug() << "UNIMPLEMENTED";
    Q_UNUSED(tabId);
    Q_UNUSED(connectInfo);
}


QVariantMap Tabs::sendMessage(TabId tabId, QVariant message /*, function responseCallback */)
{
    kDebug() << "UNIMPLEMENTED";
    Q_UNUSED(tabId);
    Q_UNUSED(message);
    
    return QVariantMap();
}


QVariantMap Tabs::create(QVariantMap createProperties /*, function callback */ )
{
    RekonqWindow *window = Application::instance()->rekonqWindow();
    QUrl urlToLoad;
    
    if (createProperties.contains("windowId")) 
    {
        window = windowFor(createProperties["windowId"].toInt());
        if (!window)
            return QVariantMap();
    }

    WebWindow *tab = window->tabWidget()->newTab();

    if (createProperties.contains("index")) 
    {
        int index = qBound(0, createProperties["index"].toInt(), window->tabWidget()->count());
        int original = window->tabWidget()->indexOf(tab);
        window->tabWidget()->moveTab(original, index);
    }

    if (createProperties.contains("selected") && createProperties["selected"].toBool())
    {
        window->tabWidget()->setCurrentWidget(tab);    
    }
    
    if (createProperties.contains("url")) 
    {
        QUrl urlToLoad = QUrl::fromUserInput( createProperties["url"].toString() );
        tab->load(urlToLoad);
    }

    return get(tab->id());
}


QVariantMap Tabs::duplicate(TabId tabId /*, function callback */ )
{
    kDebug() << "UNIMPLEMENTED";
    Q_UNUSED(tabId);

    return QVariantMap();
}


QVariantMap Tabs::query(QVariantMap queryInfo /*, function callback */)
{
    kDebug() << "UNIMPLEMENTED";
    Q_UNUSED(queryInfo);

    return QVariantMap();
}


QVariantMap Tabs::highlight(QVariantMap highlightInfo /*, function callback */)
{
    kDebug() << "UNIMPLEMENTED";
    Q_UNUSED(highlightInfo);

    return QVariantMap();
}


QVariantMap Tabs::update(TabId tabId, QVariantMap updateProperties /*, function callback */)
{
    WebWindow *tab = tabFor(tabId);
    if (!tab)
        return QVariantMap();

    if (updateProperties.contains("url")) 
    {
        KUrl url = updateProperties["url"].toString();
        
        // TODO what's the best way to do this?
    }

    if (updateProperties.contains("selected") && updateProperties["selected"].toBool()) 
    {
        TabWidget *view = qobject_cast<TabWidget*>(tab->parent()->parent());
        if (view)
            view->setCurrentWidget(tab);
    }

    return get(tabId);
}


QVariantMap Tabs::move(TabId tabId, QVariantMap moveProperties /*, function callback */)
{
    WebWindow *tab = tabFor(tabId);
    if (!tab)
        return QVariantMap();

    RekonqWindow *destWindow = windowFor(-1);
    if (moveProperties.contains("windowId")) 
    {
        destWindow = windowFor(moveProperties["windowId"].toInt());
        if (!destWindow)
            destWindow = Application::instance()->newWindow(false);
    }

    int index = -1;
    if (moveProperties.contains("index")) 
    {
        index = moveProperties["index"].toInt();
    }

    index = qBound(0, index, destWindow->tabWidget()->count());

//     TabWidget *view = qobject_cast<TabWidget*>(tab->parent()->parent());
//     Q_ASSERT(view);
// 
//     view->detachTab(view->indexOf(tab), destWindow);
// 
//     TabWidget *newView = qobject_cast<TabWidget*>(tab->parent()->parent());
//     newView->moveTab(newView->indexOf(tab), index);
//     newView->setCurrentIndex(index);
//     
//     QVariantMap jstab = get(tabId);
//     return jstab;
    
    return QVariantMap();
}


QVariantMap Tabs::reload(TabId tabId, QVariantMap reloadProperties /*, function callback */)
{
    kDebug() << "UNIMPLEMENTED";
    Q_UNUSED(tabId);
    Q_UNUSED(reloadProperties);

    return QVariantMap();
}

void Tabs::remove(TabId tabId /*, function callback */)
{
    WebWindow *tab = tabFor(tabId);
    if (!tab)
        return;

//     TabWidget *view = qobject_cast<TabWidget*>(tab->parent()->parent());
//     Q_ASSERT(view);
//     view->closeTab(view->indexOf(tab));
}


void Tabs::detectLanguage(TabId tabId /*, function callback */)
{
    kDebug() << "UNIMPLEMENTED";
    Q_UNUSED(tabId);

}


void Tabs::captureVisibleTab(TabId windowId, QVariantMap options /*, function callback */)
{
    kDebug() << "UNIMPLEMENTED";
    Q_UNUSED(windowId);
    Q_UNUSED(options);
}


void Tabs::executeScript(TabId tabId, QVariantMap details /*, function callback */)
{
    kDebug() << "UNIMPLEMENTED";
    Q_UNUSED(tabId);
    Q_UNUSED(details);
    
}


void Tabs::insertCSS(TabId tabId, QVariantMap details /*, function callback*/)
{
    kDebug() << "UNIMPLEMENTED";
    Q_UNUSED(tabId);
    Q_UNUSED(details);

}
