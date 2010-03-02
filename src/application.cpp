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

// KDE Includes
#include <KCmdLineArgs>
#include <KStandardDirs>
#include <kio/job.h>
#include <kio/jobclasses.h>
#include <KToolInvocation>
#include <KUriFilter>
#include <KMessageBox>
#include <KUrl>
#include <ThreadWeaver/Weaver>

// Qt Includes
#include <QRegExp>
#include <QFile>
#include <QFileInfo>
#include <QtCore/QTimer>


QWeakPointer<HistoryManager> Application::s_historyManager;
QWeakPointer<BookmarkProvider> Application::s_bookmarkProvider;
QWeakPointer<SessionManager> Application::s_sessionManager;
QWeakPointer<AdBlockManager> Application::s_adblockManager;


Application::Application()
    : KUniqueApplication()
{
    connect(Weaver::instance(), SIGNAL( jobDone(ThreadWeaver::Job*) ), 
            this, SLOT( loadResolvedUrl(ThreadWeaver::Job*) ) );
}


Application::~Application()
{
    foreach( QWeakPointer<MainWindow> window, m_mainWindows)
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
    KCmdLineArgs::setCwd( QDir::currentPath().toUtf8() );
    KCmdLineArgs* args = KCmdLineArgs::parsedArgs();
    
    // we share one process for several mainwindows,
    // so initialize only once
    static bool first = true;
    
    if(args->count() == 0)
    {
        if(first)   // we are starting rekonq, for the first time with no args: use startup behaviour
        {
            switch(ReKonfig::startupBehaviour())
            {
                case 0: // open home page
                    mainWindow()->homePage();
                    break;
                case 1: // open new tab page
                    loadUrl( KUrl("about:home") );
                    break;
                case 2: // restore session
                    if(sessionManager()->restoreSession())
                        break;
                default:
                    mainWindow()->homePage();
                    break;
            }    
        }
        else    // rekonq has just been started. Just open a new window
        {
            loadUrl( KUrl("about:home") , Rekonq::NewWindow );
        }
    }
    
    if (first)
    {
        QTimer::singleShot(0, this, SLOT(postLaunch()));
        first = false;
    }

    // is your app session restored? restore session...
    // this mechanism also falls back to load usual plain rekonq
    // if something goes wrong...
    if (isSessionRestored() && sessionManager()->restoreSession())
    {
        kDebug() << "session restored";
        return 1;
    }
    
    // are there args? load them..
    if (args->count() > 0)
    {
        // are there any windows there? use it
        int index = m_mainWindows.size();
        if(index > 0)
        {
            MainWindow *m = m_mainWindows.at(index - 1).data();
            m->activateWindow();
                
            for (int i = 0; i < args->count(); ++i)
                loadUrl(args->arg(i), Rekonq::NewCurrentTab);
                
            return 2;
        }
        
        // No windows in the current desktop? No windows at all?
        // Create a new one and load there sites...
        loadUrl(args->arg(0), Rekonq::CurrentTab);
        for (int i = 1; i < args->count(); ++i)
            loadUrl(args->arg(i), Rekonq::SettingOpenTab);

        return 3;
    }
    
    return 0;
}


Application *Application::instance()
{
    return (qobject_cast<Application *>(QCoreApplication::instance()));
}


void Application::postLaunch()
{
    setWindowIcon(KIcon("rekonq"));
    
    // set Icon Database Path to store "favicons" associated with web sites
    QString directory = KStandardDirs::locateLocal("cache" , "" , true);
    QWebSettings::setIconDatabasePath(directory);

    Application::historyManager();
    Application::sessionManager();
    
    // bookmarks loading
    connect(Application::bookmarkProvider(), SIGNAL(openUrl(const KUrl&, const Rekonq::OpenType&)),
            Application::instance(), SLOT(loadUrl(const KUrl&, const Rekonq::OpenType&)));
}


void Application::saveConfiguration() const
{
    ReKonfig::self()->writeConfig();
}


MainWindow *Application::mainWindow()
{
    if(m_mainWindows.isEmpty())
        return newMainWindow();
    
    MainWindow *active = qobject_cast<MainWindow*>(QApplication::activeWindow());
    
    if(!active)
    {
        return m_mainWindows.at(0).data();
    }
    return active;
}


HistoryManager *Application::historyManager()
{
    if ( s_historyManager.isNull() )
    {
        s_historyManager = new HistoryManager();
        QWebHistoryInterface::setDefaultInterface( s_historyManager.data() );
    }
    return s_historyManager.data();
}


BookmarkProvider *Application::bookmarkProvider()
{
    if ( s_bookmarkProvider.isNull() )
    {
        s_bookmarkProvider = new BookmarkProvider(instance());
    }
    return s_bookmarkProvider.data();
}


SessionManager *Application::sessionManager()
{
    if( s_sessionManager.isNull() )
    {
        s_sessionManager = new SessionManager(instance());
    }
    return s_sessionManager.data();
}


KIcon Application::icon(const KUrl &url)
{
    // avoid infinite loop at startup
    if( Application::instance()->mainWindowList().isEmpty() )
        return KIcon("text-html");

    // first things first..
    if(url.isEmpty())
        return KIcon("text-html");

    // rekonq icons..
    if(url == KUrl("about:closedTabs"))
        return KIcon("tab-close");
    if(url == KUrl("about:history"))
        return KIcon("view-history");
    if(url == KUrl("about:bookmarks"))
        return KIcon("bookmarks");
    if(url == KUrl("about:favorites"))
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

    // sanitization
    KUrl loadingUrl( url.toEncoded() );

    if ( !loadingUrl.isValid() )
    {
        KMessageBox::error(0, i18n("Malformed URL:\n%1", loadingUrl.url(KUrl::RemoveTrailingSlash)));
        return;
    }

    // first, create the webview(s) to not let hangs UI..
    WebTab *tab = 0;
    MainWindow *w = 0;
    w = (type == Rekonq::NewWindow) 
        ? newMainWindow()
        : mainWindow();
        
    switch(type)
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
    
    WebView *view = tab->view();
    
    if (view)
    {
        FilterUrlJob *job = new FilterUrlJob(view, loadingUrl.pathOrUrl(), this);
        Weaver::instance()->enqueue(job);
    }
}



void Application::loadUrl(const QString& urlString,  const Rekonq::OpenType& type)
{    
    return loadUrl( QUrl::fromUserInput(urlString), type );
}


MainWindow *Application::newMainWindow()
{
    MainWindow *w = new MainWindow();
    w->mainView()->newWebTab();    // remember using newWebTab and NOT newTab here!!
    
    m_mainWindows.prepend(w);
    w->show();
    
    return w;
}


void Application::removeMainWindow(MainWindow *window)
{
    m_mainWindows.removeAt(m_mainWindows.indexOf(window, 0));
}


MainWindowList Application::mainWindowList()
{
    return m_mainWindows;
}


AdBlockManager *Application::adblockManager()
{
    if( s_adblockManager.isNull() )
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
        if( url.protocol() == QLatin1String("http") || url.protocol() == QLatin1String("https") )
            historyManager()->addHistoryEntry( url.prettyUrl() );
    }
}


void Application::newWindow()
{
    loadUrl( KUrl("about:home"), Rekonq::NewWindow );
    mainWindow()->mainView()->urlBar()->setFocus();
}
