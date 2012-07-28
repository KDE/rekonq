/***************************************************************************
 *   Copyright (C) 2012 by Andrea Diamantini <adjam7@gmail.com>                            *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/



#ifndef TAB_WINDOW
#define TAB_WINDOW


#include <KTabWidget>

class KUrl;

class QLabel;
class QToolButton;
class QWebHistory;

class TabHistory;

class TabBar;
class WebPage;
class WebWindow;


// --------------------------------------------------------------------------------------


namespace Rekonq
{

/**
* @short Open link options
* Different modes of opening new tab
*/
enum OpenType
{
    CurrentTab,         ///< open url in current tab
    NewTab,             ///< open url according to users settings
    NewFocusedTab,      ///< open url in new tab and focus it
    NewBackGroundTab,   ///< open url in new background tab
    NewWindow           ///< open url in new window
};

    
}


// --------------------------------------------------------------------------------------


class TabWindow : public KTabWidget
{
    Q_OBJECT

public:
    TabWindow(QWidget *parent = 0);

    virtual QSize sizeHint() const;

    WebWindow* currentWebWindow() const;
    WebWindow* webWindow(int index) const;

    TabBar* tabBar() const;

public Q_SLOTS:
    void loadUrl(const KUrl &, Rekonq::OpenType type = Rekonq::CurrentTab, TabHistory *history = 0);
    void newCleanTab();

private:
    /**
     * Updates tabbar and add new tab button position
     */
    void updateTabBar();

    /**
     * Prepares the new WebWindow to be open
     */
    WebWindow *prepareNewTab(WebPage *page = 0);

private Q_SLOTS:
    void tabTitleChanged(const QString &);

    void tabLoadStarted();
    void tabLoadFinished(bool);

    void pageCreated(WebPage *);

    void currentChanged(int);

    // Indexed slots
    void cloneTab(int index = -1);
    void closeTab(int index = -1, bool del = true);
    void closeOtherTabs(int index = -1);
    void reloadTab(int index = -1);
    void reloadAllTabs();
    void restoreClosedTab(int i);

protected:
    virtual void resizeEvent(QResizeEvent *);

private:
    // the new tab button
    QToolButton *_addTabButton;

    int _openedTabsCounter;

    QList<TabHistory> m_recentlyClosedTabs;
};

#endif // TAB_WINDOW
