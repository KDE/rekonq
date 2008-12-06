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
#include "browserapplication.h"

#include "browsermainwindow.h"
#include "cookiejar.h"
#include "downloadmanager.h"
#include "history.h"
#include "networkaccessmanager.h"
#include "tabwidget.h"
#include "webview.h"

// KDE Includes
#include <KCmdLineArgs>
#include <KAboutData>
#include <KConfig>

// Qt Includes
#include <QBuffer>
#include <QDir>
#include <QTextStream>
#include <QDesktopServices>
#include <QFileOpenEvent>
#include <QLocalServer>
#include <QLocalSocket>
#include <QNetworkProxy>
#include <QWebSettings>
#include <QDebug>



DownloadManager *BrowserApplication::s_downloadManager = 0;
HistoryManager *BrowserApplication::s_historyManager = 0;
NetworkAccessManager *BrowserApplication::s_networkAccessManager = 0;



BrowserApplication::BrowserApplication(KCmdLineArgs *args, const QString &serverName)
    : KApplication()
    , m_localServer(0)
{
    QLocalSocket socket;
    socket.connectToServer(serverName);
    if (socket.waitForConnected(500)) 
    {
        QTextStream stream(&socket);
        int n = args->count();
        if (n > 1)
            stream << args->arg(n-1);
        else
            stream << QString();
        stream.flush();
        socket.waitForBytesWritten();
        return;
    }

    KApplication::setQuitOnLastWindowClosed(true);

    m_localServer = new QLocalServer(this);
    connect(m_localServer, SIGNAL(newConnection()), this, SLOT(newLocalSocketConnection()));
    if (!m_localServer->listen(serverName)) 
    {
        if (m_localServer->serverError() == QAbstractSocket::AddressInUseError
            && QFile::exists(m_localServer->serverName()))
        {
            QFile::remove(m_localServer->serverName());
            m_localServer->listen(serverName);
        }
    }

    QDesktopServices::setUrlHandler(QLatin1String("http"), this, "openUrl");
    QString localSysName = QLocale::system().name();

    KConfig config("rekonqrc");
    KConfigGroup group = config.group("sessions");
    m_lastSession = group.readEntry( QString("lastSession"), QByteArray() );

    setWindowIcon( KIcon("rekonq") );

    QTimer::singleShot(0, this, SLOT( postLaunch() ) );
}


BrowserApplication::~BrowserApplication()
{
    delete s_downloadManager;
    qDeleteAll(m_mainWindows);
    delete s_networkAccessManager;
}


BrowserApplication *BrowserApplication::instance()
{
    return (static_cast<BrowserApplication *>(QCoreApplication::instance()));
}


/*!
    Any actions that can be delayed until the window is visible
 */
void BrowserApplication::postLaunch()
{
    QString directory = QDesktopServices::storageLocation(QDesktopServices::DataLocation);
    if ( directory.isEmpty() )
    {
        directory = QDir::homePath() + QLatin1String("/.") + QCoreApplication::applicationName();
    }
    QWebSettings::setIconDatabasePath(directory);

    loadSettings();

    // newMainWindow() needs to be called in main() for this to happen
    if (m_mainWindows.count() > 0) 
    {
        KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
        int n = args->count();
        if (n > 1)
            mainWindow()->loadPage(args->arg(n-1));
        else
            mainWindow()->slotHome();
    }
    BrowserApplication::historyManager();
}



void BrowserApplication::loadSettings()
{
    KConfig config("rekonqrc");
    KConfigGroup group = config.group("Appearance Settings");
    
    QWebSettings *defaultSettings = QWebSettings::globalSettings();
    QString standardFontFamily = defaultSettings->fontFamily(QWebSettings::StandardFont);
    int standardFontSize = defaultSettings->fontSize(QWebSettings::DefaultFontSize);
    QFont standardFont = QFont(standardFontFamily, standardFontSize);
    standardFont = qVariantValue<QFont>( group.readEntry( QString("standardFont"), standardFont ) );

    defaultSettings->setFontFamily(QWebSettings::StandardFont, standardFont.family());
    defaultSettings->setFontSize(QWebSettings::DefaultFontSize, standardFont.pointSize());

    QString fixedFontFamily = defaultSettings->fontFamily(QWebSettings::FixedFont);
    int fixedFontSize = defaultSettings->fontSize(QWebSettings::DefaultFixedFontSize);
    QFont fixedFont = QFont(fixedFontFamily, fixedFontSize);
    fixedFont = qVariantValue<QFont>(group.readEntry( QString("fixedFont"), fixedFont ) );

    defaultSettings->setFontFamily(QWebSettings::FixedFont, fixedFont.family());
    defaultSettings->setFontSize(QWebSettings::DefaultFixedFontSize, fixedFont.pointSize());

    defaultSettings->setAttribute(QWebSettings::JavascriptEnabled, group.readEntry( QString("enableJavascript"), true ) );
    defaultSettings->setAttribute(QWebSettings::PluginsEnabled, group.readEntry( QString("enablePlugins"), true ) );
}




QList<BrowserMainWindow*> BrowserApplication::mainWindows()
{
    clean();
    QList<BrowserMainWindow*> list;
    for (int i = 0; i < m_mainWindows.count(); ++i)
    {
        list.append(m_mainWindows.at(i));
    }
    return list;
}




void BrowserApplication::clean()
{
    // cleanup any deleted main windows first
    for (int i = m_mainWindows.count() - 1; i >= 0; --i)
    {
        if (m_mainWindows.at(i).isNull())
        {
            m_mainWindows.removeAt(i);
        }
    }
}


void BrowserApplication::saveSession()
{
    QWebSettings *globalSettings = QWebSettings::globalSettings();
    if (globalSettings->testAttribute(QWebSettings::PrivateBrowsingEnabled))
        return;

    clean();

    KConfig config("rekonqrc");
    KConfigGroup group = config.group("sessions");
    QByteArray data;
    QBuffer buffer(&data);
    QDataStream stream(&buffer);
    buffer.open(QIODevice::ReadWrite);

    stream << m_mainWindows.count();
    for (int i = 0; i < m_mainWindows.count(); ++i)
    {
        stream << m_mainWindows.at(i)->saveState();
    }
    
    group.writeEntry( QString("lastSession"), data );
}


bool BrowserApplication::canRestoreSession() const
{
    return !m_lastSession.isEmpty();
}


void BrowserApplication::restoreLastSession()
{
    QList<QByteArray> windows;
    QBuffer buffer(&m_lastSession);
    QDataStream stream(&buffer);
    buffer.open(QIODevice::ReadOnly);
    int windowCount;
    stream >> windowCount;
    for (int i = 0; i < windowCount; ++i) 
    {
        QByteArray windowState;
        stream >> windowState;
        windows.append(windowState);
    }
    for (int i = 0; i < windows.count(); ++i) 
    {
        BrowserMainWindow *newWindow = 0;
        if (m_mainWindows.count() == 1
            && mainWindow()->tabWidget()->count() == 1
            && mainWindow()->currentTab()->url() == QUrl()) 
        {
            newWindow = mainWindow();
        } 
        else 
        {
            newWindow = newMainWindow();
        }
        newWindow->restoreState(windows.at(i));
    }
}



bool BrowserApplication::isTheOnlyBrowser() const
{
    return (m_localServer != 0);
}


void BrowserApplication::openUrl(const KUrl &url)
{
    mainWindow()->loadPage( url.url() );
}



BrowserMainWindow *BrowserApplication::newMainWindow()
{
    BrowserMainWindow *browser = new BrowserMainWindow();
    m_mainWindows.prepend(browser);
    browser->show();
    return browser;
}


BrowserMainWindow *BrowserApplication::mainWindow()
{
    clean();
    if (m_mainWindows.isEmpty())
        newMainWindow();
    return m_mainWindows[0];
}


void BrowserApplication::newLocalSocketConnection()
{
    QLocalSocket *socket = m_localServer->nextPendingConnection();
    if (!socket)
        return;
    socket->waitForReadyRead(1000);
    QTextStream stream(socket);
    QString url;
    stream >> url;
    if (!url.isEmpty()) 
    {
        KConfig config("rekonqrc");
        KConfigGroup group = config.group("Global Settings");
        int openLinksIn = group.readEntry( QString("openLinksIn"), QString().toInt() );
        if (openLinksIn == 1)
        {
            newMainWindow();
        }
        else
        {
            mainWindow()->tabWidget()->newTab();
        }
        openUrl(url);
    }
    delete socket;
    mainWindow()->raise();
    mainWindow()->activateWindow();
}



CookieJar *BrowserApplication::cookieJar()
{
    return (CookieJar*)networkAccessManager()->cookieJar();
}


DownloadManager *BrowserApplication::downloadManager()
{
    if (!s_downloadManager) 
    {
        s_downloadManager = new DownloadManager();
    }
    return s_downloadManager;
}



NetworkAccessManager *BrowserApplication::networkAccessManager()
{
    if (!s_networkAccessManager) 
    {
        s_networkAccessManager = new NetworkAccessManager();
        s_networkAccessManager->setCookieJar(new CookieJar);
    }
    return s_networkAccessManager;
}



HistoryManager *BrowserApplication::historyManager()
{
    if (!s_historyManager) 
    {
        s_historyManager = new HistoryManager();
        QWebHistoryInterface::setDefaultInterface(s_historyManager);
    }
    return s_historyManager;
}




KIcon BrowserApplication::icon(const KUrl &url) const
{
    KIcon icon = KIcon( QWebSettings::iconForUrl(url) );
    if (!icon.isNull())
        return icon;
    if (m_defaultIcon.isNull())
        m_defaultIcon = KIcon("kde");
    return m_defaultIcon;
}

