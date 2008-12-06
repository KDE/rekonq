/* ============================================================
 *
 * This file is a part of the reKonq project
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


// Local Includes
#include "browsermainwindow.h"
#include "autosaver.h"
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

// Qt Includes
#include <QDesktopWidget>
#include <QFileDialog>
#include <QPlainTextEdit>
#include <QPrintDialog>
#include <QPrintPreviewDialog>
#include <QPrinter>
#include <QToolBar>
#include <QInputDialog>
#include <QWebFrame>
#include <QWebHistory>
#include <QDebug>


BrowserMainWindow::BrowserMainWindow(QWidget *parent, Qt::WindowFlags flags)
    : KMainWindow(parent, flags)
    , m_tabWidget(new TabWidget(this))
    , m_autoSaver(new AutoSaver(this))
    , m_historyBack(0)
    , m_historyForward(0)
    , m_stop(0)
    , m_reload(0)
{
    // delete widget accepting close event
    setAttribute(Qt::WA_DeleteOnClose, true);

    setupMenu();
    setupToolBar();

    QWidget *centralWidget = new QWidget(this);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->setSpacing(0);
    layout->setMargin(0);
    addToolBarBreak();
    layout->addWidget(m_tabWidget);

    // Find Bar
    m_findBar = new FindBar(this);
    connect( m_findBar, SIGNAL( searchString(const QString &) ), this, SLOT( slotFind(const QString &) ) ); 

    centralWidget->setLayout(layout);
	setCentralWidget(centralWidget);

    connect(m_tabWidget, SIGNAL( loadPage(const QString &) ), this, SLOT( loadPage(const QString &) ) );
    connect(m_tabWidget, SIGNAL( setCurrentTitle(const QString &)), this, SLOT( slotUpdateWindowTitle(const QString &) ) );
    connect(m_tabWidget, SIGNAL( showStatusBarMessage(const QString&)), statusBar(), SLOT( showMessage(const QString&) ) );
    connect(m_tabWidget, SIGNAL( linkHovered(const QString&)), statusBar(), SLOT( showMessage(const QString&) ) );
    connect(m_tabWidget, SIGNAL( loadProgress(int)), this, SLOT( slotLoadProgress(int) ) );
    connect(m_tabWidget, SIGNAL( tabsChanged()), m_autoSaver, SLOT( changeOccurred() ) );
    connect(m_tabWidget, SIGNAL( geometryChangeRequested(const QRect &)), this, SLOT( geometryChangeRequested(const QRect &) ) );
    connect(m_tabWidget, SIGNAL( printRequested(QWebFrame *)), this, SLOT( printRequested(QWebFrame *) ) );
    connect(m_tabWidget, SIGNAL( menuBarVisibilityChangeRequested(bool)), menuBar(), SLOT( setVisible(bool) ) );
    connect(m_tabWidget, SIGNAL( statusBarVisibilityChangeRequested(bool)), statusBar(), SLOT( setVisible(bool) ) );
    connect(m_tabWidget, SIGNAL( toolBarVisibilityChangeRequested(bool) ), m_navigationBar, SLOT( setVisible(bool) ) );
    connect(m_tabWidget, SIGNAL( lastTabClosed() ), m_tabWidget, SLOT(newTab() ) );

    slotUpdateWindowTitle();
    loadDefaultState();
    m_tabWidget->newTab();
}



BrowserMainWindow::~BrowserMainWindow()
{
    m_autoSaver->changeOccurred();
    m_autoSaver->saveIfNeccessary();
}



void BrowserMainWindow::loadDefaultState()
{
    KConfig config("rekonqrc");
    KConfigGroup group1 = config.group("BrowserMainWindow");   
    QByteArray data = group1.readEntry(QString("defaultState"), QByteArray() );
    restoreState(data); 
}



QSize BrowserMainWindow::sizeHint() const
{
    QRect desktopRect = QApplication::desktop()->screenGeometry();
    QSize size = desktopRect.size() * 0.9;
    return size;
}



void BrowserMainWindow::save()
{
    BrowserApplication::instance()->saveSession();

    KConfig config("rekonqrc");
    KConfigGroup group1 = config.group("BrowserMainWindow");   
    QByteArray data = saveState(false);
    group1.writeEntry( QString("defaultState"), data );
}



static const qint32 BrowserMainWindowMagic = 0xba;



QByteArray BrowserMainWindow::saveState(bool withTabs) const
{
    int version = 2;
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);

    stream << qint32(BrowserMainWindowMagic);
    stream << qint32(version);

    stream << size();
//     stream << !( statusBar()->isHidden() ); FIXME 
    if (withTabs)
//         stream << tabWidget()->saveState();
    ;
    else
        stream << QByteArray();
    return data;
}



bool BrowserMainWindow::restoreState(const QByteArray &state)
{
    int version = 2;
    QByteArray sd = state;
    QDataStream stream(&sd, QIODevice::ReadOnly);
    if ( stream.atEnd() )
    {
        return false;
    }

    qint32 marker;
    qint32 v;
    stream >> marker;
    stream >> v;
    if (marker != BrowserMainWindowMagic || v != version)
    {
        return false;
    }

    QSize size;
    bool showStatusbar = true;
    QByteArray tabState;

    stream >> size;
//     stream >> showStatusbar; FIXME see 30 lines over..
    stream >> tabState;

    resize(size);
    statusBar()->setVisible(showStatusbar);
    updateStatusbarActionText(showStatusbar);

    if ( !tabWidget()->restoreState(tabState) )
    {
        return false;
    }
    return true;
}



void BrowserMainWindow::setupMenu()
{
    //  ------------------------------------------------------------- FILE --------------------------------------------------------------------------------------------------
    KMenu *fileMenu = (KMenu *) menuBar()->addMenu( i18n("&File") );

    fileMenu->addAction( KStandardAction::openNew(this, SLOT( slotFileNew() ) , this ) );
    fileMenu->addAction( KStandardAction::open( this, SLOT( slotFileOpen() ), this ) );
    fileMenu->addAction( i18n("Open Location"), this, SLOT( slotSelectLineEdit() ) );
    fileMenu->addSeparator();

    fileMenu->addAction( m_tabWidget->newTabAction() );
    fileMenu->addAction( m_tabWidget->closeTabAction() );
    fileMenu->addSeparator();

    fileMenu->addAction( KStandardAction::saveAs( this, SLOT( slotFileSaveAs() ), this ) );
    fileMenu->addSeparator();

    fileMenu->addAction( KStandardAction::printPreview( this, SLOT( slotFilePrintPreview() ), this ) );
    fileMenu->addAction( KStandardAction::print( this, SLOT(slotFilePrint()), this) );
    fileMenu->addSeparator();

    KAction *action = (KAction *) fileMenu->addAction( i18n("Private &Browsing..."), this, SLOT( slotPrivateBrowsing() ) );
    action->setCheckable(true);
    fileMenu->addSeparator();

    fileMenu->addAction( KStandardAction::quit( this , SLOT( close() ), this ) ); 

    //  ------------------------------------------------------------- EDIT --------------------------------------------------------------------------------------------------
    KMenu *editMenu = (KMenu *) menuBar()->addMenu( i18n("&Edit") );

    KAction *m_undo = KStandardAction::undo( this , 0 , this );
    editMenu->addAction( m_undo );
    m_tabWidget->addWebAction(m_undo, QWebPage::Undo);

    KAction *m_redo = KStandardAction::redo( this , 0 , this );
    editMenu->addAction( m_redo );
    m_tabWidget->addWebAction(m_redo, QWebPage::Redo);

    editMenu->addSeparator();

    KAction *m_cut = KStandardAction::cut( this , 0 , this );
    editMenu->addAction( m_cut );
    m_tabWidget->addWebAction(m_cut, QWebPage::Cut);

    KAction *m_copy = KStandardAction::copy( this , 0 , this );
    editMenu->addAction( m_copy );
    m_tabWidget->addWebAction(m_copy, QWebPage::Copy);

    KAction *m_paste = KStandardAction::paste( this , 0 , this );
    editMenu->addAction( m_paste );
    m_tabWidget->addWebAction(m_paste, QWebPage::Paste);

    editMenu->addSeparator();

    KAction *m_selectall = KStandardAction::selectAll( this , 0 , this );
    editMenu->addAction( m_selectall );
    m_tabWidget->addWebAction(m_selectall, QWebPage::SelectEndOfDocument );

    editMenu->addSeparator();

    editMenu->addAction( KStandardAction::find(this, SLOT( slotViewFindBar() ) , this ) );
    editMenu->addAction( KStandardAction::findNext(this, SLOT( slotFindNext() ) , this ) );
    editMenu->addAction( KStandardAction::findPrev(this, SLOT( slotFindPrevious() ) , this ) );

    //  ------------------------------------------------------------- VIEW -------------------------------------------------------------------------------------------------
    KMenu *viewMenu = (KMenu *) menuBar()->addMenu( i18n("&View") );

    m_viewStatusbar = KStandardAction::showStatusbar( this, SLOT(slotViewStatusbar() ), this);
    viewMenu->addAction(m_viewStatusbar);

    viewMenu->addSeparator();

    m_stop = (KAction *) viewMenu->addAction( KIcon( "process-stop" ), i18n("&Stop") );
    m_stop->setShortcut( QKeySequence(Qt::CTRL | Qt::Key_Period) );
    m_tabWidget->addWebAction(m_stop, QWebPage::Stop);

    m_reload = (KAction *) viewMenu->addAction( KIcon("view-refresh"), i18n("Reload Page") );
    m_reload->setShortcut(QKeySequence::Refresh);
    m_tabWidget->addWebAction(m_reload, QWebPage::Reload);

    viewMenu->addSeparator();

    KMenu *fontMenu = new KMenu( i18n("Make Text..."), this );
    fontMenu->addAction( i18n("&Bigger"), this, SLOT(slotViewTextBigger()), QKeySequence(Qt::CTRL | Qt::Key_Plus));
    fontMenu->addAction( i18n("&Normal"), this, SLOT(slotViewTextNormal()), QKeySequence(Qt::CTRL | Qt::Key_0));
    fontMenu->addAction( i18n("&Smaller"), this, SLOT(slotViewTextSmaller()), QKeySequence(Qt::CTRL | Qt::Key_Minus));

    viewMenu->addMenu( fontMenu );

    viewMenu->addSeparator();

    // TODO set encoding

    viewMenu->addAction( i18n("Page S&ource"), this, SLOT( slotViewPageSource() ), i18n("Ctrl+Alt+U"));

    KToggleFullScreenAction *tfsa = KStandardAction::fullScreen( this, SLOT( slotViewFullScreen(bool) ), this, this);
    viewMenu->addAction( tfsa );

    //  ------------------------------------------------------------- HISTORY --------------------------------------------------------------------------------------------------
    HistoryMenu *historyMenu = new HistoryMenu(this);
    connect(historyMenu, SIGNAL(openUrl(const KUrl&)), m_tabWidget, SLOT(loadUrlInCurrentTab(const KUrl&)));
    connect(historyMenu, SIGNAL(hovered(const QString&)), this, SLOT(slotUpdateStatusbar(const QString&)));
    historyMenu->setTitle( i18n("Hi&story") );
    menuBar()->addMenu(historyMenu);
    QList<KAction*> historyActions;

    m_historyBack = new KAction( i18n("Back"), this);
    m_historyBack->setShortcut( KShortcut( QKeySequence::Back ) ); 
    m_tabWidget->addWebAction(m_historyBack, QWebPage::Back);
    m_historyBack->setIconVisibleInMenu(false);

    m_historyForward = new KAction( i18n("Forward"), this);
    m_historyForward->setShortcut( KShortcut( QKeySequence::Forward ) );
    m_tabWidget->addWebAction(m_historyForward, QWebPage::Forward);
    m_historyForward->setIconVisibleInMenu(false);

    m_restoreLastSession = new KAction( i18n("Restore Last Session"), this);
    connect(m_restoreLastSession, SIGNAL(triggered()), BrowserApplication::instance(), SLOT(restoreLastSession()));
    m_restoreLastSession->setEnabled(BrowserApplication::instance()->canRestoreSession());

    historyActions.append(m_historyBack);
    historyActions.append(m_historyForward);
    historyActions.append( KStandardAction::home(this, SLOT( slotHome() ) , this ) );
    historyActions.append(m_tabWidget->recentlyClosedTabsAction());
    historyActions.append(m_restoreLastSession);
    historyMenu->setInitialActions(historyActions);



    //  ------------------------------------------------------------- BOOKMARKS --------------------------------------------------------------------------------------------------

    BookmarksMenu *bookmarksMenu = new BookmarksMenu( this );
    bookmarksMenu->setTitle( i18n("&Bookmarks") );
    menuBar()->addMenu( bookmarksMenu );

    //  ------------------------------------------------------------- TOOLS ------------------------------------------------------------------------------------------------------
    KMenu* toolsMenu = (KMenu *) menuBar()->addMenu( i18n("&Tools") );

    toolsMenu->addAction( i18n("Downloads"), this, SLOT( slotDownloadManager() ), i18n("Alt+Ctrl+D") );

    toolsMenu->addSeparator();

    action = (KAction *) toolsMenu->addAction( i18n("Enable Web &Inspector"), this, SLOT(slotToggleInspector(bool)));
    action->setCheckable(true);

    //  ------------------------------------------------------------- SETTINGS ------------------------------------------------------------------------------------------------------
    KMenu *settingsMenu = (KMenu *) menuBar()->addMenu( i18n("&Settings") );

//   TODO  settingsMenu->addAction( KStandardAction::configureToolbars(this, SLOT(  ) , this ) );
    settingsMenu->addAction( KStandardAction::preferences(this, SLOT( slotPreferences() ) , this ) );

    //  ------------------------------------------------------------- HELP  --------------------------------------------------------------------------------------------------
   menuBar()->addMenu( helpMenu() );
}


void BrowserMainWindow::setupToolBar()
{
    m_navigationBar = new KToolBar( i18n("Navigation") , this, Qt::TopToolBarArea, true, false, false); 

    m_historyBack = new KAction( KIcon("go-previous"), i18n("Back"), this);
    m_historyBackMenu = new KMenu(this);
    m_historyBack->setMenu(m_historyBackMenu);
    connect(m_historyBack, SIGNAL( triggered() ), this, SLOT( slotOpenPrevious() ) );
    connect(m_historyBackMenu, SIGNAL(aboutToShow()), this, SLOT(slotAboutToShowBackMenu()));
    connect(m_historyBackMenu, SIGNAL(triggered(QAction *)), this, SLOT(slotOpenActionUrl(QAction *)));
    m_navigationBar->addAction(m_historyBack);

    m_historyForward = new KAction( KIcon("go-next"), i18n("Forward"), this );
    connect(m_historyForward, SIGNAL( triggered() ), this, SLOT( slotOpenNext() ) );
    m_navigationBar->addAction(m_historyForward);

    m_stopReload = new KAction( KIcon("view-refresh"), i18n("Reload"), this);
    m_navigationBar->addAction(m_stopReload);

    m_goHome = new KAction( KIcon( "go-home" ), i18n("Home"),this);
    m_navigationBar->addAction(m_goHome);
    connect(m_goHome, SIGNAL(triggered()), this, SLOT(slotHome()));

    m_navigationBar->addWidget(m_tabWidget->lineEditStack());

    m_searchBar = new SearchBar(m_navigationBar);
    m_navigationBar->addWidget(m_searchBar);
    connect(m_searchBar, SIGNAL(search(const KUrl&)), this, SLOT(loadUrl(const KUrl&)));

    // UI settings
    m_navigationBar->setIconDimensions(16);
    m_navigationBar->setContextMenuPolicy( Qt::NoContextMenu );
    KToolBar::setToolBarsLocked( true ); 
}


void BrowserMainWindow::updateStatusbarActionText(bool visible)
{
    m_viewStatusbar->setText(!visible ? i18n("Show Status Bar") : i18n("Hide Status Bar"));
}


void BrowserMainWindow::slotViewStatusbar()
{
    if (statusBar()->isVisible()) 
    {
        updateStatusbarActionText(false);
        statusBar()->close();
    } 
    else 
    {
        updateStatusbarActionText(true);
        statusBar()->show();
    }
    m_autoSaver->changeOccurred();
}


KUrl BrowserMainWindow::guessUrlFromString(const QString &string)
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


void BrowserMainWindow::loadUrl(const KUrl &url)
{
    loadPage( url.url() );
}


void BrowserMainWindow::slotDownloadManager()
{
    BrowserApplication::downloadManager()->show();
}


void BrowserMainWindow::slotSelectLineEdit()
{
    m_tabWidget->currentLineEdit()->selectAll();
    m_tabWidget->currentLineEdit()->setFocus();
}


void BrowserMainWindow::slotFileSaveAs()
{
    BrowserApplication::downloadManager()->download(currentTab()->url(), true);
}


void BrowserMainWindow::slotPreferences()
{
    SettingsDialog *s = new SettingsDialog(this);
    s->show();
}


void BrowserMainWindow::slotUpdateStatusbar(const QString &string)
{
    statusBar()->showMessage(string, 2000);
}


void BrowserMainWindow::slotUpdateWindowTitle(const QString &title)
{
    if (title.isEmpty()) {
        setWindowTitle("reKonq");
    } else {
        setWindowTitle(title + " - reKonq");
    }
}


void BrowserMainWindow::slotFileNew()
{
    BrowserApplication::instance()->newMainWindow();
    BrowserMainWindow *mw = BrowserApplication::instance()->mainWindow();
    mw->slotHome();
}


void BrowserMainWindow::slotFileOpen()
{
    QString file = QFileDialog::getOpenFileName(this, i18n("Open Web Resource"), QString(),
            i18n("Web Resources (*.html *.htm *.svg *.png *.gif *.svgz);;All files (*.*)"));

    if (file.isEmpty())
        return;

    loadPage(file);
}


void BrowserMainWindow::slotFilePrintPreview()
{
    if (!currentTab())
        return;
    QPrintPreviewDialog *dialog = new QPrintPreviewDialog(this);
    connect(dialog, SIGNAL(paintRequested(QPrinter *)), currentTab(), SLOT(print(QPrinter *)));
    dialog->exec();
}


void BrowserMainWindow::slotFilePrint()
{
    if (!currentTab())
        return;
    printRequested(currentTab()->page()->mainFrame());
}


void BrowserMainWindow::printRequested(QWebFrame *frame)
{
    QPrinter printer;
    QPrintDialog *dialog = new QPrintDialog(&printer, this);
    dialog->setWindowTitle( i18n("Print Document") );
    if (dialog->exec() != QDialog::Accepted )
        return;
    frame->print(&printer);
}


void BrowserMainWindow::slotPrivateBrowsing()
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

        QList<BrowserMainWindow*> windows = BrowserApplication::instance()->mainWindows();
        for (int i = 0; i < windows.count(); ++i) 
        {
            BrowserMainWindow *window = windows.at(i);
            window->m_lastSearch = QString::null;
            window->tabWidget()->clear();
        }
    }
}


void BrowserMainWindow::closeEvent(QCloseEvent *event)
{
    if (m_tabWidget->count() > 1) 
    {
        int ret = KMessageBox::warningYesNo(this, 
                                                         i18n("Are you sure you want to close the window?" "  There are %1 tab open" , m_tabWidget->count() ) ,
                                                        QString() );
        if (ret == KMessageBox::No) 
        {
            event->ignore();
            return;
        }
    }
    event->accept();
    deleteLater();
}


void BrowserMainWindow::slotFind(const QString & search)
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


void BrowserMainWindow::slotViewFindBar()
{
    m_findBar->showFindBar();
}


void BrowserMainWindow::slotFindNext()
{
    if (!currentTab() && !m_lastSearch.isEmpty())
        return;
    currentTab()->findText(m_lastSearch);
}


void BrowserMainWindow::slotFindPrevious()
{
    if (!currentTab() && !m_lastSearch.isEmpty())
        return;
    currentTab()->findText(m_lastSearch, QWebPage::FindBackward);
}


void BrowserMainWindow::slotViewTextBigger()
{
    if (!currentTab())
        return;
    currentTab()->setTextSizeMultiplier(currentTab()->textSizeMultiplier() + 0.1);
}


void BrowserMainWindow::slotViewTextNormal()
{
    if (!currentTab())
        return;
    currentTab()->setTextSizeMultiplier(1.0);
}


void BrowserMainWindow::slotViewTextSmaller()
{
    if (!currentTab())
        return;
    currentTab()->setTextSizeMultiplier(currentTab()->textSizeMultiplier() - 0.1);
}


// TODO improve this
void BrowserMainWindow::slotViewFullScreen(bool makeFullScreen)
{
    KToggleFullScreenAction::setFullScreen( this, makeFullScreen );
}


void BrowserMainWindow::slotViewPageSource()
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


void BrowserMainWindow::slotHome()
{
    KConfig config("rekonqrc");
    KConfigGroup group = config.group("Global Settings");   
    QString home = group.readEntry( QString("home"), QString("http://www.kde.org/") );
    loadPage(home);
}


void BrowserMainWindow::slotToggleInspector(bool enable)
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


void BrowserMainWindow::slotSwapFocus()
{
    if (currentTab()->hasFocus())
        m_tabWidget->currentLineEdit()->setFocus();
    else
        currentTab()->setFocus();
}


void BrowserMainWindow::loadPage(const QString &page)
{
    if (!currentTab() || page.isEmpty())
        return;

    KUrl url = guessUrlFromString(page);
    m_tabWidget->currentLineEdit()->setText( url.prettyUrl() );
    m_tabWidget->loadUrlInCurrentTab(url);
}


TabWidget *BrowserMainWindow::tabWidget() const
{
    return m_tabWidget;
}



WebView *BrowserMainWindow::currentTab() const
{
    return m_tabWidget->currentWebView();
}


void BrowserMainWindow::slotLoadProgress(int progress)
{
    if (progress < 100 && progress > 0) 
    {
        disconnect(m_stopReload, SIGNAL(triggered()), m_reload, SLOT(trigger()));
        if (m_stopIcon.isNull())
            m_stopIcon = KIcon( "process-stop" );
        m_stopReload->setIcon(m_stopIcon);
        connect(m_stopReload, SIGNAL(triggered()), m_stop, SLOT(trigger()));
        m_stopReload->setToolTip( i18n("Stop loading the current page") );
    } 
    else 
    {
        disconnect(m_stopReload, SIGNAL(triggered()), m_stop, SLOT(trigger()));
        m_stopReload->setIcon( KIcon("view-refresh") );
        connect(m_stopReload, SIGNAL(triggered()), m_reload, SLOT(trigger()));
        m_stopReload->setToolTip( i18n("Reload the current page") );
    }
}


void BrowserMainWindow::slotAboutToShowBackMenu()
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


void BrowserMainWindow::slotShowWindow()
{
    if (KAction *action = qobject_cast<KAction*>(sender())) 
    {
        QVariant v = action->data();
        if (v.canConvert<int>()) 
        {
            int offset = qvariant_cast<int>(v);
            QList<BrowserMainWindow*> windows = BrowserApplication::instance()->mainWindows();
            windows.at(offset)->activateWindow();
            windows.at(offset)->currentTab()->setFocus();
        }
    }
}


void BrowserMainWindow::slotOpenActionUrl(QAction *action)
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


void BrowserMainWindow::slotOpenPrevious()
{
    QWebHistory *history = currentTab()->history();
    if ( history->canGoBack() )
        history->goToItem( history->backItem() );
}


void BrowserMainWindow::slotOpenNext()
{
    QWebHistory *history = currentTab()->history();
    if ( history->canGoForward() )
        history->goToItem( history->forwardItem() );
}


void BrowserMainWindow::geometryChangeRequested(const QRect &geometry)
{
    setGeometry(geometry);
}

