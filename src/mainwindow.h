
/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2008-2010 by Andrea Diamantini <adjam7 at gmail dot com>
* Copyright (C) 2009 by Paweł Prażak <pawelprazak at gmail dot com>
* Copyright (C) 2009-2010 by Lionel Chauvin <megabigbug@yahoo.fr>
* Copyright (C) 2010 by Matthieu Gicquel <matgic78 at gmail dot com>
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


#ifndef MAINWINDOW_H
#define MAINWINDOW_H

// Rekonq Includes
#include "rekonq_defines.h"

// Local Includes
#include "bookmarkprovider.h"

// KDE Includes
#include <KXmlGuiWindow>
#include <KActionCollection>
#include <KToolBar>
#include <KUrl>

// Forward Declarations
class FindBar;
class ZoomBar;
class HistoryPanel;
class BookmarksPanel;
class WebInspectorPanel;
class WebTab;
class MainView;
class NetworkAnalyzerPanel;

class KAction;
class KPassivePopup;

class QWebFrame;
class QSlider;


/**
 * This class serves as the main window for rekonq.
 * It handles the menus, toolbars, and status bars.
 *
 */
class REKONQ_TESTS_EXPORT MainWindow : public KXmlGuiWindow
{
    Q_OBJECT

public:
    MainWindow();
    ~MainWindow();

    inline MainView *mainView() const { return m_view; }
    inline QAction *actionByName(const QString &name) { return actionCollection()->action(name); }

    WebTab *currentTab() const;
    virtual QSize sizeHint() const;
    void setWidgetsVisible(bool makeFullScreen);

    QString selectedText() const;

private:
    void setupBookmarksAndToolsShortcuts();
    void setupActions();
    void setupTools();
    void setupToolbars();
    void setupPanels();

public slots:
    void homePage(Qt::MouseButtons = Qt::LeftButton, Qt::KeyboardModifiers = Qt::NoModifier);

    /**
     * Notifies a message in a popup
     *
     * @param msg The message to notify
     *
     * @param status The status message
     *
     */
    void notifyMessage(const QString &msg, Rekonq::Notify status = Rekonq::Url);

    void printRequested(QWebFrame *frame = 0);

    void updateActions();

    virtual void configureToolbars();

signals:
    // switching tabs
    void ctrlTabPressed();
    void shiftCtrlTabPressed();

protected slots:
    void saveNewToolbarConfig();

protected:
    /**
    * Filters (SHIFT + ) CTRL + TAB events and emit (shift)ctrlTabPressed()
    * to make switch tab
    * Filters out ESC key to show/hide the search bar
    */
    void keyPressEvent(QKeyEvent *event);

    bool queryClose();

private slots:
    void postLaunch();
    void browserLoading(bool);
    void updateWindowTitle(const QString &title = QString());

    // history related
    void openPrevious(Qt::MouseButtons = Qt::LeftButton, Qt::KeyboardModifiers = Qt::NoModifier);
    void openNext(Qt::MouseButtons = Qt::LeftButton, Qt::KeyboardModifiers = Qt::NoModifier);

    // Find Action slots
    void find(const QString &);
    void matchCaseUpdate();
    void findNext();
    void findPrevious();
    void highlightAll();

    // File Menu slots
    void openLocation();
    void fileOpen();
    void fileSaveAs();

    void viewPageSource();
    void viewFullScreen(bool enable);

    // Tools Menu slots
    void privateBrowsing(bool enable);

    // Settings Menu slot
    void preferences();

    // clear private data
    void clearPrivateData();

    void aboutToShowBackMenu();
    void openActionUrl(QAction *action);

    // encodings
    void setEncoding(QAction *);
    void populateEncodingMenu();

    void enableNetworkAnalysis(bool);

    void initBookmarkBar();
    void initToolsMenu();
    
private:
    MainView *m_view;
    FindBar *m_findBar;
    ZoomBar *m_zoomBar;

    HistoryPanel *m_historyPanel;
    BookmarksPanel *m_bookmarksPanel;
    WebInspectorPanel *m_webInspectorPanel;
    NetworkAnalyzerPanel *m_analyzerPanel;

    KAction *m_stopReloadAction;
    KMenu *m_historyBackMenu;
    KMenu *m_encodingMenu;

    BookmarkToolBar *m_bookmarksBar;

    QString m_lastSearch;

    KPassivePopup *m_popup;
    QTimer *m_hidePopup;

    KActionMenu *m_toolsMenu;
};

#endif // MAINWINDOW_H
