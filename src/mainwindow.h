
/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2008-2012 by Andrea Diamantini <adjam7 at gmail dot com>
* Copyright (C) 2009 by Paweł Prażak <pawelprazak at gmail dot com>
* Copyright (C) 2009-2011 by Lionel Chauvin <megabigbug@yahoo.fr>
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

// KDE Includes
#include <KActionCollection>
#include <KXmlGuiWindow>

// Forward Declarations
class BookmarksPanel;
class BookmarkToolBar;
class FindBar;
class HistoryPanel;
class MainView;
class NetworkAnalyzerPanel;
class RekonqMenu;
class WebInspectorPanel;
class WebTab;
class ZoomBar;

class KUriFilter;

class QWebFrame;
class QWebHistory;
class QLabel;


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

    
    static KUriFilter *s_uriFilter;
    static KUrl filterUrlToLoad(const KUrl &url);
    

    inline MainView *mainView() const
    {
        return m_view;
    }
    inline QAction *actionByName(const QString &name)
    {
        return actionCollection()->action(name);
    }

    WebTab *currentTab() const;
    virtual QSize sizeHint() const;
    void setWidgetsVisible(bool makeFullScreen);

    QString selectedText() const;

    void loadCheckedUrl(const KUrl& url,
                    const Rekonq::OpenType& type = Rekonq::CurrentTab,
                    QWebHistory *webHistory = 0
                    );
private:
    void setupBookmarksAndToolsShortcuts();
    void setupActions();
    void setupTools();
    void setupToolbars();
    void setupPanels();

public Q_SLOTS:
    void loadUrl(const KUrl& url,
                 const Rekonq::OpenType& type = Rekonq::CurrentTab,
                 QWebHistory *webHistory = 0
                );

    void openDownloadsPage();
    void homePage();

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

    void updateHistoryActions();
    void updateTabActions();

    virtual void configureToolbars();

    // Find Bar slots
    void findNext();
    void findPrevious();
    void updateHighlight();
    void findSelectedText();

    void changeWindowIcon(int index);

Q_SIGNALS:
    // switching tabs
    void ctrlTabPressed();
    void shiftCtrlTabPressed();

    void triggerPartPrint();
    void triggerPartFind();

    void windowClosing();

protected Q_SLOTS:
    void saveNewToolbarConfig();

protected:
    /**
    * Filters (SHIFT + ) CTRL + TAB events and emit (shift)ctrlTabPressed()
    * to make switch tab
    * Filters out ESC key to show/hide the search bar
    */
    void keyPressEvent(QKeyEvent *event);
    void moveEvent(QMoveEvent *event);
    void resizeEvent(QResizeEvent *event);
    void closeEvent(QCloseEvent *event);

    bool event(QEvent *event);

    void readProperties(const KConfigGroup & config);
    void finalizeGUI(KXMLGUIClient *client);

private Q_SLOTS:
    void updateWindowTitle(const QString &title = QString());

    // history related
    void openPrevious(Qt::MouseButtons = Qt::LeftButton, Qt::KeyboardModifiers = Qt::NoModifier);
    void openNext(Qt::MouseButtons = Qt::LeftButton, Qt::KeyboardModifiers = Qt::NoModifier);

    // Find Action slots
    void find(const QString &);
    void matchCaseUpdate();

    // File Menu slots
    void openLocation();
    void fileOpen();
    void fileSaveAs();

    void viewPageSource();
    void viewFullScreen(bool enable);

    // Settings Menu slot
    void preferences();

    // clear private data
    void clearPrivateData();

    void aboutToShowBackMenu();
    void aboutToShowForwardMenu();

    void aboutToShowTabListMenu();
    void openActionUrl(QAction *action);
    void openActionTab(QAction *action);

    void populateUserAgentMenu();

    void enableNetworkAnalysis(bool);
    void setEditable(bool);

    void toggleBookmarkBarVisible(bool);

    /**
     * This is for the things to do ABSOLUTELY AFTER ctor launch:
     * the less, the better.
     */
    void postLaunch();

private:
    MainView *m_view;
    FindBar *m_findBar;
    ZoomBar *m_zoomBar;

    HistoryPanel *m_historyPanel;
    BookmarksPanel *m_bookmarksPanel;
    WebInspectorPanel *m_webInspectorPanel;
    NetworkAnalyzerPanel *m_analyzerPanel;

    KAction *m_loadStopReloadAction;

    KMenu *m_historyBackMenu;
    KMenu *m_historyForwardMenu;

    KMenu *m_tabListMenu;

    BookmarkToolBar *m_bookmarksBar;

    QString m_lastSearch;

    QLabel *m_popup;
    QTimer *m_hidePopupTimer;

    RekonqMenu *m_rekonqMenu;
};

#endif // MAINWINDOW_H
