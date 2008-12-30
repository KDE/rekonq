/* ============================================================
 *
 * This file is a part of the rekonq project
 *
 * Copyright (C) 2007-2008 Trolltech ASA. All rights reserved
 * Copyright (C) 2008 by Andrea Diamantini <adjam7 at gmail dot com>
 *
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */


// Self Includes
#include "mainwindow.h"

// Local Includes
#include "browserapplication.h"
#include "downloadmanager.h"
#include "history.h"
#include "settings.h"
#include "tabwidget.h"
#include "bookmarks.h"
#include "webview.h"

// UI Includes
#include "ui_passworddialog.h"

// KDE Includes
#include <KStatusBar>
#include <KMenuBar>
#include <KShortcut>
#include <KStandardAction>
#include <KAction>
#include <KToggleFullScreenAction>
#include <KActionCollection>
#include <KMessageBox>
#include <KFileDialog>

// Qt Includes
#include <QPlainTextEdit>
#include <QPrintDialog>
#include <QPrintPreviewDialog>
#include <QPrinter>
#include <QWebFrame>
#include <QWebHistory>
#include <QDebug>


MainWindow::MainWindow()
    : KXmlGuiWindow()
    , m_tabWidget( new TabWidget(this) )
{
    // accept dnd
    setAcceptDrops(true);

    m_tabWidget->newTab();

    // tell the KXmlGuiWindow that this is indeed the main widget
    setCentralWidget(m_tabWidget);

    connect(m_tabWidget, SIGNAL( loadPage(const QString &) ), this, SLOT( loadPage(const QString &) ) );
    connect(m_tabWidget, SIGNAL( setCurrentTitle(const QString &)), this, SLOT( slotUpdateWindowTitle(const QString &) ) );
    connect(m_tabWidget, SIGNAL( showStatusBarMessage(const QString&)), statusBar(), SLOT( showMessage(const QString&) ) );
    connect(m_tabWidget, SIGNAL( linkHovered(const QString&)), statusBar(), SLOT( showMessage(const QString&) ) );
    connect(m_tabWidget, SIGNAL( loadProgress(int)), this, SLOT( slotLoadProgress(int) ) );
//     connect(m_tabWidget, SIGNAL( tabsChanged()), m_autoSaver, SLOT( changeOccurred() ) );
    connect(m_tabWidget, SIGNAL( geometryChangeRequested(const QRect &)), this, SLOT( geometryChangeRequested(const QRect &) ) );
    connect(m_tabWidget, SIGNAL( printRequested(QWebFrame *)), this, SLOT( printRequested(QWebFrame *) ) );
    connect(m_tabWidget, SIGNAL( menuBarVisibilityChangeRequested(bool)), menuBar(), SLOT( setVisible(bool) ) );
    connect(m_tabWidget, SIGNAL( statusBarVisibilityChangeRequested(bool)), statusBar(), SLOT( setVisible(bool) ) );
//     connect(m_tabWidget, SIGNAL( toolBarVisibilityChangeRequested(bool) ), m_navigationBar, SLOT( setVisible(bool) ) );
    connect(m_tabWidget, SIGNAL( lastTabClosed() ), m_tabWidget, SLOT(newTab() ) );

    slotUpdateWindowTitle();
// --------------------------------------------------------------------------------------------------------------------------------


    // then, setup our actions
    setupActions();

    // add a status bar
    statusBar()->show();

    // a call to KXmlGuiWindow::setupGUI() populates the GUI
    // with actions, using KXMLGUI.
    // It also applies the saved mainwindow settings, if any, and ask the
    // mainwindow to automatically save settings if changed: window size,
    // toolbar position, icon size, etc.
    setupGUI();

    // setup history & bookmarks menus
    setupCustomMenu();

    // setting up custom widgets..
    KToolBar *navigationBar = toolBar( "mainToolBar" );
    navigationBar->addWidget( m_tabWidget->lineEditStack() );

    m_searchBar = new SearchBar( this );
    connect(m_searchBar, SIGNAL(search(const KUrl&)), this, SLOT(loadUrl(const KUrl&)));
    navigationBar->addWidget(m_searchBar);

    // Find Bar
    m_findBar = new FindBar(this);
    connect( m_findBar, SIGNAL( searchString(const QString &) ), this, SLOT( slotFind(const QString &) ) ); 

}


MainWindow::~MainWindow()
{
}


void MainWindow::setupActions()
{
    KAction *a;

    // Standard Actions
    KStandardAction::openNew(this, SLOT( slotFileNew() ) , actionCollection() );
    KStandardAction::open( this, SLOT( slotFileOpen() ), actionCollection() );
    KStandardAction::saveAs( this, SLOT( slotFileSaveAs() ), actionCollection() );
    KStandardAction::printPreview( this, SLOT( slotFilePrintPreview() ), actionCollection() );
    KStandardAction::print( this, SLOT( slotFilePrint() ), actionCollection() );
    KStandardAction::quit( this , SLOT( close() ), actionCollection() ); 
    KStandardAction::find(this, SLOT( slotViewFindBar() ) , actionCollection() );
    KStandardAction::findNext(this, SLOT( slotFindNext() ) , actionCollection() );
    KStandardAction::findPrev(this, SLOT( slotFindPrevious() ) , actionCollection() );
    KStandardAction::fullScreen( this, SLOT( slotViewFullScreen(bool) ), this, actionCollection() );
    KStandardAction::home( this, SLOT( slotHome() ), actionCollection() );
    KStandardAction::preferences( this, SLOT( slotPreferences() ), actionCollection() );

    a = KStandardAction::redisplay( this, 0, actionCollection() );
    m_tabWidget->addWebAction( a, QWebPage::Reload );

    a = KStandardAction::back( this, 0, actionCollection() );
    m_tabWidget->addWebAction( a, QWebPage::Back );

    a = KStandardAction::forward( this, 0, actionCollection() );
    m_tabWidget->addWebAction( a, QWebPage::Forward );

    a = KStandardAction::undo( this , 0 , actionCollection() );
    m_tabWidget->addWebAction( a , QWebPage::Undo );

    a = KStandardAction::redo( this , 0 , actionCollection() );
    m_tabWidget->addWebAction( a, QWebPage::Redo );

    a = KStandardAction::cut( this , 0 , actionCollection() );
    m_tabWidget->addWebAction( a, QWebPage::Cut );

    a = KStandardAction::copy( this , 0 , actionCollection() );
    m_tabWidget->addWebAction( a, QWebPage::Copy );

    a = KStandardAction::paste( this , 0 , actionCollection() );
    m_tabWidget->addWebAction( a, QWebPage::Paste );

    a = KStandardAction::selectAll( this , 0 , actionCollection() );
    m_tabWidget->addWebAction( a, QWebPage::SelectEndOfDocument );

    // stop reload Action 
    m_stopReload = new KAction( KIcon("view-refresh"), i18n("reload"), this );
    actionCollection()->addAction( QLatin1String("stop reload") , m_stopReload );

    // Custom Actions
    a = new KAction ( KIcon( "process-stop" ), i18n("&Stop"), this );
    a->setShortcut( QKeySequence(Qt::CTRL | Qt::Key_Period) );
    actionCollection()->addAction( QLatin1String("stop"), a );
    m_tabWidget->addWebAction( a, QWebPage::Stop);
    
    a = new KAction( KIcon(), i18n("Open Location"), this); 
    actionCollection()->addAction( QLatin1String("open location"), a );
    connect( a, SIGNAL( triggered(bool) ) , this, SLOT( slotOpenLocation() ) );

    actionCollection()->addAction( QLatin1String("new tab"), m_tabWidget->newTabAction() );
    actionCollection()->addAction( QLatin1String("close tab"), m_tabWidget->closeTabAction() );

    a = new KAction( i18n("Private &Browsing..."), this );
    a->setCheckable(true);
    actionCollection()->addAction( i18n("private browsing"), a );
    connect( a, SIGNAL( triggered(bool) ) , this, SLOT( slotPrivateBrowsing() ) );

    a = new KAction( KIcon("zoom-in"), i18n("&Enlarge font"), this );
    a->setShortcut( QKeySequence(Qt::CTRL | Qt::Key_Plus) );
    actionCollection()->addAction( QLatin1String("bigger font"), a );
    connect( a, SIGNAL( triggered( bool ) ), this, SLOT( slotViewTextBigger() ) );

    a = new KAction( KIcon("zoom-original"),  i18n("&Normal font"), this );
    a->setShortcut( QKeySequence(Qt::CTRL | Qt::Key_0) );
    actionCollection()->addAction( QLatin1String("normal font"), a );
    connect( a, SIGNAL( triggered( bool ) ), this, SLOT( slotViewTextNormal() ) );

    a = new KAction( KIcon("zoom-out"),  i18n("&Shrink font"), this );
    a->setShortcut( QKeySequence(Qt::CTRL | Qt::Key_Minus) );
    actionCollection()->addAction( QLatin1String("smaller font"), a );
    connect( a, SIGNAL( triggered( bool ) ), this, SLOT( slotViewTextSmaller() ) );

    a = new KAction( i18n("Page S&ource"), this );
    actionCollection()->addAction( QLatin1String("page source"), a );
    connect( a, SIGNAL( triggered( bool ) ), this, SLOT( slotViewPageSource() ) );

    a = new KAction( KIcon( "kget" ), i18n("Downloads"), this );
    actionCollection()->addAction( QLatin1String("downloads"), a);
    connect( a, SIGNAL( triggered( bool ) ), this, SLOT( slotDownloadManager() ) );

    a = new KAction( KIcon("page-zoom"), i18n("Enable Web &Inspector"), this );
    a->setCheckable(true);
    actionCollection()->addAction( QLatin1String("web inspector"), a );
    connect( a, SIGNAL( triggered( bool ) ), this, SLOT( slotToggleInspector(bool) ) );

    a = new KActionMenu( i18n("B&ookmarks"), this );
    actionCollection()->addAction( QLatin1String("bookmarks"), a );
    BookmarksMenu *bookmarksMenu = new BookmarksMenu( this );
    a->setMenu( bookmarksMenu );

    // ===================================================================================================================
    // ===================================================================================================================
    // FIXME

    KAction *historyBack = new KAction( KIcon("go-previous"), i18n("Back"), this);
    m_historyBackMenu = new KMenu(this);
    historyBack->setMenu(m_historyBackMenu);
    connect(historyBack, SIGNAL( triggered( bool ) ), this, SLOT( slotOpenPrevious() ) );
    connect(m_historyBackMenu, SIGNAL(aboutToShow()), this, SLOT(slotAboutToShowBackMenu()));
    connect(m_historyBackMenu, SIGNAL(triggered(QAction *)), this, SLOT(slotOpenActionUrl(QAction *)));
    actionCollection()->addAction( QLatin1String("history back"), historyBack);

    KAction *historyForward = new KAction( KIcon("go-next"), i18n("Forward"), this );
    connect(historyForward, SIGNAL( triggered( bool ) ), this, SLOT( slotOpenNext() ) );
    actionCollection()->addAction( QLatin1String("history forward"), historyForward );
}


void MainWindow::setupCustomMenu()
{
    //  ------------------------------------------------------------- HISTORY MENU--------------------------------------------------------------------------------------------------
    HistoryMenu *historyMenu = new HistoryMenu(this);
    connect(historyMenu, SIGNAL(openUrl(const KUrl&)), m_tabWidget, SLOT(loadUrlInCurrentTab(const KUrl&)));
    connect(historyMenu, SIGNAL(hovered(const QString&)), this, SLOT(slotUpdateStatusbar(const QString&)));
    historyMenu->setTitle( i18n("Hi&story") );
    menuBar()->insertMenu( actionCollection()->action("bookmarks"), historyMenu);
    QList<QAction*> historyActions;

    historyActions.append( actionCollection()->action("Back") );
    historyActions.append( actionCollection()->action("Forward") );
    historyActions.append( actionCollection()->action("Home") );
    historyActions.append( m_tabWidget->recentlyClosedTabsAction() );

    historyMenu->setInitialActions(historyActions);
    // ------------------------------------------------------------------------------------------------------------------------------------------------------------------------

    // --------------------------------------------- BOOKMARKS  MENU -----------------------------------------------------------------------------------------------------

        
    // ------------------------------------------------------------------------------------------------------------------------------------------------------------------------
}

KUrl MainWindow::guessUrlFromString(const QString &string)
{
    QString urlStr = string.trimmed();
    QRegExp test(QLatin1String("^[a-zA-Z]+\\:.*"));

    // Check if it looks like a qualified URL. Try parsing it and see.
    bool hasSchema = test.exactMatch(urlStr);
    if (hasSchema) 
    {
        QUrl qurl(urlStr, QUrl::TolerantMode);
        KUrl url(qurl);
        if ( url.isValid() )
        {
            return url;
        }
    }

    // Might be a file.
    if (QFile::exists(urlStr)) 
    {
        QFileInfo info(urlStr);
        return KUrl::fromPath( info.absoluteFilePath() );
    }

    // Might be a shorturl - try to detect the schema.
    if (!hasSchema) 
    {
        int dotIndex = urlStr.indexOf(QLatin1Char('.'));
        if (dotIndex != -1) 
        {
            QString prefix = urlStr.left(dotIndex).toLower();
            QString schema = (prefix == QLatin1String("ftp")) ? prefix : QLatin1String("http");
            QUrl qurl(schema + QLatin1String("://") + urlStr, QUrl::TolerantMode);
            KUrl url(qurl);
            if ( url.isValid() )
            {
                return url;
            }
        }
    }

    // Fall back to QUrl's own tolerant parser.
    QUrl qurl = QUrl(string, QUrl::TolerantMode);
    KUrl url(qurl);

    // finally for cases where the user just types in a hostname add http
    if ( qurl.scheme().isEmpty() )
    {
        qurl = QUrl(QLatin1String("http://") + string, QUrl::TolerantMode);
        url = KUrl(qurl);
    }
    return url;
}


void MainWindow::loadUrl(const KUrl &url)
{
    loadPage( url.url() );
}


void MainWindow::slotDownloadManager()
{
    BrowserApplication::downloadManager()->show();
}


void MainWindow::slotOpenLocation()
{
    m_tabWidget->currentLineEdit()->selectAll();
    m_tabWidget->currentLineEdit()->setFocus();
}


void MainWindow::slotFileSaveAs()
{
    BrowserApplication::downloadManager()->download(currentTab()->url(), true);
}


void MainWindow::slotPreferences()
{
    SettingsDialog *s = new SettingsDialog(this);
    s->show();
}


void MainWindow::slotUpdateStatusbar(const QString &string)
{
    statusBar()->showMessage(string, 2000);
}


void MainWindow::slotUpdateWindowTitle(const QString &title)
{
    if (title.isEmpty()) 
    {
        setWindowTitle("rekonq");
    } 
    else 
    {
        setWindowTitle(title + " - rekonq");
    }
}


void MainWindow::slotFileNew()
{
    BrowserApplication::instance()->newMainWindow();
    MainWindow *mw = BrowserApplication::instance()->mainWindow();
    mw->slotHome();
}


void MainWindow::slotFileOpen()
{
    QString file = KFileDialog::getOpenFileName( KUrl(),
                                                                            i18n("Web Resources (*.html *.htm *.svg *.png *.gif *.svgz);;All files (*.*)"),
                                                                            this, 
                                                                            i18n("Open Web Resource") );

    if (file.isEmpty())
        return;

    loadPage(file);
}


void MainWindow::slotFilePrintPreview()
{
    if (!currentTab())
        return;
    QPrintPreviewDialog *dialog = new QPrintPreviewDialog(this);
    connect(dialog, SIGNAL(paintRequested(QPrinter *)), currentTab(), SLOT(print(QPrinter *)));
    dialog->exec();
}


void MainWindow::slotFilePrint()
{
    if (!currentTab())
        return;
    printRequested(currentTab()->page()->mainFrame());
}


void MainWindow::printRequested(QWebFrame *frame)
{
    QPrinter printer;
    QPrintDialog *dialog = new QPrintDialog(&printer, this);
    dialog->setWindowTitle( i18n("Print Document") );
    if (dialog->exec() != QDialog::Accepted )
        return;
    frame->print(&printer);
}


void MainWindow::slotPrivateBrowsing()
{
    QWebSettings *settings = QWebSettings::globalSettings();
    bool pb = settings->testAttribute(QWebSettings::PrivateBrowsingEnabled);
    if (!pb) 
    {
        QString title = i18n("Are you sure you want to turn on private browsing?");
        QString text = "<b>" + title + i18n("</b><br><br>When private browsing in turned on,"
            " webpages are not added to the history,"
            " items are automatically removed from the Downloads window," \
            " new cookies are not stored, current cookies can't be accessed," \
            " site icons wont be stored, session wont be saved, " \
            " and searches are not addded to the pop-up menu in the Google search box." \
            "  Until you close the window, you can still click the Back and Forward buttons" \
            " to return to the webpages you have opened.");

        int  button = KMessageBox::questionYesNo( this, text, title );
        if (button == KMessageBox::Ok) 
        {
            settings->setAttribute(QWebSettings::PrivateBrowsingEnabled, true);
        }
    } 
    else 
    {
        settings->setAttribute(QWebSettings::PrivateBrowsingEnabled, false);

        QList<MainWindow*> windows = BrowserApplication::instance()->mainWindows();
        for (int i = 0; i < windows.count(); ++i) 
        {
            MainWindow *window = windows.at(i);
            window->m_lastSearch = QString::null;
            window->tabWidget()->clear();
        }
    }
}


// void MainWindow::closeEvent(QCloseEvent *event)
// {
//     if (m_tabWidget->count() > 1) 
//     {
//         int ret = KMessageBox::warningYesNo(this, 
//                                                          i18n("Are you sure you want to close the window?" "  There are %1 tab open" , m_tabWidget->count() ) ,
//                                                         i18n("Closing") );
//         if (ret == KMessageBox::No) 
//         {
//             event->ignore();
//             return;
//         }
//     }
//     event->accept();
//     deleteLater();
// }


void MainWindow::slotFind(const QString & search)
{
    if (!currentTab())
        return;
    if (!search.isEmpty()) 
    {
        m_lastSearch = search;
        if (!currentTab()->findText(m_lastSearch))
            slotUpdateStatusbar( QString(m_lastSearch) + i18n(" not found.") );
    }
}


void MainWindow::slotViewFindBar()
{
    m_findBar->showFindBar();
}


void MainWindow::slotFindNext()
{
    if (!currentTab() && !m_lastSearch.isEmpty())
        return;
    currentTab()->findText(m_lastSearch);
}


void MainWindow::slotFindPrevious()
{
    if (!currentTab() && !m_lastSearch.isEmpty())
        return;
    currentTab()->findText(m_lastSearch, QWebPage::FindBackward);
}


void MainWindow::slotViewTextBigger()
{
    if (!currentTab())
        return;
    currentTab()->setTextSizeMultiplier(currentTab()->textSizeMultiplier() + 0.1);
}


void MainWindow::slotViewTextNormal()
{
    if (!currentTab())
        return;
    currentTab()->setTextSizeMultiplier(1.0);
}


void MainWindow::slotViewTextSmaller()
{
    if (!currentTab())
        return;
    currentTab()->setTextSizeMultiplier(currentTab()->textSizeMultiplier() - 0.1);
}


// TODO improve this
void MainWindow::slotViewFullScreen( bool makeFullScreen )
{
    KToggleFullScreenAction::setFullScreen( this, makeFullScreen );
}


void MainWindow::slotViewPageSource()
{
    if (!currentTab())
        return;

    QString markup = currentTab()->page()->mainFrame()->toHtml();
    QPlainTextEdit *view = new QPlainTextEdit(markup);
    view->setWindowTitle( i18n("Page Source of ") + currentTab()->title() );
    view->setMinimumWidth(640);
    view->setAttribute(Qt::WA_DeleteOnClose);
    view->show();
}


void MainWindow::slotHome()
{
    KConfig config("rekonqrc");
    KConfigGroup group = config.group("Global Settings");   
    QString home = group.readEntry( QString("home"), QString("http://www.kde.org/") );
    loadPage(home);
}


void MainWindow::slotToggleInspector(bool enable)
{
    QWebSettings::globalSettings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, enable);
    if (enable) 
    {
        int result = KMessageBox::questionYesNo(this, 
                                           i18n("The web inspector will only work correctly for pages that were loaded after enabling.\n"
                                           "Do you want to reload all pages?"),
                                            i18n("Web Inspector") );
        if (result == KMessageBox::Yes) 
        {
            m_tabWidget->reloadAllTabs();
        }
    }
}


void MainWindow::slotSwapFocus()
{
    if ( currentTab()->hasFocus() )
    {
        m_tabWidget->currentLineEdit()->setFocus();
    }
    else
    {
        currentTab()->setFocus();
    }
}


void MainWindow::loadPage(const QString &page)
{
    if (!currentTab() || page.isEmpty())
        return;

    KUrl url = guessUrlFromString(page);
    m_tabWidget->currentLineEdit()->setText( url.prettyUrl() );
    m_tabWidget->loadUrlInCurrentTab(url);
}


TabWidget *MainWindow::tabWidget() const
{
    return m_tabWidget;
}



WebView *MainWindow::currentTab() const
{
    return m_tabWidget->currentWebView();
}


// FIXME: this actually doesn't work properly..
void MainWindow::slotLoadProgress(int progress)
{
    QAction *stop = actionCollection()->action( "stop" );
    QAction *reload = actionCollection()->action(" redisplay" );
    if (progress < 100 && progress > 0) 
    {
        disconnect( m_stopReload, SIGNAL( triggered( bool ) ), reload , SIGNAL( triggered(bool) ) );
        m_stopReload->setIcon( KIcon( "process-stop" ) );
        connect(m_stopReload, SIGNAL( triggered(bool ) ), stop, SIGNAL( triggered(bool) ) );
        m_stopReload->setToolTip( i18n("Stop loading the current page") );
    } 
    else 
    {
        disconnect( m_stopReload, SIGNAL( triggered( bool ) ), stop , SIGNAL( triggered(bool ) ) );
        m_stopReload->setIcon( KIcon( "view-refresh" ) );
        connect(m_stopReload, SIGNAL( triggered( bool ) ), reload, SIGNAL( triggered(bool) ) );
        m_stopReload->setToolTip( i18n("Reload the current page") );
    }
}


void MainWindow::slotAboutToShowBackMenu()
{
    m_historyBackMenu->clear();
    if (!currentTab())
        return;
    QWebHistory *history = currentTab()->history();
    int historyCount = history->count();
    for (int i = history->backItems(historyCount).count() - 1; i >= 0; --i) 
    {
        QWebHistoryItem item = history->backItems(history->count()).at(i);
        KAction *action = new KAction(this);
        action->setData(-1*(historyCount-i-1));
        QIcon icon = BrowserApplication::instance()->icon(item.url());
        action->setIcon(icon);
        action->setText(item.title());
        m_historyBackMenu->addAction(action);
    }
}


void MainWindow::slotShowWindow()
{
    if (KAction *action = qobject_cast<KAction*>(sender())) 
    {
        QVariant v = action->data();
        if (v.canConvert<int>()) 
        {
            int offset = qvariant_cast<int>(v);
            QList<MainWindow*> windows = BrowserApplication::instance()->mainWindows();
            windows.at(offset)->activateWindow();
            windows.at(offset)->currentTab()->setFocus();
        }
    }
}


void MainWindow::slotOpenActionUrl(QAction *action)
{
    int offset = action->data().toInt();
    QWebHistory *history = currentTab()->history();
    if (offset < 0)
    {
        history->goToItem(history->backItems(-1*offset).first()); // back
    }
    else 
    {
        if (offset > 0)
        {
            history->goToItem(history->forwardItems(history->count() - offset + 1).back()); // forward
        }
    }
}


void MainWindow::slotOpenPrevious()
{
    QWebHistory *history = currentTab()->history();
    if ( history->canGoBack() )
        history->goToItem( history->backItem() );
}


void MainWindow::slotOpenNext()
{
    QWebHistory *history = currentTab()->history();
    if ( history->canGoForward() )
        history->goToItem( history->forwardItem() );
}


void MainWindow::geometryChangeRequested(const QRect &geometry)
{
    setGeometry(geometry);
}

