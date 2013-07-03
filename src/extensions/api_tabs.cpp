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

#include "webview.h"


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


// Retrieves details about the specified tab.
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


// Gets the tab that this script call is being made from. 
// May be undefined if called from a non-tab context (for example: a background page or popup view).
QVariantMap Tabs::getCurrent(/* function callback */ )
{
    RekonqWindow *window = rApp->rekonqWindow();
    if (!window)
        return QVariantMap();
    
    return get(window->currentWebWindow()->id());
}


// Connects to the content script(s) in the specified tab. 
// The runtime.onConnect event is fired in each content script running in the specified tab for the current extension. 
// For more details, see Content Script Messaging.
// Chrome return value type is "Runtime.Port"
void Tabs::connect(TabId tabId, QVariantMap connectInfo)
{
    kDebug() << "UNIMPLEMENTED";
    Q_UNUSED(tabId);
    Q_UNUSED(connectInfo);
}


// Sends a single message to the content script(s) in the specified tab, 
// with an optional callback to run when a response is sent back. 
// The runtime.onMessage event is fired in each content script running in the specified tab for the current extension.
QVariantMap Tabs::sendMessage(TabId tabId, QVariant message /*, function responseCallback */)
{
    kDebug() << "UNIMPLEMENTED";
    Q_UNUSED(tabId);
    Q_UNUSED(message);
    
    return QVariantMap();
}


// Creates a new tab. 
// NOTE: This function can be used without requesting the 'tabs' permission in the manifest.
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


// Duplicates a tab. 
// NOTE: This function can be used without requesting the 'tabs' permission in the manifest.
QVariantMap Tabs::duplicate(TabId tabId /*, function callback */ )
{
    WebWindow *tab = tabFor(tabId);
    if (!tab)
        return QVariantMap();

    RekonqWindow *window = Application::instance()->rekonqWindow();
    int index = window->tabWidget()->indexOf(tab);
    kDebug() << "INDEX: " << index;
    
    window->tabWidget()->cloneTab(index);
    
    return get(tab->id());
}


// Gets all tabs that have the specified properties, or all tabs if no properties are specified.
QVariantMap Tabs::query(QVariantMap queryInfo /*, function callback */)
{
    kDebug() << "UNIMPLEMENTED";
    Q_UNUSED(queryInfo);

    return QVariantMap();
}


// Highlights the given tabs.
QVariantMap Tabs::highlight(QVariantMap highlightInfo /*, function callback */)
{
    kDebug() << "UNIMPLEMENTED";
    Q_UNUSED(highlightInfo);

    return QVariantMap();
}


// Modifies the properties of a tab. 
// Properties that are not specified in updateProperties are not modified. 
// NOTE: This function can be used without requesting the 'tabs' permission in the manifest.
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


// Moves one or more tabs to a new position within its window, or to a new window. 
// Note that tabs can only be moved to and from normal (window.type === "normal") windows.
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

    RekonqWindow *window = Application::instance()->rekonqWindow();
    window->tabWidget()->detachTab(index, destWindow);
    
    return get(tab->id());
}


// Reload a tab.
QVariantMap Tabs::reload(TabId tabId, QVariantMap reloadProperties /*, function callback */)
{
    Q_UNUSED(reloadProperties);

    WebWindow *tab = tabFor(tabId);
    if (!tab)
        return QVariantMap();

    RekonqWindow *window = Application::instance()->rekonqWindow();
    int index = window->tabWidget()->indexOf(tab);
    kDebug() << "INDEX: " << index;
    
    window->tabWidget()->reloadTab(index);
    
    return get(tab->id());
}


// Closes one or more tabs. 
// NOTE: This function can be used without requesting the 'tabs' permission in the manifest.
void Tabs::remove(TabId tabId /*, function callback */)
{
    WebWindow *tab = tabFor(tabId);
    if (!tab)
        return;

    RekonqWindow *window = Application::instance()->rekonqWindow();
    int index = window->tabWidget()->indexOf(tab);
    kDebug() << "INDEX: " << index;
    
    window->tabWidget()->closeTab(index);
    
    return;
}


// Detects the primary language of the content in a tab.
void Tabs::detectLanguage(TabId tabId /*, function callback */)
{
    kDebug() << "UNIMPLEMENTED";
    Q_UNUSED(tabId);
}


// Captures the visible area of the currently active tab in the specified window. 
// You must have host permission for the URL displayed by the tab.
void Tabs::captureVisibleTab(TabId windowId, QVariantMap options /*, function callback */)
{
    kDebug() << "UNIMPLEMENTED";
    Q_UNUSED(windowId);
    Q_UNUSED(options);
}


// Injects JavaScript code into a page. 
// For details, see the programmatic injection section of the content scripts doc.
void Tabs::executeScript(TabId tabId, QVariantMap details /*, function callback */)
{
    kDebug() << "UNIMPLEMENTED";
    Q_UNUSED(tabId);
    Q_UNUSED(details);
}


// Injects CSS into a page. 
// For details, see the programmatic injection section of the content scripts doc.
void Tabs::insertCSS(TabId tabId, QVariantMap details /*, function callback*/)
{
    kDebug() << "UNIMPLEMENTED";
    Q_UNUSED(tabId);
    Q_UNUSED(details);
}
