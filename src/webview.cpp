/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2007-2008 Trolltech ASA. All rights reserved
* Copyright (C) 2008-2009 by Andrea Diamantini <adjam7 at gmail dot com>
* Copyright (C) 2009 rekonq team. Please, see AUTHORS file for details
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
#include "download.h"
#include "history.h"


// KDE Includes
#include <KStandardDirs>
#include <KUrl>
#include <KActionCollection>
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
    setNetworkAccessManager(Application::networkAccessManager());
    connect(this, SIGNAL(unsupportedContent(QNetworkReply *)), this, SLOT(handleUnsupportedContent(QNetworkReply *)));
}


WebPage::~WebPage()
{
}


bool WebPage::acceptNavigationRequest(QWebFrame *frame, const QNetworkRequest &request, NavigationType type)
{
    QString scheme = request.url().scheme();
    if (scheme == QLatin1String("mailto") )
    {
        QDesktopServices::openUrl(request.url());
        return false;
    }

    WebView *webView;

    switch(type)
    {

    // user clicked on a link or pressed return on a focused link.
    case QWebPage::NavigationTypeLinkClicked:

        if(m_keyboardModifiers & Qt::ControlModifier || m_pressedButtons == Qt::MidButton)
        {
            webView = Application::instance()->newWebView();
            webView->setFocus();
            webView->load(request);
            m_keyboardModifiers = Qt::NoModifier;
            m_pressedButtons = Qt::NoButton;
            return false;
        }

        if (frame == mainFrame())
        {
            m_loadingUrl = request.url();
            emit loadingUrl(m_loadingUrl);
        }
        else
        {
            // if frame doesn't exists (perhaps) we are pointing to a blank target..
            if(!frame)
            {
                webView = Application::instance()->newWebView();
                webView->setFocus();
                webView->load(request);
                return false;
            }
        }
        break;

    // user activated a submit button for an HTML form.
    case QWebPage::NavigationTypeFormSubmitted:
        break;

    // Navigation to a previously shown document in the back or forward history is requested.
    case QWebPage::NavigationTypeBackOrForward:
        break;

    // user activated the reload action.
    case QWebPage::NavigationTypeReload:
        break;

    // An HTML form was submitted a second time.
    case QWebPage::NavigationTypeFormResubmitted:
        break;

    // A navigation to another document using a method not listed above.
    case QWebPage::NavigationTypeOther:
        break;

    // should be nothing..
    default:
        break;
    }

    return QWebPage::acceptNavigationRequest(frame, request, type);
}


QWebPage *WebPage::createWindow(QWebPage::WebWindowType type)
{
    // added to manage web modal dialogs
    if (type == QWebPage::WebModalDialog)
    {
        // FIXME
        kWarning() << "trying QWebView here ---------------------------------------";
        QWebView *w = new QWebView();
        return w->page();
    }

    if (m_keyboardModifiers & Qt::ControlModifier || m_pressedButtons == Qt::MidButton)
    {
        m_openInNewTab = true;
    }

    if (m_openInNewTab)
    {
        m_openInNewTab = false;
        return Application::instance()->mainWindow()->mainView()->newWebView()->page();
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
        Application::downloadManager()->newDownload(srcUrl);
        return;
    }

    // display "not found" page
    QString notfoundFilePath =  KStandardDirs::locate("data", "rekonq/htmls/notfound.html");
    QFile file(notfoundFilePath);
    bool isOpened = file.open(QIODevice::ReadOnly);
    if (!isOpened)
    {
        kWarning() << "Couldn't open the notfound.html file";
        return;
    }
    QString title = i18n("Error loading page: ") + reply->url().toString();

    QString imagePath = KIconLoader::global()->iconPath("rekonq", KIconLoader::NoGroup, false);

    QString html = QString(QLatin1String(file.readAll()))
                   .arg(title)
                   .arg("file://" + imagePath)
                   .arg(reply->errorString())
                   .arg(reply->url().toString());

    QList<QWebFrame*> frames;
    frames.append(mainFrame());
    while (!frames.isEmpty())
    {
        QWebFrame *firstFrame = frames.takeFirst();
        if (firstFrame->url() == reply->url())
        {
            firstFrame->setHtml(html, reply->url());
            return;
        }
        QList<QWebFrame *> children = firstFrame->childFrames();
        foreach(QWebFrame *frame, children)
        frames.append(frame);
    }
    if (m_loadingUrl == reply->url())
    {
        mainFrame()->setHtml(html, reply->url());
        // Don't put error pages to the history.
        Application::historyManager()->removeHistoryEntry(reply->url(), mainFrame()->title());
    }
}


// -----------------------------------------------------------------------------------------------------------------


WebView::WebView(QWidget* parent)
        : QWebView(parent)
        , m_webActionCollection(new KActionCollection(this))
        , m_page(new WebPage(this))
        , m_progress(0)
{
    setPage(m_page);
    connect(page(), SIGNAL(statusBarMessage(const QString&)), this, SLOT(setStatusBarText(const QString&)));
    connect(this, SIGNAL(loadProgress(int)), this, SLOT(setProgress(int)));
    connect(this, SIGNAL(loadFinished(bool)), this, SLOT(loadFinished()));
    connect(page(), SIGNAL(loadingUrl(const QUrl&)),  this, SIGNAL(urlChanged(const QUrl &)));
    connect(page(), SIGNAL(downloadRequested(const QNetworkRequest &)), this, SLOT(downloadRequested(const QNetworkRequest &)));
    page()->setForwardUnsupportedContent(true);

    fillWebActions();
}


void WebView::fillWebActions()
{

    QAction *a;

    a = new KAction(KIcon("tab-new"), i18n("Open Link in New &Tab"), this);
    connect(a, SIGNAL(triggered()), this, SLOT(openLinkInNewTab()) );
    m_webActionCollection->addAction( QLatin1String("open_link_in_new_tab"), a);
    
    a = pageAction(QWebPage::Cut);
    a->setIcon(KIcon("edit-cut"));
    a->setText(i18n("Cu&t"));
    m_webActionCollection->addAction( QLatin1String("edit_cut"), a);
    
    a = pageAction(QWebPage::Copy);
    a->setIcon(KIcon("edit-copy"));
    a->setText(i18n("&Copy"));
    m_webActionCollection->addAction( QLatin1String("edit_copy"), a );
    
    a = pageAction(QWebPage::Paste);
    a->setIcon(KIcon("edit-paste"));
    a->setText(i18n("&Paste"));
    m_webActionCollection->addAction( QLatin1String("edit_paste"), a );
    
    a = pageAction(QWebPage::DownloadImageToDisk);
    a->setIcon(KIcon("folder-image"));
    a->setText(i18n("&Save Image As..."));
    m_webActionCollection->addAction( QLatin1String("save_image_as"), a );
    
    a = pageAction(QWebPage::CopyImageToClipboard);
    a->setIcon(KIcon("insert-image"));
    a->setText(i18n("&Copy This Image"));
    m_webActionCollection->addAction( QLatin1String("copy_this_image"), a);

    a = pageAction(QWebPage::DownloadLinkToDisk);
    a->setIcon(KIcon("folder-downloads"));
    a->setText(i18n("&Save Link As..."));
    m_webActionCollection->addAction( QLatin1String("save_link_as"), a);
    
    a = pageAction(QWebPage::CopyLinkToClipboard);
    a->setIcon(KIcon("insert-link"));
    a->setText(i18n("&Copy Link Location"));
    m_webActionCollection->addAction( QLatin1String("copy_link_location"), a);
}


void WebView::contextMenuEvent(QContextMenuEvent *event)
{
    QWebHitTestResult result = page()->mainFrame()->hitTestContent(event->pos());
    MainWindow *mainwindow = Application::instance()->mainWindow();
    
    QAction *addBookmarkAction = Application::bookmarkProvider()->actionByName("add_bookmark_payload");
    addBookmarkAction->setText(i18n("Bookmark This Page"));
    addBookmarkAction->setData(QVariant());
    KMenu menu(this);
    
    bool linkIsEmpty = result.linkUrl().isEmpty();
    if (!linkIsEmpty)
    {
        menu.addAction( m_webActionCollection->action("open_link_in_new_tab") );
    }
    else
    {
        menu.addAction(mainwindow->actionByName("new_tab"));
    }

    menu.addAction(mainwindow->actionByName("view_redisplay"));
    menu.addSeparator();
    menu.addAction(mainwindow->actionByName("history_back"));
    menu.addAction(mainwindow->actionByName("history_forward"));
    menu.addSeparator();

    if (result.isContentSelected() && result.isContentEditable())
    {
        menu.addAction( m_webActionCollection->action("edit_cut") );
    }

    if (result.isContentSelected())
    {
        menu.addAction( m_webActionCollection->action("edit_copy") );
    }

    if (result.isContentEditable())
    {
        menu.addAction( m_webActionCollection->action("edit_paste") );
    }

    if (!linkIsEmpty)
    {
        menu.addSeparator();
        if (!result.pixmap().isNull())
        {
            // TODO Add "View Image"
            menu.addAction( m_webActionCollection->action("save_image_as") );
            menu.addAction( m_webActionCollection->action("copy_this_image") );
        }
        menu.addAction( m_webActionCollection->action("save_link_as") );
        menu.addAction( m_webActionCollection->action("copy_link_location") );
        addBookmarkAction->setData(result.linkUrl());
        addBookmarkAction->setText(i18n("&Bookmark This Link"));
    }
    menu.addSeparator();
    
    menu.addAction(addBookmarkAction);
    menu.exec(mapToGlobal(event->pos()));
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

    if (m_initialUrl.isRelative())
    {
        kWarning() << "1: " << m_initialUrl.url();
        QString fn = m_initialUrl.url(KUrl::RemoveTrailingSlash);
        kWarning() << "2: " << fn;
        m_initialUrl.setUrl("//" + fn);
        m_initialUrl.setScheme("http");
        kWarning() << "3: " << m_initialUrl.url();
    }

    load(m_initialUrl);
}


KUrl WebView::url() const
{
    KUrl url = QWebView::url();
    if (!url.isEmpty())
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
        KUrl url(QApplication::clipboard()->text(QClipboard::Selection));
        if (!url.isEmpty() && url.isValid() && !url.scheme().isEmpty())
        {
            setUrl(url);
        }
    }
}


void WebView::downloadRequested(const QNetworkRequest &request)
{
    KUrl srcUrl = request.url();
    QString path = ReKonfig::downloadDir() + QString("/") + srcUrl.fileName();
    KUrl destUrl = KUrl(path);
    Application::downloadManager()->newDownload(srcUrl);
}


void WebView::keyPressEvent(QKeyEvent *event)
{
    if ((event->modifiers() == Qt::ControlModifier) && (event->key() == Qt::Key_Tab))
    {
        emit ctrlTabPressed();
        return;
    }

    if ((event->modifiers() == Qt::ControlModifier + Qt::ShiftModifier) && (event->key() == Qt::Key_Backtab))
    {
        emit shiftCtrlTabPressed();
        return;
    }

    QWebView::keyPressEvent(event);
}
