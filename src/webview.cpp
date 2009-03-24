/* ============================================================
*
* This file is a part of the rekonq project
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
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* ============================================================ */


// Self Includes
#include "webview.h"
#include "webview.moc"

// Auto Includes
#include "rekonq.h"

// Local Includes
#include "application.h"
#include "mainwindow.h"
#include "mainview.h"
#include "cookiejar.h"
#include "networkaccessmanager.h"

// KDE Includes
#include <KStandardDirs>
#include <KUrl>
#include <KDebug>

// Qt Includes
#include <QtCore>
#include <QtGui>
#include <QtWebKit>
#include <QUiLoader>


WebPage::WebPage(QObject *parent)
    : QWebPage(parent)
    , m_keyboardModifiers(Qt::NoModifier)
    , m_pressedButtons(Qt::NoButton)
    , m_openInNewTab(false)
{
    setNetworkAccessManager( Application::networkAccessManager() );
    connect(this, SIGNAL(unsupportedContent(QNetworkReply *)), this, SLOT(handleUnsupportedContent(QNetworkReply *)));
}


WebPage::~WebPage()
{

}


MainWindow *WebPage::mainWindow()
{
    QObject *w = this->parent();
    while (w) 
    {
        if (MainWindow *mw = qobject_cast<MainWindow*>(w))
            return mw;
        w = w->parent();
    }
    return Application::instance()->mainWindow();
}


bool WebPage::acceptNavigationRequest(QWebFrame *frame, const QNetworkRequest &request, NavigationType type)
{
    kWarning() << "Accepting Navigation Request..";

    // ctrl open in new tab and select
    // ctrl-alt open in new window
    if ( type == QWebPage::NavigationTypeLinkClicked && (m_keyboardModifiers & Qt::ControlModifier
            || m_pressedButtons == Qt::MidButton) ) 
    {
        WebView *webView = Application::instance()->newTab();
        webView->setFocus();
        webView->load(request);
        m_keyboardModifiers = Qt::NoModifier;
        m_pressedButtons = Qt::NoButton;
        return false;
    }
    if ( frame == mainFrame() ) 
    {
        m_loadingUrl = request.url();
        emit loadingUrl(m_loadingUrl);
    }
    else
    {
        kWarning() << "NO Main Frame..";
        kWarning() << "Frame : " << frame->frameName();
    }
    return QWebPage::acceptNavigationRequest(frame, request, type);
}


QWebPage *WebPage::createWindow(QWebPage::WebWindowType type)
{
    // added to manage web modal dialogs
    if(type == QWebPage::WebModalDialog)
    {
        WebView *w = new WebView;
        return w->page();
    }

    if (m_keyboardModifiers & Qt::ControlModifier || m_pressedButtons == Qt::MidButton)
    {
        m_openInNewTab = true;
    }
    
    if (m_openInNewTab) 
    {
        m_openInNewTab = false;
        return mainWindow()->mainView()->newTab()->page();
    }

    MainWindow *mainWindow = Application::instance()->mainWindow();
    return mainWindow->currentTab()->page();
}


QObject *WebPage::createPlugin(const QString &classId, const QUrl &url, const QStringList &paramNames, const QStringList &paramValues)
{
    kWarning() << "creating PLUGIN for rekonq ";
    kWarning() << "classId = " << classId;
    kWarning() << "url = " << url;
    kWarning() << "Param Names = " << paramNames;
    kWarning() << "Param Values = " << paramValues;

    QUiLoader loader;
    return loader.createWidget(classId, view());
}


void WebPage::handleUnsupportedContent(QNetworkReply *reply)
{
    if (reply->error() == QNetworkReply::NoError) 
    {
        KUrl srcUrl = reply->url();
        QString path = ReKonfig::downloadDir() + QString("/") + srcUrl.fileName();
        KUrl destUrl = KUrl(path);
        Application::instance()->downloadUrl( srcUrl, destUrl );
        return;
    }

    QString myfilestr =  KStandardDirs::locate("data", "rekonq/htmls/notfound.html");
    QFile file( myfilestr );
    bool isOpened = file.open(QIODevice::ReadOnly);
    Q_ASSERT(isOpened);

    QString title = i18n("Error loading page: ") + reply->url().toString();

    QString imagePath = KIconLoader::global()->iconPath( "rekonq", KIconLoader::NoGroup, false);

    QString html = QString(QLatin1String(file.readAll()))
                        .arg(title)
                        .arg("file://" + imagePath)
                        .arg(reply->errorString())
                        .arg(reply->url().toString());

    QList<QWebFrame*> frames;
    frames.append(mainFrame());
    while (!frames.isEmpty()) 
    {
        QWebFrame *frame = frames.takeFirst();
        if (frame->url() == reply->url()) 
        {
            frame->setHtml(html, reply->url());
            return;
        }
        QList<QWebFrame *> children = frame->childFrames();
        foreach(QWebFrame *frame, children)
            frames.append(frame);
    }
    if (m_loadingUrl == reply->url()) 
    {
        mainFrame()->setHtml(html, reply->url());
    }
}


// -----------------------------------------------------------------------------------------------------------------


WebView::WebView(QWidget* parent)
    : QWebView(parent)
    , m_progress(0)
    , m_page(new WebPage(this))
{
    setPage(m_page);
    connect(page(), SIGNAL(statusBarMessage(const QString&)), this, SLOT(setStatusBarText(const QString&)));
    connect(this, SIGNAL(loadProgress(int)), this, SLOT(setProgress(int)));
    connect(this, SIGNAL(loadFinished(bool)), this, SLOT(loadFinished()));
    connect(page(), SIGNAL(loadingUrl(const QUrl&)),  this, SIGNAL(urlChanged(const QUrl &)));
    connect(page(), SIGNAL(downloadRequested(const QNetworkRequest &)), this, SLOT(downloadRequested(const QNetworkRequest &)));
    page()->setForwardUnsupportedContent(true);
}


// TODO : improve and KDE-ize this menu
// 1. Add link to bookmarks
// 2. Add "save link as" action
void WebView::contextMenuEvent(QContextMenuEvent *event)
{
    QWebHitTestResult r = page()->mainFrame()->hitTestContent(event->pos());
    if (!r.linkUrl().isEmpty()) 
    {
        KMenu menu(this);
        KAction *a = new KAction( KIcon("tab-new"), i18n("Open in New Tab"), this);
        connect( a, SIGNAL( triggered() ), this , SLOT( openLinkInNewTab() ) );
        menu.addAction(a);
        menu.addSeparator();
        menu.addAction( pageAction(QWebPage::DownloadLinkToDisk) );
        // Add link to bookmarks...
        menu.addSeparator();
        menu.addAction( pageAction(QWebPage::CopyLinkToClipboard) );
        if ( page()->settings()->testAttribute(QWebSettings::DeveloperExtrasEnabled) )
        {
            menu.addAction(pageAction(QWebPage::InspectElement));
        }
        menu.exec(mapToGlobal(event->pos()));
        return;
    }
    QWebView::contextMenuEvent(event);
}


void WebView::wheelEvent(QWheelEvent *event)
{
    if (QApplication::keyboardModifiers() & Qt::ControlModifier) 
    {
        int numDegrees = event->delta() / 8;
        int numSteps = numDegrees / 15;
        setTextSizeMultiplier(textSizeMultiplier() + numSteps * 0.1);
        event->accept();
        return;
    }
    QWebView::wheelEvent(event);
}


void WebView::openLinkInNewTab()
{
    m_page->m_openInNewTab = true;
    pageAction(QWebPage::OpenLinkInNewWindow)->trigger();
}


void WebView::setProgress(int progress)
{
    m_progress = progress;
}


void WebView::loadFinished()
{
    if (m_progress != 100) 
    {
        kWarning() << "Recieved finished signal while progress is still:" << progress()
                   << "Url:" << url();
    }
    m_progress = 0;
}


// FIXME: load http by default!!
void WebView::loadUrl(const KUrl &url)
{
    m_initialUrl = url;

    if( m_initialUrl.isRelative() )
    {
        kWarning() << "1: " << m_initialUrl.url();
        QString fn = m_initialUrl.url( KUrl::RemoveTrailingSlash );
        kWarning() << "2: " << fn;
        m_initialUrl.setUrl( "//" + fn );
        m_initialUrl.setScheme("http");
        kWarning() << "3: " << m_initialUrl.url();
    }

    load(m_initialUrl);
}


QString WebView::lastStatusBarText() const
{
    return m_statusBarText;
}


KUrl WebView::url() const
{
    KUrl url = QWebView::url();
    if ( !url.isEmpty() )
    {
        return url;
    }
    return m_initialUrl;
}


void WebView::mousePressEvent(QMouseEvent *event)
{
    m_page->m_pressedButtons = event->buttons();
    m_page->m_keyboardModifiers = event->modifiers();
    QWebView::mousePressEvent(event);
}


void WebView::mouseReleaseEvent(QMouseEvent *event)
{
    QWebView::mouseReleaseEvent(event);
    if (!event->isAccepted() && (m_page->m_pressedButtons & Qt::MidButton)) 
    {
        KUrl url( QApplication::clipboard()->text(QClipboard::Selection) );
        if (!url.isEmpty() && url.isValid() && !url.scheme().isEmpty()) 
        {
            setUrl(url);
        }
    }
}


void WebView::setStatusBarText(const QString &string)
{
    m_statusBarText = string;
}


void WebView::downloadRequested(const QNetworkRequest &request)
{
    KUrl srcUrl = request.url();
    QString path = ReKonfig::downloadDir() + QString("/") + srcUrl.fileName();
    KUrl destUrl = KUrl(path);
    Application::instance()->downloadUrl( srcUrl, destUrl );
}


void WebView::keyPressEvent(QKeyEvent *event)
{
    if ( (event->modifiers() == Qt::ControlModifier) && (event->key() == Qt::Key_Tab) )
    {
        emit ctrlTabPressed();
        return;
    }

    if( (event->modifiers() == Qt::ControlModifier + Qt::ShiftModifier) && (event->key() == Qt::Key_Backtab) )
    {
        emit shiftCtrlTabPressed();
        return;
    }

    QWebView::keyPressEvent( event );
}

