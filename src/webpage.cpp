/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2008 Benjamin C. Meyer <ben@meyerhome.net>
* Copyright (C) 2008 Dirk Mueller <mueller@kde.org>
* Copyright (C) 2008 Urs Wolfer <uwolfer @ kde.org>
* Copyright (C) 2008 Michael Howell <mhowell123@gmail.com>
* Copyright (C) 2008-2009 by Andrea Diamantini <adjam7 at gmail dot com>
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
#include "webpage.h"
#include "webpage.moc"

// Auto Includes
#include "rekonq.h"

// Local Includes
#include "application.h"
#include "mainwindow.h"
#include "mainview.h"
#include "webview.h"
#include "webpluginfactory.h"

// KDE Includes
#include <KStandardDirs>
#include <KUrl>
#include <KDebug>
#include <KToolInvocation>
#include <KProtocolManager>

#include <kparts/browseropenorsavequestion.h>

#include <KDE/KMimeTypeTrader>
#include <KDE/KRun>
#include <KDE/KFileDialog>
#include <KDE/KMessageBox>
#include <KDE/KJobUiDelegate>

// Qt Includes
#include <QtGui/QContextMenuEvent>
#include <QtGui/QWheelEvent>
#include <QtGui/QMouseEvent>
#include <QtGui/QClipboard>
#include <QtGui/QKeyEvent>


WebPage::WebPage(QObject *parent, qlonglong windowId)
        : KWebPage(parent, windowId)
        , m_keyboardModifiers(Qt::NoModifier)
        , m_pressedButtons(Qt::NoButton)
{
    setPluginFactory(new WebPluginFactory(this));
    
    setForwardUnsupportedContent(true);

    connect(networkAccessManager(), SIGNAL(finished(QNetworkReply*)), this, SLOT(manageNetworkErrors(QNetworkReply*)));
    
    connect(this, SIGNAL(downloadRequested(const QNetworkRequest &)), this, SLOT(downloadRequest(const QNetworkRequest &)));
    connect(this, SIGNAL(unsupportedContent(QNetworkReply *)), this, SLOT(handleUnsupportedContent(QNetworkReply *)));
}


WebPage::~WebPage()
{
}


bool WebPage::acceptNavigationRequest(QWebFrame *frame, const QNetworkRequest &request, NavigationType type)
{
    // advise users on resubmitting data
    if(type == QWebPage::NavigationTypeFormResubmitted)
    {
        int risp = KMessageBox::warningContinueCancel(view(), 
                                                      i18n("Are you sure you want to send your data again?"), 
                                                      i18n("Resend form data") );
        if(risp == KMessageBox::Cancel)
            return false;
    }
    
    if (m_keyboardModifiers & Qt::ControlModifier || m_pressedButtons == Qt::MidButton)
    {
        Application::instance()->loadUrl(request.url(), Rekonq::SettingOpenTab);
        m_keyboardModifiers = Qt::NoModifier;
        m_pressedButtons = Qt::NoButton;
        return false;
    }

    if (request.url().scheme() == QLatin1String("mailto"))
    {
        KToolInvocation::invokeMailer(request.url());
        return false;
    }

    if (request.url().scheme() == QLatin1String("about"))
    {
        Application::instance()->mainWindow()->newTabPage( request.url() );
        return true;
    }
    
    m_requestedUrl = request.url();

    return KWebPage::acceptNavigationRequest(frame, request, type);
}


WebPage *WebPage::createWindow(QWebPage::WebWindowType type)
{
    kDebug() << "WebPage createWindow slot";

    // added to manage web modal dialogs
    if (type == QWebPage::WebModalDialog)
        kDebug() << "Modal Dialog";

    WebView *w = 0;
    if(ReKonfig::openTabNoWindow())
    {
        w = Application::instance()->mainWindow()->mainView()->newWebView(!ReKonfig::openTabsBack());
    }
    else
    {
        w = Application::instance()->newMainWindow()->mainView()->currentWebView();
    }
    return w->page();
}


void WebPage::handleUnsupportedContent(QNetworkReply *reply)
{
    if (reply->error() == QNetworkReply::NoError)
    {
        const KUrl url(reply->request().url());

        QString mimeType = reply->header(QNetworkRequest::ContentTypeHeader).toString();
        KService::Ptr offer = KMimeTypeTrader::self()->preferredService(mimeType);

        if( offer.isNull() ) // no service can handle this. We can just download it..
        {
            downloadRequest(reply->request());
            return;
        }

        KParts::BrowserOpenOrSaveQuestion dlg(Application::instance()->mainWindow(), url, mimeType);
        switch ( dlg.askEmbedOrSave() )
        {
            case KParts::BrowserOpenOrSaveQuestion::Save:
                downloadRequested(reply->request());
                return;
            case KParts::BrowserOpenOrSaveQuestion::Cancel:
                return;
            default: // non extant case
                break;
        }
        // case KParts::BrowserRun::Embed
        KUrl::List list;
        list.append(url);
        KRun::run(*offer,url,0);
    }
}


void WebPage::manageNetworkErrors(QNetworkReply* reply)
{
    if( reply->error() == QNetworkReply::NoError )
        return;


    if( reply->url() != m_requestedUrl ) // prevent favicon loading
        return;
    
    if( reply->error() == QNetworkReply::ContentNotFoundError )
    {
        QList<QWebFrame*> frames;
        frames.append(mainFrame());
        while (!frames.isEmpty())
        {
            QWebFrame *firstFrame = frames.takeFirst();

            if (firstFrame->url() == reply->url())
            {
                firstFrame->setHtml(errorPage(reply), reply->url());
                return;
            }
            QList<QWebFrame *> children = firstFrame->childFrames();
            Q_FOREACH(QWebFrame *frame, children)
            {
                frames.append(frame);
            }
        }
    }
    else
    {
        mainFrame()->setHtml(errorPage(reply), reply->url());
    }
}


QString WebPage::errorPage(QNetworkReply *reply)
{
    // display "not found" page
    QString notfoundFilePath =  KStandardDirs::locate("data", "rekonq/htmls/notfound.html");
    QFile file(notfoundFilePath);
    bool isOpened = file.open(QIODevice::ReadOnly);
    if (!isOpened)
    {
        kWarning() << "Couldn't open the notfound.html file";
        return QString("");
    }

    QString title = i18n("Error loading: %1", reply->url().path()); 
    
    QString imagesPath = QString("file://") + KGlobal::dirs()->findResourceDir("data", "rekonq/pics/bg.png") + QString("rekonq/pics");

    QString msg = "<h1>" + reply->errorString() + "</h1>";
    
    msg += "<h2>" + i18nc("%1=an URL, e.g.'kde.org'", "When connecting to: %1", reply->url().toString()) + "</h2>";
    msg += "<ul><li>" + i18n("Check the address for errors such as <b>ww</b>.kde.org instead of <b>www</b>.kde.org");
    msg += "</li><li>" + i18n("If the address is correct, try to check the network connection.") + "</li><li>" ;
    msg += i18n("If your computer or network is protected by a firewall or proxy, make sure that rekonq is permitted to access the network.");
    msg += "</li><li>" + i18n("Of course, if rekonq does not work properly, you can always say it is a programmer error ;)");
    msg += "</li></ul><br/><br/>";
    msg += "<input type=\"button\" id=\"reloadButton\" onClick=\"document.location.href='" + reply->url().path() + "';\" value=\"";
    msg += i18n("Try Again") + "\" />";
    
    QString html = QString(QLatin1String(file.readAll()))
                            .arg(title)
                            .arg(imagesPath)
                            .arg(msg)
                            ;
    return html;
}


bool WebPage::authorizedRequest(const QUrl &url) const
{
    Q_UNUSED(url)
    // TODO implement ad-block here
    return true;
}

