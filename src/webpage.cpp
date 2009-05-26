/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2008 Benjamin C. Meyer <ben@meyerhome.net>
* Copyright (C) 2009 by Andrea Diamantini <adjam7 at gmail dot com>
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
#include "webpage.h"
#include "webpage.moc"

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
#include "webview.h"

// KDE Includes
#include <KStandardDirs>
#include <KUrl>
#include <KActionCollection>
#include <KDebug>
#include <KToolInvocation>

#include <kdewebkit/kwebpage.h>
#include <kdewebkit/kwebview.h>

// Qt Includes
#include <QtGui/QContextMenuEvent>
#include <QtGui/QWheelEvent>
#include <QtGui/QMouseEvent>
#include <QtGui/QClipboard>
#include <QtGui/QKeyEvent>

#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>

#include <QtWebKit/QWebFrame>
#include <QtWebKit/QWebHitTestResult>
#include <QtWebKit/QWebPage>
#include <QtWebKit/QWebSettings>
#include <QtWebKit/QWebView>

// #include <QtUiTools/QUiLoader>


WebPage::WebPage(QObject *parent)
        : KWebPage(parent)
{
    setForwardUnsupportedContent(true);
    setNetworkAccessManager(Application::networkAccessManager());
}


bool WebPage::acceptNavigationRequest(QWebFrame *frame, const QNetworkRequest &request, NavigationType type)
{

    // TODO: implement ioslaves protocols
    QString scheme = request.url().scheme();
    if (scheme == QLatin1String("mailto"))
    {
        KToolInvocation::invokeMailer(request.url());
        return false;
    }


    switch (type)
    {
    // user activated a submit button for an HTML form.
    case QWebPage::NavigationTypeFormSubmitted:
        kDebug() << "NavigationTypeFormSubmitted";
        kDebug() << request.url();
        break;

    // An HTML form was submitted a second time.
    case QWebPage::NavigationTypeFormResubmitted:
        kDebug() << "NavigationTypeFormResubmitted";
        break;

    // A navigation to another document using a method not listed above.
    case QWebPage::NavigationTypeOther:
        kDebug() << "NavigationTypeOther";
        break;

    // user clicked on a link or pressed return on a focused link.
    case QWebPage::NavigationTypeLinkClicked:
        kDebug() << "NavigationTypeLinkClicked";
        break;

    // Navigation to a previously shown document in the back or forward history is requested.
    case QWebPage::NavigationTypeBackOrForward:
        kDebug() << "NavigationTypeBackOrForward";
        break;

        // user activated the reload action.
    case QWebPage::NavigationTypeReload:
        kDebug() << "NavigationTypeReload";
        break;

        // should be nothing..
    default:
        kDebug() << "Default NON extant case..";
        break;
    }

    return QWebPage::acceptNavigationRequest(frame, request, type);
}


KWebPage *WebPage::createWindow(QWebPage::WebWindowType type)
{
    kDebug() << "creating window as new tab.. ";

    // added to manage web modal dialogs
    if (type == QWebPage::WebModalDialog)
    {
        // FIXME : need a "real" implementation..
        kDebug() << "Modal Dialog ---------------------------------------";
    }

    WebView *w = Application::instance()->newWebView();
    return w->page();
}


void WebPage::slotHandleUnsupportedContent(QNetworkReply *reply)
{
    // create convenience fake api:// protocol for KDE apidox search and Qt docs
    if (reply->url().scheme() == "api")
    {
        QString path;
        QString className = reply->url().host().toLower();
        if (className[0] == 'k')
        {
            path = QString("http://api.kde.org/new.classmapper.php?class=%1").arg(className);
        }
        else if (className[0] == 'q')
        {
            path = QString("http://doc.trolltech.com/4.5/%1.html").arg(className);
        }
        KUrl url(path);

        Application::instance()->mainWindow()->loadUrl(url);
        return;
    }

    if (reply->error() == QNetworkReply::NoError)
    {
        return slotDownloadRequested(reply->request(), reply);
//         // st iframe unwanted download fix
//         if (reply->header(QNetworkRequest::ContentTypeHeader).isValid())
//         {
//             KUrl srcUrl = reply->url();
//             Application::downloadManager()->newDownload(srcUrl);
//         }
//         else
//         {
//              kDebug() << "invalid content type header";
//         }
//         return;
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
        {
            frames.append(frame);
        }
    }
    if (m_loadingUrl == reply->url())
    {
        mainFrame()->setHtml(html, reply->url());
        // Don't put error pages to the history.
        Application::historyManager()->removeHistoryEntry(reply->url(), mainFrame()->title());
    }
}
