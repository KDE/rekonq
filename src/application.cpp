/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2008-2010 by Andrea Diamantini <adjam7 at gmail dot com>
* Copyright (C) 2009 by Paweł Prażak <pawelprazak at gmail dot com>
* Copyright (C) 2009-2010 by Lionel Chauvin <megabigbug@yahoo.fr>
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
#include "application.h"
#include "application.moc"

// Auto Includes
#include "rekonq.h"

// Local Includes
#include "adblockmanager.h"
#include "bookmarkprovider.h"
#include "filterurljob.h"
#include "historymanager.h"
#include "iconmanager.h"
#include "mainview.h"
#include "mainwindow.h"
#include "messagebar.h"
#include "opensearchmanager.h"
#include "sessionmanager.h"
#include "stackedurlbar.h"
#include "tabbar.h"
#include "urlbar.h"
#include "webtab.h"

// KDE Includes
#include <KCmdLineArgs>
#include <KIcon>
#include <KMessageBox>
#include <KStandardDirs>
#include <ThreadWeaver/Weaver>

// Qt Includes
#include <QVBoxLayout>

QWeakPointer<AdBlockManager> Application::s_adblockManager;
QWeakPointer<BookmarkProvider> Application::s_bookmarkProvider;
QWeakPointer<HistoryManager> Application::s_historyManager;
QWeakPointer<IconManager> Application::s_iconManager;
QWeakPointer<OpenSearchManager> Application::s_opensearchManager;
QWeakPointer<SessionManager> Application::s_sessionManager;

using namespace ThreadWeaver;

Application::Application()
        : KUniqueApplication()
{
    connect(Weaver::instance(), SIGNAL(jobDone(ThreadWeaver::Job*)),
            this, SLOT(loadResolvedUrl(ThreadWeaver::Job*)));
}


Application::~Application()
{
    // ok, we are closing well.
    // Don't recover on next load..
    ReKonfig::setRecoverOnCrash(0);
    saveConfiguration();

    foreach(QWeakPointer<MainWindow> window, m_mainWindows)
    {
        delete window.data();
        window.clear();
    }

    delete s_bookmarkProvider.data();
    s_bookmarkProvider.clear();

    delete s_historyManager.data();
    s_historyManager.clear();

    delete s_sessionManager.data();
    s_sessionManager.clear();

    delete s_adblockManager.data();
    s_adblockManager.clear();

    delete s_opensearchManager.data();
    s_opensearchManager.clear();
}


int Application::newInstance()
{
    KCmdLineArgs::setCwd(QDir::currentPath().toUtf8());
    KCmdLineArgs* args = KCmdLineArgs::parsedArgs();

    // not that easy, indeed
    // We have to consider 3 variables here:
    // 1) Is first load?
    // 2) Are there arguments?
    // 3) Is rekonq recovering from crash?
    // so, we have 8 possible cases...
    bool isFirstLoad = m_mainWindows.isEmpty();
    bool areThereArguments = (args->count() > 0);
    bool isRekonqCrashed = ReKonfig::recoverOnCrash();

    kDebug() << "is first load? " << isFirstLoad;
    kDebug() << "are there arguments? " << areThereArguments;
    kDebug() << "is rekonq crashed? " << isRekonqCrashed;

    int exitValue = 1 * isFirstLoad + 2 * areThereArguments + 4 * isRekonqCrashed;

    if (isRekonqCrashed && isFirstLoad) {
            loadUrl(KUrl("about:closedTabs"));
            MessageBar *msgBar = new MessageBar(i18n("It seems rekonq was not closed properly. Do you want "
                                                     "to restore the last saved session?")
                                                , mainWindow()->currentTab()
                                                , QMessageBox::Warning
                                                , MessageBar::Yes | MessageBar::No );

            connect(msgBar, SIGNAL(accepted()), sessionManager(), SLOT(restoreSession()));
            mainWindow()->currentTab()->insertBar(msgBar);
    }

    if (areThereArguments) {
        KUrl::List urlList;
        for(int i = 0; i < args->count(); ++i)
        {
            const KUrl u = args->url(i);
            if (u.isLocalFile() && QFile::exists(u.toLocalFile())) // "rekonq somefile.html" case
                urlList += u;
            else
                urlList += KUrl( args->arg(i) ); // "rekonq kde.org" || "rekonq kde:kdialog" case
        }

        if (isFirstLoad && !isRekonqCrashed) {
            // No windows in the current desktop? No windows at all?
            // Create a new one and load there sites...
            loadUrl(urlList.at(0), Rekonq::CurrentTab);
        }
        else
        {
            if(ReKonfig::openTabNoWindow())
                loadUrl(urlList.at(0), Rekonq::NewTab);
            else
                loadUrl(urlList.at(0), Rekonq::NewWindow);
        }
        
        for (int i = 1; i < urlList.count(); ++i)
            loadUrl( urlList.at(i), Rekonq::NewTab);

    } else if (!isRekonqCrashed) {

        if (isFirstLoad)  // we are starting rekonq, for the first time with no args: use startup behaviour
        {
            switch (ReKonfig::startupBehaviour())
            {
            case 0: // open home page
                mainWindow()->homePage();
                break;
            case 1: // open new tab page
                loadUrl(KUrl("about:home"));
                break;
            case 2: // restore session
                sessionManager()->restoreSession();
                kDebug() << "session restored following settings";
                break;
            default:
                mainWindow()->homePage();
                break;
            }
        }
        else    // rekonq has just been started. Just open a new window
        {
            switch (ReKonfig::newTabsBehaviour())
            {
            case 0: // new tab page
                loadUrl(KUrl("about:home") , Rekonq::NewWindow);
                break;
            case 1: // blank page
                loadUrl(KUrl("about:blank") , Rekonq::NewWindow);
                break;
            case 2: // homepage
                loadUrl(KUrl(ReKonfig::homePage()) , Rekonq::NewWindow);
                break;
           default:
                loadUrl(KUrl("about:blank") , Rekonq::NewWindow);
                break;
           }

        }
    }

    if(isFirstLoad)
    {
        // give me some time to do the other things..
        QTimer::singleShot(100, this, SLOT(postLaunch()));
    }

    return exitValue;
}


Application *Application::instance()
{
    return (qobject_cast<Application *>(QCoreApplication::instance()));
}


void Application::postLaunch()
{
    // updating rekonq configuration
    updateConfiguration();

    setWindowIcon(KIcon("rekonq"));

    Application::historyManager();
    Application::sessionManager()->setSessionManagementEnabled(true);

    // bookmarks loading
    connect(Application::bookmarkProvider(), SIGNAL(openUrl(const KUrl&, const Rekonq::OpenType&)),
            Application::instance(), SLOT(loadUrl(const KUrl&, const Rekonq::OpenType&)));

    // crash recovering
    ReKonfig::setRecoverOnCrash(ReKonfig::recoverOnCrash() + 1);
    saveConfiguration();
}


void Application::saveConfiguration() const
{
    ReKonfig::self()->writeConfig();
}


MainWindow *Application::mainWindow()
{
    if (m_mainWindows.isEmpty())
        return newMainWindow();

    MainWindow *active = qobject_cast<MainWindow*>(QApplication::activeWindow());

    if (!active)
    {
        return m_mainWindows.at(0).data();
    }
    return active;
}


HistoryManager *Application::historyManager()
{
    if (s_historyManager.isNull())
    {
        s_historyManager = new HistoryManager();
        QWebHistoryInterface::setDefaultInterface(s_historyManager.data());
    }
    return s_historyManager.data();
}


BookmarkProvider *Application::bookmarkProvider()
{
    if (s_bookmarkProvider.isNull())
    {
        s_bookmarkProvider = new BookmarkProvider(instance());
    }
    return s_bookmarkProvider.data();
}


SessionManager *Application::sessionManager()
{
    if (s_sessionManager.isNull())
    {
        s_sessionManager = new SessionManager(instance());
    }
    return s_sessionManager.data();
}


OpenSearchManager *Application::opensearchManager()
{
    if (s_opensearchManager.isNull())
    {
        s_opensearchManager = new OpenSearchManager(instance());
        s_opensearchManager.data()->setSearchProvider("google"); //TODO: use other suggestion engines
    }
    return s_opensearchManager.data();
}


IconManager *Application::iconManager()
{
    if (s_iconManager.isNull())
    {
        s_iconManager = new IconManager(instance());
    }
    return s_iconManager.data();
}


void Application::loadUrl(const KUrl& url, const Rekonq::OpenType& type)
{
    if (url.isEmpty())
        return;

    if (!url.isValid())
    {
        KMessageBox::error(0, i18n("Malformed URL:\n%1", url.url(KUrl::RemoveTrailingSlash)));
        return;
    }

    // first, create the webview(s) to not let hangs UI..
    WebTab *tab = 0;
    MainWindow *w = 0;
    w = (type == Rekonq::NewWindow)
        ? newMainWindow()
        : mainWindow();

    switch (type)
    {
    case Rekonq::NewTab:
        if( ReKonfig::openTabNoWindow() )
            tab = w->mainView()->newWebTab( !ReKonfig::openTabsBack() );
        else
        {
            w = newMainWindow();
            tab = w->mainView()->currentWebTab();
        }
        break;
    case Rekonq::NewFocusedTab:
        tab = w->mainView()->newWebTab(true);
        break;
    case Rekonq::NewBackTab:
        tab = w->mainView()->newWebTab(false);
        break;
    case Rekonq::NewWindow:
    case Rekonq::CurrentTab:
        tab = w->mainView()->currentWebTab();
        break;
    };


    // rapidly show first loading url..
    int tabIndex = w->mainView()->indexOf(tab);
    Q_ASSERT( tabIndex != -1 );
    UrlBar *barForTab = qobject_cast<UrlBar *>(w->mainView()->widgetBar()->widget(tabIndex));
    barForTab->activateSuggestions(false);
    barForTab->setQUrl(url);

    WebView *view = tab->view();

    if (view)
    {
        FilterUrlJob *job = new FilterUrlJob(view, url.pathOrUrl(), this);
        Weaver::instance()->enqueue(job);
    }
}


MainWindow *Application::newMainWindow(bool withTab)
{
    MainWindow *w = new MainWindow();

    if (withTab)
        w->mainView()->newWebTab();    // remember using newWebTab and NOT newTab here!!

    m_mainWindows.prepend(w);
    w->show();

    return w;
}


void Application::removeMainWindow(MainWindow *window)
{
    m_mainWindows.removeOne(window);
}


MainWindowList Application::mainWindowList()
{
    return m_mainWindows;
}


AdBlockManager *Application::adblockManager()
{
    if (s_adblockManager.isNull())
    {
        s_adblockManager = new AdBlockManager(instance());
    }
    return s_adblockManager.data();
}


void Application::loadResolvedUrl(ThreadWeaver::Job *job)
{
    FilterUrlJob *threadedJob = static_cast<FilterUrlJob *>(job);
    KUrl url = threadedJob->url();
    WebView *view = threadedJob->view();

    if (view)
    {
        view->load(url);
    }

    // Bye and thanks :)
    delete threadedJob;
}


void Application::newWindow()
{
    loadUrl(KUrl("about:home"), Rekonq::NewWindow);
    mainWindow()->mainView()->urlBar()->setFocus();
}


void Application::updateConfiguration()
{
    // ============== Tabs ==================
    bool b = ReKonfig::closeTabSelectPrevious();
    Q_FOREACH(const QWeakPointer<MainWindow> &w, m_mainWindows)
    {
        MainView *mv = w.data()->mainView();
        mv->updateTabBar();

        if (b)
            mv->tabBar()->setSelectionBehaviorOnRemove(QTabBar::SelectPreviousTab);
        else
            mv->tabBar()->setSelectionBehaviorOnRemove(QTabBar::SelectRightTab);
    }

    QWebSettings *defaultSettings = QWebSettings::globalSettings();

    // =========== Fonts ==============
    defaultSettings->setFontFamily(QWebSettings::StandardFont, ReKonfig::standardFontFamily() );
    defaultSettings->setFontFamily(QWebSettings::FixedFont, ReKonfig::fixedFontFamily() );
    defaultSettings->setFontFamily(QWebSettings::SerifFont, ReKonfig::serifFontFamily() );
    defaultSettings->setFontFamily(QWebSettings::SansSerifFont, ReKonfig::sansSerifFontFamily() );
    defaultSettings->setFontFamily(QWebSettings::CursiveFont, ReKonfig::cursiveFontFamily());
    defaultSettings->setFontFamily(QWebSettings::FantasyFont, ReKonfig::fantasyFontFamily());

    // compute font size
    // (I have to admit I know nothing about these DPI questions..: copied from kwebkitpart, as someone suggested)
    // font size in pixels =  font size in inches × screen dpi
    int defaultFontSize = ReKonfig::defaultFontSize();
    int minimumFontSize = ReKonfig::minFontSize();

    int logDpiY = mainWindow()->currentTab()->view()->logicalDpiY();
    kDebug() << "Logical Dot per Inch Y: " << logDpiY;

    float toPix = (logDpiY < 96.0)
        ? 96.0/72.0
        : logDpiY/72.0 ;

    defaultSettings->setFontSize(QWebSettings::DefaultFontSize, qRound(defaultFontSize * toPix) );
    defaultSettings->setFontSize(QWebSettings::MinimumFontSize, qRound(minimumFontSize * toPix) );


    // ================ WebKit ============================
    defaultSettings->setAttribute(QWebSettings::AutoLoadImages, ReKonfig::autoLoadImages());
    defaultSettings->setAttribute(QWebSettings::DnsPrefetchEnabled, ReKonfig::dnsPrefetch());
    defaultSettings->setAttribute(QWebSettings::JavascriptEnabled, ReKonfig::javascriptEnabled());
    defaultSettings->setAttribute(QWebSettings::JavaEnabled, ReKonfig::javaEnabled());
    defaultSettings->setAttribute(QWebSettings::JavascriptCanOpenWindows, ReKonfig::javascriptCanOpenWindows());
    defaultSettings->setAttribute(QWebSettings::JavascriptCanAccessClipboard, ReKonfig::javascriptCanAccessClipboard());
    defaultSettings->setAttribute(QWebSettings::LinksIncludedInFocusChain, ReKonfig::linksIncludedInFocusChain());
    defaultSettings->setAttribute(QWebSettings::ZoomTextOnly, ReKonfig::zoomTextOnly());
    defaultSettings->setAttribute(QWebSettings::PrintElementBackgrounds, ReKonfig::printElementBackgrounds());

    if (ReKonfig::pluginsEnabled() == 2)
        defaultSettings->setAttribute(QWebSettings::PluginsEnabled, false);
    else
        defaultSettings->setAttribute(QWebSettings::PluginsEnabled, true);

    // Enabling WebKit "Page Cache" feature: http://webkit.org/blog/427/webkit-page-cache-i-the-basics/
    defaultSettings->setMaximumPagesInCache(3);
    
    // ===== HTML 5 features WebKit support ======
    defaultSettings->setAttribute(QWebSettings::OfflineStorageDatabaseEnabled, ReKonfig::offlineStorageDatabaseEnabled());
    defaultSettings->setAttribute(QWebSettings::OfflineWebApplicationCacheEnabled, ReKonfig::offlineWebApplicationCacheEnabled());
    defaultSettings->setAttribute(QWebSettings::LocalStorageEnabled, ReKonfig::localStorageEnabled());
    if (ReKonfig::localStorageEnabled())
    {
        QString path = KStandardDirs::locateLocal("cache", QString("WebkitLocalStorage/rekonq"), true);
        path.remove("rekonq");
        QWebSettings::setOfflineStoragePath(path);
        QWebSettings::setOfflineStorageDefaultQuota(50000);
    }

    // Applies user defined CSS to all open webpages. If there no longer is a
    // user defined CSS removes it from all open webpages.
    if (!ReKonfig::userCSS().isEmpty())
        defaultSettings->setUserStyleSheetUrl(ReKonfig::userCSS());

    // ====== load Settings on main classes
    Application::historyManager()->loadSettings();
    Application::adblockManager()->loadSettings();

    defaultSettings = 0;
}


void Application::addDownload(const QString &srcUrl, const QString &destUrl)
{
    QWebSettings *globalSettings = QWebSettings::globalSettings();
    if (globalSettings->testAttribute(QWebSettings::PrivateBrowsingEnabled))
        return;
    QString downloadFilePath = KStandardDirs::locateLocal("appdata" , "downloads");
    QFile downloadFile(downloadFilePath);
    if (!downloadFile.open(QFile::WriteOnly | QFile::Append))
    {
        kDebug() << "Unable to open download file (WRITE mode)..";
        return;
    }
    QDataStream out(&downloadFile);
    out << srcUrl;
    out << destUrl;
    out << QDateTime::currentDateTime();
    downloadFile.close();
}


DownloadList Application::downloads()
{
    DownloadList list;

    QString downloadFilePath = KStandardDirs::locateLocal("appdata" , "downloads");
    QFile downloadFile(downloadFilePath);
    if (!downloadFile.open(QFile::ReadOnly))
    {
        kDebug() << "Unable to open download file (READ mode)..";
        return list;
    }

    QDataStream in(&downloadFile);
    while (!in.atEnd())
    {
        QString srcUrl;
        in >> srcUrl;
        QString destUrl;
        in >> destUrl;
        QDateTime dt;
        in >> dt;
        DownloadItem item(srcUrl, destUrl, dt);
        list << item;
    }
    return list;
}


bool Application::clearDownloadsHistory()
{
    QString downloadFilePath = KStandardDirs::locateLocal("appdata" , "downloads");
    QFile downloadFile(downloadFilePath);
    return downloadFile.remove();
}
