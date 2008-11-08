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


#include "browserapplication.h"

#include "bookmarks.h"
#include "browsermainwindow.h"
#include "cookiejar.h"
#include "downloadmanager.h"
#include "history.h"
#include "networkaccessmanager.h"
#include "tabwidget.h"
#include "webview.h"

#include <KCmdLineArgs>
#include <KAboutData>

#include <QBuffer>
#include <QDir>
#include <QLibraryInfo>
#include <QSettings>
#include <QTextStream>
#include <QTranslator>
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
BookmarksManager *BrowserApplication::s_bookmarksManager = 0;



BrowserApplication::BrowserApplication(KCmdLineArgs *args, const QString &serverName)
    : KApplication()
    , m_localServer(0)
{
    QLocalSocket socket;
    socket.connectToServer(serverName);
    if (socket.waitForConnected(500)) {
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

    installTranslator(QLatin1String("qt_") + localSysName);

    QSettings settings;
    settings.beginGroup(QLatin1String("sessions"));
    m_lastSession = settings.value(QLatin1String("lastSession")).toByteArray();
    settings.endGroup();

    QTimer::singleShot(0, this, SLOT(postLaunch()));
}




BrowserApplication::~BrowserApplication()
{
    delete s_downloadManager;
    qDeleteAll(m_mainWindows);
    delete s_networkAccessManager;
    delete s_bookmarksManager;
}



#if defined(Q_WS_MAC)
void BrowserApplication::lastWindowClosed()
{
    clean();
    BrowserMainWindow *mw = new BrowserMainWindow;
    mw->slotHome();
    m_mainWindows.prepend(mw);
}
#endif



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
    if (directory.isEmpty())
        directory = QDir::homePath() + QLatin1String("/.") + QCoreApplication::applicationName();
    QWebSettings::setIconDatabasePath(directory);

//     setWindowIcon(QIcon(QLatin1String(":browser.svg")));  // FIXME setICON

    loadSettings();

    // newMainWindow() needs to be called in main() for this to happen
    if (m_mainWindows.count() > 0) {
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
    QSettings settings;
    settings.beginGroup(QLatin1String("websettings"));

    QWebSettings *defaultSettings = QWebSettings::globalSettings();
    QString standardFontFamily = defaultSettings->fontFamily(QWebSettings::StandardFont);
    int standardFontSize = defaultSettings->fontSize(QWebSettings::DefaultFontSize);
    QFont standardFont = QFont(standardFontFamily, standardFontSize);
    standardFont = qVariantValue<QFont>(settings.value(QLatin1String("standardFont"), standardFont));
    defaultSettings->setFontFamily(QWebSettings::StandardFont, standardFont.family());
    defaultSettings->setFontSize(QWebSettings::DefaultFontSize, standardFont.pointSize());

    QString fixedFontFamily = defaultSettings->fontFamily(QWebSettings::FixedFont);
    int fixedFontSize = defaultSettings->fontSize(QWebSettings::DefaultFixedFontSize);
    QFont fixedFont = QFont(fixedFontFamily, fixedFontSize);
    fixedFont = qVariantValue<QFont>(settings.value(QLatin1String("fixedFont"), fixedFont));
    defaultSettings->setFontFamily(QWebSettings::FixedFont, fixedFont.family());
    defaultSettings->setFontSize(QWebSettings::DefaultFixedFontSize, fixedFont.pointSize());

    defaultSettings->setAttribute(QWebSettings::JavascriptEnabled, settings.value(QLatin1String("enableJavascript"), true).toBool());
    defaultSettings->setAttribute(QWebSettings::PluginsEnabled, settings.value(QLatin1String("enablePlugins"), true).toBool());

    QUrl url = settings.value(QLatin1String("userStyleSheet")).toUrl();
    defaultSettings->setUserStyleSheetUrl(url);

    settings.endGroup();
}




QList<BrowserMainWindow*> BrowserApplication::mainWindows()
{
    clean();
    QList<BrowserMainWindow*> list;
    for (int i = 0; i < m_mainWindows.count(); ++i)
        list.append(m_mainWindows.at(i));
    return list;
}




void BrowserApplication::clean()
{
    // cleanup any deleted main windows first
    for (int i = m_mainWindows.count() - 1; i >= 0; --i)
        if (m_mainWindows.at(i).isNull())
            m_mainWindows.removeAt(i);
}




void BrowserApplication::saveSession()
{
    QWebSettings *globalSettings = QWebSettings::globalSettings();
    if (globalSettings->testAttribute(QWebSettings::PrivateBrowsingEnabled))
        return;

    clean();

    QSettings settings;
    settings.beginGroup(QLatin1String("sessions"));

    QByteArray data;
    QBuffer buffer(&data);
    QDataStream stream(&buffer);
    buffer.open(QIODevice::ReadWrite);

    stream << m_mainWindows.count();
    for (int i = 0; i < m_mainWindows.count(); ++i)
        stream << m_mainWindows.at(i)->saveState();
    settings.setValue(QLatin1String("lastSession"), data);
    settings.endGroup();
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
    for (int i = 0; i < windowCount; ++i) {
        QByteArray windowState;
        stream >> windowState;
        windows.append(windowState);
    }
    for (int i = 0; i < windows.count(); ++i) {
        BrowserMainWindow *newWindow = 0;
        if (m_mainWindows.count() == 1
            && mainWindow()->tabWidget()->count() == 1
            && mainWindow()->currentTab()->url() == QUrl()) {
            newWindow = mainWindow();
        } else {
            newWindow = newMainWindow();
        }
        newWindow->restoreState(windows.at(i));
    }
}



bool BrowserApplication::isTheOnlyBrowser() const
{
    return (m_localServer != 0);
}



void BrowserApplication::installTranslator(const QString &name)
{
    QTranslator *translator = new QTranslator(this);
    translator->load(name, QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    KApplication::installTranslator(translator);
}



void BrowserApplication::openUrl(const QUrl &url)
{
    mainWindow()->loadPage(url.toString());
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
    if (!url.isEmpty()) {
        QSettings settings;
        settings.beginGroup(QLatin1String("general"));
        int openLinksIn = settings.value(QLatin1String("openLinksIn"), 0).toInt();
        settings.endGroup();
        if (openLinksIn == 1)
            newMainWindow();
        else
            mainWindow()->tabWidget()->newTab();
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
    if (!s_downloadManager) {
        s_downloadManager = new DownloadManager();
    }
    return s_downloadManager;
}



NetworkAccessManager *BrowserApplication::networkAccessManager()
{
    if (!s_networkAccessManager) {
        s_networkAccessManager = new NetworkAccessManager();
        s_networkAccessManager->setCookieJar(new CookieJar);
    }
    return s_networkAccessManager;
}



HistoryManager *BrowserApplication::historyManager()
{
    if (!s_historyManager) {
        s_historyManager = new HistoryManager();
        QWebHistoryInterface::setDefaultInterface(s_historyManager);
    }
    return s_historyManager;
}




BookmarksManager *BrowserApplication::bookmarksManager()
{
    if (!s_bookmarksManager) {
        s_bookmarksManager = new BookmarksManager;
    }
    return s_bookmarksManager;
}



KIcon BrowserApplication::icon(const QUrl &url) const
{
    KIcon icon = KIcon( QWebSettings::iconForUrl(url) );
    if (!icon.isNull())
        return icon;
    if (m_defaultIcon.isNull())
        m_defaultIcon = KIcon("rekonq");
    return m_defaultIcon;
}

