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
#include "cookiejar.h"
#include "history.h"
#include "networkaccessmanager.h"
#include "mainview.h"
#include "webview.h"
#include "urlbar.h"
#include "sessionmanager.h"
#include "homepage.h"

// KDE Includes
#include <KCmdLineArgs>
#include <KStandardDirs>
#include <kio/job.h>
#include <kio/jobclasses.h>
#include <KToolInvocation>
#include <KUriFilter>
#include <KMessageBox>
#include <KProtocolInfo>
#include <KWindowInfo>

// Qt Includes
#include <QRegExp>
#include <QFile>
#include <QFileInfo>
#include <QtCore/QTimer>


QPointer<HistoryManager> Application::s_historyManager;
QPointer<NetworkAccessManager> Application::s_networkAccessManager;
QPointer<BookmarkProvider> Application::s_bookmarkProvider;
QPointer<SessionManager> Application::s_sessionManager;



Application::Application()
        : KUniqueApplication()
{
}


Application::~Application()
{
    qDeleteAll(m_mainWindows);
    delete s_bookmarkProvider;
    delete s_networkAccessManager;
    delete s_historyManager;
}


int Application::newInstance()
{
    KCmdLineArgs::setCwd(QDir::currentPath().toUtf8());
    KCmdLineArgs* args = KCmdLineArgs::parsedArgs();

    // is your app session restored? restore session...
    // this mechanism also falls back to load usual plain rekonq
    // if something goes wrong...
    if (isSessionRestored() && sessionManager()->restoreSession())
    {
        kDebug() << "session restored";
        return 1;
    }

// --------------------------------------------------------------------------

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
        loadUrl(args->arg(0), Rekonq::NewWindow);
        for (int i = 1; i < args->count(); ++i)
            loadUrl(args->arg(i), Rekonq::SettingOpenTab);

        return 3;
    }
    
    // creating new window
    MainWindow *w = newMainWindow();
    w->slotHome();
        
    return 0;
}


Application *Application::instance()
{
    return (static_cast<Application *>(QCoreApplication::instance()));
}


void Application::postLaunch()
{
    setWindowIcon(KIcon("rekonq"));
    
    // set Icon Database Path to store "favicons" associated with web sites
    QString directory = KStandardDirs::locateLocal("cache" , "" , true);
    QWebSettings::setIconDatabasePath(directory);

    Application::historyManager();
    Application::sessionManager();
}


void Application::slotSaveConfiguration() const
{
    ReKonfig::self()->writeConfig();
}


MainWindow *Application::mainWindow()
{
    if(m_mainWindows.isEmpty())
    {
        kDebug() << "No extant windows: creating one new...";
        MainWindow *w = newMainWindow();
        return w;
    }
    
    MainWindow *active = qobject_cast<MainWindow*>(QApplication::activeWindow());
    
    if(!active)
    {
        return m_mainWindows.at(0);
    }
    return active;
}


CookieJar *Application::cookieJar()
{
    return (CookieJar *)networkAccessManager()->cookieJar();
}


NetworkAccessManager *Application::networkAccessManager()
{
    if (!s_networkAccessManager)
    {
        s_networkAccessManager = new NetworkAccessManager();
        s_networkAccessManager->setCookieJar(new CookieJar);
    }
    return s_networkAccessManager;
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
    if(url.isEmpty())
        return KIcon("text-html");

    KIcon icon = KIcon(QWebSettings::iconForUrl(url));
    if (icon.isNull())
    {
        icon = KIcon("text-html");
    }
    return icon;
}


KUrl Application::guessUrlFromString(const QString &string)
{
    QString urlStr = string.trimmed();
    QRegExp test(QLatin1String("^[a-zA-Z]+\\:.*"));

    // Might be a file.
    if (QFile::exists(urlStr))
    {
        QFileInfo info(urlStr);
        return KUrl::fromPath(info.absoluteFilePath());
    }
    
    // Check if it looks like a qualified URL. Try parsing it and see.
    if (test.exactMatch(urlStr))
    {
        KUrl url(urlStr);
        
        if (url.isValid())
        {
            return url;
        }
    }
    else    // Might be a shorturl - try to detect the schema.
    {
        int dotIndex = urlStr.indexOf(QLatin1Char(':'));

        if (dotIndex != -1)
        {
            QString prefix = urlStr.left(dotIndex).toLower();
            QString schema = (prefix == QLatin1String("ftp")) ? prefix : QLatin1String("http");
            QUrl qurl(schema + QLatin1String("://") + urlStr, QUrl::TolerantMode);
            KUrl url(qurl);

            if (url.isValid())
            {
                return url;
            }
        }
    }
    
    // Fall back to QUrl's own tolerant parser.
    KUrl url = KUrl(urlStr);

    return url;
}


void Application::loadUrl(const KUrl& url, const Rekonq::OpenType& type)
{
    if (url.isEmpty())
        return;

    if ( !url.isValid() )
    {
        KMessageBox::error(0, i18n("Malformed URL:\n%1", url.url()));
        return;
    }

    // loading home pages
    if (homePage(url))
        return;
    
    if (url.scheme() == QLatin1String("mailto"))
    {
        KToolInvocation::invokeMailer(url);
        return;
    }

    KUrl loadingUrl(url);

    if (loadingUrl.isRelative())
    {
        QString fn = loadingUrl.url(KUrl::RemoveTrailingSlash);
        if(loadingUrl.path().contains('.'))
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

    if ( !KProtocolInfo::isKnownProtocol( loadingUrl ) )
    {
        KMessageBox::error(0, i18n("Protocol not supported:\n%1", url.protocol()));
        return;
    }

    WebView *webView = 0;
    MainWindow *w = 0;
    
    if(type == Rekonq::NewWindow)
    {
        w = newMainWindow();
    }
    else
    {
        w = mainWindow();
    }
    
    switch(type)
    {
    case Rekonq::SettingOpenTab:
        webView = w->mainView()->newWebView(!ReKonfig::openTabsBack(),
                                            ReKonfig::openTabsNearCurrent());
        if (!ReKonfig::openTabsBack())
        {
            w->mainView()->urlBar()->setUrl(loadingUrl.prettyUrl());
        }
        break;
    case Rekonq::NewCurrentTab:
        webView = w->mainView()->newWebView(true);
        w->mainView()->urlBar()->setUrl(loadingUrl.prettyUrl());
        break;
    case Rekonq::NewBackTab:
        webView = w->mainView()->newWebView(false, ReKonfig::openTabsNearCurrent());
        break;
    case Rekonq::NewWindow:
    case Rekonq::CurrentTab:
        webView = w->mainView()->currentWebView();
        w->mainView()->urlBar()->setUrl(loadingUrl.prettyUrl());
        break;
    };

    if (webView)
    {
        webView->setFocus();
        webView->load(loadingUrl);
    }
}


void Application::loadUrl(const QString& urlString,  const Rekonq::OpenType& type)
{    
    return loadUrl( guessUrlFromString(urlString), type );
}


MainWindow *Application::newMainWindow()
{
    MainWindow *w = new MainWindow();
    w->mainView()->newWebView();    // remember using newWebView and NOT newTab here!!
    
    m_mainWindows.prepend(w);
    w->show();
    QTimer::singleShot(0, this, SLOT(postLaunch()));
    
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


bool Application::homePage(const KUrl &url)
{
    if (    url == KUrl("rekonq:lastSites") 
         || url == KUrl("rekonq:history") 
         || url == KUrl("rekonq:bookmarks")
         || url == KUrl("rekonq:favorites")
         || url == KUrl("rekonq:home")
    )
    {
        kDebug() << "loading home: " << url;
        MainView *view = mainWindow()->mainView(); 
        WebView *w = view->currentWebView();
        HomePage p(w);
        w->setHtml( p.rekonqHomePage(url), url);
        return true;
    }
    return false;
}
