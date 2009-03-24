/* ============================================================
*
* This file is a part of the rekonq project
*
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
#include "download.h"

// KDE Includes
#include <KCmdLineArgs>
#include <KAboutData>
#include <KStandardDirs>
#include <KConfig>
#include <kio/job.h>
#include <kio/jobclasses.h>

// Qt Includes
#include <QBuffer>
#include <QDir>
#include <QTextStream>
#include <QFileOpenEvent>
#include <QNetworkProxy>
#include <QWebSettings>
#include <QDebug>



HistoryManager *Application::s_historyManager = 0;
NetworkAccessManager *Application::s_networkAccessManager = 0;


Application::Application()
    : KUniqueApplication()
{
    m_mainWindow = new MainWindow();
    m_mainWindow->setObjectName("MainWindow");
    setWindowIcon( KIcon("rekonq") );
    newTab();
    mainWindow()->slotHome();

    m_mainWindow->show();

    QTimer::singleShot(0, this, SLOT( postLaunch() ) );
}


Application::~Application()
{
    delete s_networkAccessManager;
    delete s_historyManager;
}


int Application::newInstance()
{
    KCmdLineArgs::setCwd(QDir::currentPath().toUtf8());
    KCmdLineArgs* args = KCmdLineArgs::parsedArgs();

    if (args->count() > 0) 
    {
        for (int i = 0; i < args->count(); ++i) 
        {
            KUrl url = MainWindow::guessUrlFromString( args->arg(i) );
            newTab();
            mainWindow()->loadUrl( url );
        }
        args->clear();
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
    QString directory = KStandardDirs::locateLocal( "cache" , "" , true );
    if ( directory.isEmpty() )
    {
        directory = QDir::homePath() + QLatin1String("/.") + QCoreApplication::applicationName();
    }
    QWebSettings::setIconDatabasePath(directory);

    Application::historyManager();
}


void Application::downloadUrl(const KUrl &srcUrl, const KUrl &destUrl)
{
    new Download( srcUrl, destUrl );
}


void Application::openUrl(const KUrl &url)
{
    mainWindow()->loadUrl(url);
}


MainWindow *Application::mainWindow()
{
    return m_mainWindow;
}


WebView *Application::newTab()
{
    return m_mainWindow->mainView()->newTab();
}


CookieJar *Application::cookieJar()
{
    return (CookieJar*)networkAccessManager()->cookieJar();
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


KIcon Application::icon(const KUrl &url) const
{
    KIcon icon = KIcon( QWebSettings::iconForUrl(url) );
    if (icon.isNull())
    {
        icon = KIcon("kde");
    }
    return icon;
}

