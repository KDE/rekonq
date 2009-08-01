/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2008 by Andrea Diamantini <adjam7 at gmail dot com>
* Copyright (C) 2009 by Paweł Prażak <pawelprazak at gmail dot com>
*
*
* This program is free software; you can redistribute it
* and/or modify it under the terms of the GNU General
* Public License as published by the Free Software Foundation;
* either version 3, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
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

// KDE Includes
#include <KCmdLineArgs>
#include <KAboutData>
#include <KStandardDirs>
#include <KConfig>
#include <kio/job.h>
#include <kio/jobclasses.h>
#include <KPassivePopup>
#include <KToolInvocation>
#include <KUriFilter>

// Qt Includes
#include <QRegExp>
#include <QFile>
#include <QFileInfo>
#include <QtCore/QTimer>
#include <QtWebKit/QWebSettings>
#include <QtWebKit/QWebHistoryInterface>


QPointer<HistoryManager> Application::s_historyManager;
QPointer<NetworkAccessManager> Application::s_networkAccessManager;
QPointer<BookmarkProvider> Application::s_bookmarkProvider;



Application::Application()
        : KUniqueApplication()
{
}


Application::~Application()
{
    delete m_mainWindow;
    delete s_bookmarkProvider;
    delete s_networkAccessManager;
    delete s_historyManager;
}


int Application::newInstance()
{
    KCmdLineArgs::setCwd(QDir::currentPath().toUtf8());
    KCmdLineArgs* args = KCmdLineArgs::parsedArgs();

    if (!m_mainWindow)
    {
        m_mainWindow = new MainWindow();

        m_mainWindow->setObjectName("MainWindow");
        setWindowIcon(KIcon("rekonq"));

        m_mainWindow->show();

        QTimer::singleShot(0, this, SLOT(postLaunch()));
    }

    if (args->count() > 0)
    {
        for (int i = 0; i < args->count(); ++i)
        {
            loadUrl(args->arg(i), Rekonq::NewCurrentTab); 
        }
        args->clear();
    }
    else
    {
        m_mainWindow->mainView()->newTab();
        m_mainWindow->slotHome();
    }

    return 0;
}


Application *Application::instance()
{
    return (static_cast<Application *>(QCoreApplication::instance()));
}


void Application::postLaunch()
{
    // set Icon Database Path to store "favicons" associated with web sites
    QString directory = KStandardDirs::locateLocal("cache" , "" , true);
    if (directory.isEmpty())
    {
        directory = QDir::homePath() + QLatin1String("/.") + QCoreApplication::applicationName();
    }
    QWebSettings::setIconDatabasePath(directory);

    Application::historyManager();
}


void Application::slotSaveConfiguration() const
{
    ReKonfig::self()->writeConfig();
}


MainWindow *Application::mainWindow()
{
    return m_mainWindow;
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
        s_bookmarkProvider = new BookmarkProvider(instance()->mainWindow());
    }
    return s_bookmarkProvider;
}


KIcon Application::icon(const KUrl &url)
{
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

    // Check if it looks like a qualified URL. Try parsing it and see.
    bool hasSchema = test.exactMatch(urlStr);

    if (hasSchema)
    {
        QUrl qurl(urlStr, QUrl::TolerantMode);
        KUrl url(qurl);

        if (url.isValid())
        {
            return url;
        }
    }

    // Might be a file.
    if (QFile::exists(urlStr))
    {
        QFileInfo info(urlStr);
        return KUrl::fromPath(info.absoluteFilePath());
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

            if (url.isValid())
            {
                return url;
            }
        }
    }

    // Fall back to QUrl's own tolerant parser.
    QUrl qurl = QUrl(string, QUrl::TolerantMode);
    KUrl url(qurl);

    // finally for cases where the user just types in a hostname add http
    if (qurl.scheme().isEmpty())
    {
        qurl = QUrl(QLatin1String("http://") + string, QUrl::TolerantMode);
        url = KUrl(qurl);
    }
    return url;
}


void Application::loadUrl(const KUrl& url, const Rekonq::OpenType& type)
{
    if (url.isEmpty())
        return;

    if (url.scheme() == QLatin1String("mailto"))
    {
        KToolInvocation::invokeMailer(url);
        return;
    }

    KUrl loadingUrl(url);
    if (loadingUrl.isRelative() && loadingUrl.path().contains("."))
    {
        QString fn = loadingUrl.url(KUrl::RemoveTrailingSlash);
        loadingUrl.setUrl("//" + fn);
        loadingUrl.setScheme("http");
    }
    else if(loadingUrl.scheme()!="http")
    {
        // this should let rekonq to support the beautiful KDE web browsing shortcuts
        KUriFilterData data(loadingUrl.pathOrUrl());
        data.setCheckForExecutables (false); //if true, querries like "rekonq" or "dolphin" are considered as executables
        if (KUriFilter::self()->filterUri(data))
        {
            loadingUrl = data.uri().url();
        }
    }

    WebView *webView;
    
    switch(type)
    {
    case Rekonq::SettingOpenTab:
        webView = m_mainWindow->mainView()->newTab(!ReKonfig::openTabsBack());
        break;
    case Rekonq::NewCurrentTab:
        webView = m_mainWindow->mainView()->newTab(true);
        break;
    case Rekonq::NewBackTab:
        webView = m_mainWindow->mainView()->newTab(false);
        break;
    case Rekonq::CurrentTab:
        webView = m_mainWindow->mainView()->currentWebView();
        break;
    };

    m_mainWindow->mainView()->currentUrlBar()->setUrl(loadingUrl.prettyUrl());

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
