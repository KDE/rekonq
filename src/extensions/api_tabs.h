/* ============================================================
*
* This file is a part of the rekonq project
*
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


#ifndef API_TABS_H
#define API_TABS_H


// Rekonq Includes
#include "rekonq_defines.h"

// Qt Includes
#include <QObject>

// Forward Declarations
class WebWindow;
class RekonqWindow;

// typedefs
typedef quint32 TabId;
typedef quint32 WindowId;


class REKONQ_TESTS_EXPORT Tabs : public QObject
{
    Q_OBJECT

public:
    Tabs(QObject *parent = 0);

private:
    /**
     * Searches and eventually returns a pointer
     * to the tab with id tabId
     */
    WebWindow *tabFor(TabId tabId);
    
    /**
     * Searches and eventually returns a pointer
     * to the window with id winId
     */
    RekonqWindow *windowFor(int winId);
    
public Q_SLOTS:
    // For the public API implemented here
    // see: http://developer.chrome.com/extensions/tabs.html

    QVariantMap get(TabId tabId /*, function callback */ );
    
    QVariantMap getCurrent(/* function callback */ );
    
    // Chrome return value type is "Runtime.Port"
    void connect(TabId tabId, QVariantMap connectInfo);
    
    QVariantMap sendMessage(TabId tabId, QVariant message /*, function responseCallback */);
    
    QVariantMap create(QVariantMap createProperties /*, function callback */ );

    QVariantMap duplicate(TabId tabId /*, function callback */ );
    
    QVariantMap query(QVariantMap queryInfo /*, function callback */);
    
    QVariantMap highlight(QVariantMap highlightInfo /*, function callback */);
    
    QVariantMap update(TabId tabId, QVariantMap updateProperties /*, function callback */);

    QVariantMap move(TabId tabId, QVariantMap moveProperties /*, function callback */);

    QVariantMap reload(TabId tabId, QVariantMap reloadProperties /*, function callback */);
    
    void remove(TabId tabId /*, function callback */);
 
    void detectLanguage(TabId tabId /*, function callback */);
    
    void captureVisibleTab(WindowId windowId, QVariantMap options /*, function callback */);
    
    void executeScript(TabId tabId, QVariantMap details /*, function callback */);
    
    void insertCSS(TabId tabId, QVariantMap details /*, function callback*/);
    
    
Q_SIGNALS:
    void onCreated(QVariantMap tab);
    void onUpdated(TabId tabId, QObject *changeInfo, QObject *tab);
    void onMoved(TabId tabId, QObject *moveInfo);
    void onActivated(TabId tabId, QObject *moveInfo);
    void onHighlighted(TabId tabId, QObject *moveInfo);
    void onDetached(TabId tabId, QObject *detachInfo);
    void onAttached(TabId tabId, QObject *attachInfo);
    void onRemoved(TabId tabId);
    void onReplaced(TabId tabId, QObject *selectInfo);
};

#endif // API_TABS_H
