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
    WebWindow *tabFor(int tabId);
    
    /**
     * Searches and eventually returns a pointer
     * to the window with id winId
     */
    RekonqWindow *windowFor(int winId);
    
public Q_SLOTS:
     /**
     * chrome.tabs.captureVisibleTab()
     */
    void captureVisibleTab(int windowId /* default is current */, const QVariantMap options);

    /**
     * chrome.tabs.create()
     */
    QVariantMap create(QVariantMap createProperties);

    /**
     * chrome.tabs.get()
     */
    QVariantMap get(int tabId);

    /**
     * chrome.tabs.getAllInWindow()
     */
    QVariantList getAllInWindow(int windowId);

    /**
     * chrome.tabs.getSelected()
     */
    QVariantMap getSelected(int windowId);

    /**
     * chrome.tabs.move()
     */
    QVariantMap move(int tabId, QVariantMap moveProperties);

    /**
     * chrome.tabs.remove()
     */
    void remove(int tabId);
 
    /**
     * chrome.tabs.update()
     */
    QVariantMap update(int tabId, QVariantMap updateProperties);

    /**
     * chrome.tabs.connect()
     */
    void connect(int tabId, QVariantMap connectInfo);
    
    /**
     * chrome.tabs.detectLanguage()
     */
    void detectLanguage(int tabId);
    
    /**
     * chrome.tabs.executeScript()
     */
    void executeScript(int tabId, QVariantMap details);
    
    /**
     * chrome.tabs.insertCSS()
     */
    void insertCSS(int tabId, QVariantMap details);

    /**
     * chrome.tabs.sendRequest()
     */
    void sendRequest(int tabId, QVariant anything);
    
};

#endif // API_TABS_H
