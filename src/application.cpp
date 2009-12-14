/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2008-2009 by Andrea Diamantini <adjam7 at gmail dot com>
* Copyright (C) 2009 by Paweł Prażak <pawelprazak at gmail dot com>
* Copyright (C) 2009 by Lionel Chauvin <megabigbug@yahoo.fr>
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

// KDE Includes
#include <KCmdLineArgs>
#include <KStandardDirs>
#include <kio/job.h>
#include <kio/jobclasses.h>
#include <KToolInvocation>
#include <KUriFilter>
#include <KMessageBox>
#include <KWindowInfo>
#include <KUrl>

// Qt Includes
#include <QRegExp>
#include <QFile>
#include <QFileInfo>
#include <QtCore/QTimer>


QPointer<HistoryManager> Application::s_historyManager;
QPointer<BookmarkProvider> Application::s_bookmarkProvider;
QPointer<SessionManager> Application::s_sessionManager;
QPointer<AdBlockManager> Application::s_adblockManager;


Application::Application()
        : KUniqueApplication()
{
}


Application::~Application()
{
    qDeleteAll(m_mainWindows);
    delete s_bookmarkProvider;
    delete s_historyManager;
}


int Application::newInstance()
{
    KCmdLineArgs::setCwd(QDir::currentPath().toUtf8());
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
                    mainWindow()->newTabPage();
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
            newMainWindow();
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
        // is there a window open on the current desktop ? use it!
        for (int i = 0; i < m_mainWindows.size(); ++i)
        {
            MainWindow *m = m_mainWindows.at(i);
            KWindowInfo w = KWindowInfo(m->winId(), NET::WMDesktop);
            if(w.isOnCurrentDesktop())
            {
                m->activateWindow();
                m->raise();
                
                for (int i = 0; i < args->count(); ++i)
                    loadUrl(args->arg(i), Rekonq::NewCurrentTab);
                
                return 2;
            }
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
        return m_mainWindows.at(0);
    }
    return active;
}


HistoryManager *Application::historyManager()
{
    if (!s_historyManager)
    {
        s_historyManager = new HistoryManager();
        QWebHistoryInterface::setDefaultInterface(s_historyManager);
    }
    return s_historyManager;
}


BookmarkProvider *Application::bookmarkProvider()
{
    if (!s_bookmarkProvider)
    {
        s_bookmarkProvider = new BookmarkProvider(instance());
    }
    return s_bookmarkProvider;
}


SessionManager *Application::sessionManager()
{
    if(!s_sessionManager)
    {
        s_sessionManager = new SessionManager(instance());
    }
    return s_sessionManager;
}


KIcon Application::icon(const KUrl &url)
{
    if(!Application::instance()->mainWindowList().isEmpty()) // avoid infinite loop at startup
    {

        if(url == KUrl("about:closedTabs"))
            return KIcon("tab-close");
        if(url == KUrl("about:history"))
            return KIcon("view-history");
        if(url == KUrl("about:bookmarks"))
            return KIcon("bookmarks");
        if(url == KUrl("about:home") || url == KUrl("about:favorites"))
            return KIcon("emblem-favorite");
    }
    
    if(url.isEmpty())
        return KIcon("text-html");
    
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

    KUrl loadingUrl = xssSanitization(url);

    if ( !loadingUrl.isValid() )
    {
        KMessageBox::error(0, i18n("Malformed URL:\n%1", loadingUrl.url(KUrl::RemoveTrailingSlash)));
        return;
    }

    // loading home pages
    if (mainWindow()->newTabPage(loadingUrl))
        return;
    
    if (loadingUrl.scheme() == QLatin1String("mailto"))
    {
        KToolInvocation::invokeMailer(loadingUrl);
        return;
    }

    // first, create the webview(s) to not let hangs UI..
    WebTab *webView = 0;
    MainWindow *w = 0;
    if(type == Rekonq::NewWindow)
        w = newMainWindow();
    else
        w = mainWindow();
        
    switch(type)
    {
    case Rekonq::SettingOpenTab:
        webView = w->mainView()->newWebTab(!ReKonfig::openTabsBack(),
                                           ReKonfig::openTabsNearCurrent());
        break;
    case Rekonq::NewCurrentTab:
        webView = w->mainView()->newWebTab(true);
        break;
    case Rekonq::NewBackTab:
        webView = w->mainView()->newWebTab(false, ReKonfig::openTabsNearCurrent());
        break;
    case Rekonq::NewWindow:
    case Rekonq::CurrentTab:
        webView = w->mainView()->currentWebTab();
        break;
    };

    // WARNING this is just a temporary hack waiting for the new "awesome bar" (rekonq 0.4 target)
    // and it may not work in all cases.
    // Known issues are (add that here to test the awesome bar code):
    // - web shortcuts with space separator
    // - relative urls
    // - ...
    if (loadingUrl.isRelative())
    {
        QString fn = loadingUrl.url(KUrl::RemoveTrailingSlash);
        if(loadingUrl.path().contains('.') && !loadingUrl.path().contains(' '))
        {
            loadingUrl.setUrl("//" + fn);
            loadingUrl.setScheme("http");
        }
        else
        {
            loadingUrl.setUrl(url.fileName());
            loadingUrl.setScheme("gg");
        }
    }

    // this should let rekonq filtering URI info and supporting
    // the beautiful KDE web browsing shortcuts
    KUriFilterData data(loadingUrl.pathOrUrl());
    data.setCheckForExecutables(false); // if true, queries like "rekonq" or "dolphin" 
                                        // are considered as executables
    if (KUriFilter::self()->filterUri(data))
    {
        loadingUrl = data.uri().url();
    }
    // ------------------- END WARNING --------------------------------------

    // we are sure of the url now, let's add it to history
    historyManager()->addHistoryEntry( loadingUrl.prettyUrl() );
    
    if (!ReKonfig::openTabsBack())
    {
        w->mainView()->urlBar()->setUrl(loadingUrl.prettyUrl());
    }
    
    if (webView)
    {
        webView->setFocus();
        webView->view()->load(loadingUrl);
    }
}


void Application::loadUrl(const QString& urlString,  const Rekonq::OpenType& type)
{    
    return loadUrl( QUrl::fromUserInput(urlString), type );
}


MainWindow *Application::newMainWindow()
{
    MainWindow *w = new MainWindow();
    w->mainView()->newWebTab();    // remember using newWebView and NOT newTab here!!
    
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
    if(!s_adblockManager)
    {
        s_adblockManager = new AdBlockManager(instance());
    }
    return s_adblockManager;
}


KUrl Application::xssSanitization(const KUrl &url)
{
    QString urlString = url.url();
    
    QList<QChar> l;     // TODO: learn regular expression
    l << '\'' << '\"' << '<' << '>';
    foreach(const QChar &c, l)
    {
        QStringList list = urlString.split(c);
        urlString = list.at(0);
    }
    return KUrl(urlString);
}
    