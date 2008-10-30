/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the demonstration applications of the Qt Toolkit.
**
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License versions 2.0 or 3.0 as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file.  Please review the following information
** to ensure GNU General Public Licensing requirements will be met:
** http://www.fsf.org/licensing/licenses/info/GPLv2.html and
** http://www.gnu.org/copyleft/gpl.html.  In addition, as a special
** exception, Nokia gives you certain additional rights. These rights
** are described in the Nokia Qt GPL Exception version 1.3, included in
** the file GPL_EXCEPTION.txt in this package.
**
** Qt for Windows(R) Licensees
** As a special exception, Nokia, as the sole copyright holder for Qt
** Designer, grants users of the Qt/Eclipse Integration plug-in the
** right for the Qt/Eclipse Integration to link to functionality
** provided by Qt Designer and its related libraries.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
**
****************************************************************************/

#ifndef BROWSERMAINWINDOW_H
#define BROWSERMAINWINDOW_H

#include <QtGui/QMainWindow>
#include <QtGui/QIcon>
#include <QtCore/QUrl>

class AutoSaver;
class BookmarksToolBar;
class ChaseWidget;
class QWebFrame;
class TabWidget;
class ToolbarSearch;
class WebView;

/*!
    The MainWindow of the Browser Application.

    Handles the tab widget and all the actions
 */
class BrowserMainWindow : public QMainWindow {
    Q_OBJECT

public:
    BrowserMainWindow(QWidget *parent = 0, Qt::WindowFlags flags = 0);
    ~BrowserMainWindow();
    QSize sizeHint() const;

public:
    static QUrl guessUrlFromString(const QString &url);
    TabWidget *tabWidget() const;
    WebView *currentTab() const;
    QByteArray saveState(bool withTabs = true) const;
    bool restoreState(const QByteArray &state);

public slots:
    void loadPage(const QString &url);
    void slotHome();

protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void save();

    void slotLoadProgress(int);
    void slotUpdateStatusbar(const QString &string);
    void slotUpdateWindowTitle(const QString &title = QString());

    void loadUrl(const QUrl &url);
    void slotPreferences();

    void slotFileNew();
    void slotFileOpen();
    void slotFilePrintPreview();
    void slotFilePrint();
    void slotPrivateBrowsing();
    void slotFileSaveAs();
    void slotEditFind();
    void slotEditFindNext();
    void slotEditFindPrevious();
    void slotShowBookmarksDialog();
    void slotAddBookmark();
    void slotViewTextBigger();
    void slotViewTextNormal();
    void slotViewTextSmaller();
    void slotViewToolbar();
    void slotViewBookmarksBar();
    void slotViewStatusbar();
    void slotViewPageSource();
    void slotViewFullScreen(bool enable);

    void slotWebSearch();
    void slotToggleInspector(bool enable);
    void slotAboutApplication();
    void slotDownloadManager();
    void slotSelectLineEdit();

    void slotAboutToShowBackMenu();
    void slotAboutToShowForwardMenu();
    void slotAboutToShowWindowMenu();
    void slotOpenActionUrl(QAction *action);
    void slotShowWindow();
    void slotSwapFocus();

    void printRequested(QWebFrame *frame);
    void geometryChangeRequested(const QRect &geometry);
    void updateToolbarActionText(bool visible);
    void updateBookmarksToolbarActionText(bool visible);

private:
    void loadDefaultState();
    void setupMenu();
    void setupToolBar();
    void updateStatusbarActionText(bool visible);

private:
    QToolBar *m_navigationBar;
    ToolbarSearch *m_toolbarSearch;
    BookmarksToolBar *m_bookmarksToolbar;
    ChaseWidget *m_chaseWidget;
    TabWidget *m_tabWidget;
    AutoSaver *m_autoSaver;

    QAction *m_historyBack;
    QMenu *m_historyBackMenu;
    QAction *m_historyForward;
    QMenu *m_historyForwardMenu;
    QMenu *m_windowMenu;

    QAction *m_stop;
    QAction *m_reload;
    QAction *m_stopReload;
    QAction *m_viewToolbar;
    QAction *m_viewBookmarkBar;
    QAction *m_viewStatusbar;
    QAction *m_restoreLastSession;
    QAction *m_addBookmark;

    QIcon m_reloadIcon;
    QIcon m_stopIcon;

    QString m_lastSearch;
};

#endif // BROWSERMAINWINDOW_H

