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
#include "mainwindow.h"
#include "historymanager.h"
#include "bookmarksmanager.h"
#include "mainview.h"
#include "webtab.h"
#include "urlbar.h"
#include "sessionmanager.h"
#include "adblockmanager.h"
#include "webview.h"
#include "filterurljob.h"
#include "tabbar.h"

// KDE Includes
#include <KCmdLineArgs>
#include <KStandardDirs>
#include <kio/job.h>
#include <kio/jobclasses.h>
#include <KUriFilter>
#include <KMessageBox>
#include <KUrl>
#include <ThreadWeaver/Weaver>

// Qt Includes
#include <QtCore/QTimer>


QWeakPointer<HistoryManager> Application::s_historyManager;
QWeakPointer<BookmarkProvider> Application::s_bookmarkProvider;
QWeakPointer<SessionManager> Application::s_sessionManager;
QWeakPointer<AdBlockManager> Application::s_adblockManager;


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
}


int Application::newInstance()
{
    KCmdLineArgs::setCwd(QDir::currentPath().toUtf8());
    KCmdLineArgs* args = KCmdLineArgs::parsedArgs();

    bool isFirstLoad = m_mainWindows.isEmpty();

    // is your app session restored? restore session...
    // this mechanism also falls back to load usual plain rekonq
    // if something goes wrong...
    if (isFirstLoad && ReKonfig::recoverOnCrash() == 1 && sessionManager()->restoreSession())
    {
        QTimer::singleShot(0, this, SLOT(postLaunch()));
        kDebug() << "session restored";
        return 1;
    }

    if (args->count() == 0)
    {
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
                if (sessionManager()->restoreSession())
                    break;
            default:
                mainWindow()->homePage();
                break;
            }
        }
        else    // rekonq has just been started. Just open a new window
        {
            loadUrl(KUrl("about:home") , Rekonq::NewWindow);
        }
    }
    else
    {
        if (isFirstLoad)
        {
            // No windows in the current desktop? No windows at all?
            // Create a new one and load there sites...
            loadUrl(args->arg(0), Rekonq::CurrentTab);
            for (int i = 1; i < args->count(); ++i)
                loadUrl(KUrl(args->arg(i)), Rekonq::SettingOpenTab);
        }
        else
        {
            // are there any windows there? use it
            int index = m_mainWindows.size();
            if (index > 0)
            {
                MainWindow *m = m_mainWindows.at(index - 1).data();
                m->activateWindow();
                for (int i = 0; i < args->count(); ++i)
                    loadUrl(KUrl(args->arg(i)), Rekonq::NewCurrentTab);
            }
        }
    }

    QTimer::singleShot(0, this, SLOT(postLaunch()));
    return 0;
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

    // set Icon Database Path to store "favicons" associated with web sites
    QString directory = KStandardDirs::locateLocal("cache" , "" , true);
    QWebSettings::setIconDatabasePath(directory);

    Application::historyManager();
    Application::sessionManager();

    // bookmarks loading
    connect(Application::bookmarkProvider(), SIGNAL(openUrl(const KUrl&, const Rekonq::OpenType&)),
            Application::instance(), SLOT(loadUrl(const KUrl&, const Rekonq::OpenType&)));

    // crash recovering
    int n = ReKonfig::recoverOnCrash();
    ReKonfig::setRecoverOnCrash(++n);
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


KIcon Application::icon(const KUrl &url)
{
    // avoid infinite loop at startup
    if (Application::instance()->mainWindowList().isEmpty())
        return KIcon("text-html");

    // first things first..
    if (url.isEmpty())
        return KIcon("text-html");

    // rekonq icons..
    if (url == KUrl("about:closedTabs"))
        return KIcon("tab-close");
    if (url == KUrl("about:history"))
        return KIcon("view-history");
    if (url == KUrl("about:bookmarks"))
        return KIcon("bookmarks");
    if (url == KUrl("about:favorites"))
        return KIcon("emblem-favorite");

    KIcon icon = KIcon(QWebSettings::iconForUrl(url));
    if (icon.isNull())
    {
        icon = KIcon("text-html");
    }
    return icon;
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
    case Rekonq::SettingOpenTab:
        tab = w->mainView()->newWebTab(!ReKonfig::openTabsBack(), ReKonfig::openTabsNearCurrent());
        break;
    case Rekonq::NewCurrentTab:
        tab = w->mainView()->newWebTab(true);
        break;
    case Rekonq::NewBackTab:
        tab = w->mainView()->newWebTab(false, ReKonfig::openTabsNearCurrent());
        break;
    case Rekonq::NewWindow:
    case Rekonq::CurrentTab:
        tab = w->mainView()->currentWebTab();
        break;
    };


    // rapidly show first loading url..
    w->mainView()->urlBar()->setQUrl(url);
    
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

    // Bye and thanks :)
    delete threadedJob;

    if (view)
    {
        view->load(url);

        // we are sure of the url now, let's add it to history
        // anyway we store here just http sites because local and ftp ones are
        // added trough the protocol handler and the other are ignored
        if (url.protocol() == QL1S("http") || url.protocol() == QL1S("https"))
            historyManager()->addHistoryEntry(url.prettyUrl());
    }
}


void Application::newWindow()
{
    loadUrl(KUrl("about:home"), Rekonq::NewWindow);
    mainWindow()->mainView()->urlBar()->setFocus();
}


void Application::updateConfiguration()
{
    // FIXME:
    // all things related to mainview can be
    // improved/moved/replicated in all the mainwindows
    MainView *view = mainWindow()->mainView();

    // ============== General ==================
    view->updateTabBar();

    // ============== Tabs ==================
    if (ReKonfig::closeTabSelectPrevious())
        view->tabBar()->setSelectionBehaviorOnRemove(QTabBar::SelectPreviousTab);
    else
        view->tabBar()->setSelectionBehaviorOnRemove(QTabBar::SelectRightTab);

    // =========== Fonts ==============
    QWebSettings *defaultSettings = QWebSettings::globalSettings();

    int fnSize = ReKonfig::fontSize();
    int minFnSize = ReKonfig::minFontSize();

    QFont standardFont = ReKonfig::standardFont();
    defaultSettings->setFontFamily(QWebSettings::StandardFont, standardFont.family());
    defaultSettings->setFontSize(QWebSettings::DefaultFontSize, fnSize);
    defaultSettings->setFontSize(QWebSettings::MinimumFontSize, minFnSize);

    QFont fixedFont = ReKonfig::fixedFont();
    defaultSettings->setFontFamily(QWebSettings::FixedFont, fixedFont.family());
    defaultSettings->setFontSize(QWebSettings::DefaultFixedFontSize, fnSize);

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
    if (ReKonfig::userCSS().isEmpty())
        defaultSettings->setUserStyleSheetUrl(KUrl(KStandardDirs::locate("appdata" , "default.css")));
    else
        defaultSettings->setUserStyleSheetUrl(ReKonfig::userCSS());

    // ====== load Settings on main classes
    Application::historyManager()->loadSettings();
    Application::adblockManager()->loadSettings();

    defaultSettings = 0;
}
