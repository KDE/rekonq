/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2008 Benjamin C. Meyer <ben@meyerhome.net>
* Copyright (C) 2008 Dirk Mueller <mueller@kde.org>
* Copyright (C) 2008 Urs Wolfer <uwolfer @ kde.org>
* Copyright (C) 2008 Michael Howell <mhowell123@gmail.com>
* Copyright (C) 2008-2012 by Andrea Diamantini <adjam7 at gmail dot com>
* Copyright (C) 2010 by Matthieu Gicquel <matgic78 at gmail dot com>
* Copyright (C) 2009-2010 Dawit Alemayehu <adawit at kde dot org>
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
#include "adblockmanager.h"
#include "application.h"
#include "downloadmanager.h"
#include "iconmanager.h"
#include "mainview.h"
#include "mainwindow.h"
#include "networkaccessmanager.h"
#include "urlbar.h"
#include "webpluginfactory.h"
#include "websnap.h"
#include "webtab.h"
#include "searchengine.h"
#include "sslwidget.h"
#include "sslinfodialog.h"

// KDE Includes
#include <KTemporaryFile>
#include <KStandardDirs>
#include <KJobUiDelegate>
#include <KLocalizedString>
#include <KMessageBox>
#include <KMimeTypeTrader>
#include <KService>
#include <KWebWallet>
#include <KProtocolInfo>
#include <KRun>

#include <KIO/JobUiDelegate>

#include <kparts/browseropenorsavequestion.h>

// Qt Includes
#include <QtGui/QTextDocument>


// Returns true if the scheme and domain of the two urls match...
static bool domainSchemeMatch(const QUrl& u1, const QUrl& u2)
{
    if (u1.scheme() != u2.scheme())
        return false;

    QStringList u1List = u1.host().split(QL1C('.'), QString::SkipEmptyParts);
    QStringList u2List = u2.host().split(QL1C('.'), QString::SkipEmptyParts);

    if (qMin(u1List.count(), u2List.count()) < 2)
        return false;  // better safe than sorry...

    while (u1List.count() > 2)
        u1List.removeFirst();

    while (u2List.count() > 2)
        u2List.removeFirst();

    return (u1List == u2List);
}


static void extractMimeType(const QNetworkReply* reply, QString& mimeType)
{
    mimeType.clear();
    const KIO::MetaData& metaData = reply->attribute(static_cast<QNetworkRequest::Attribute>(KIO::AccessManager::MetaData)).toMap();
    if (metaData.contains(QL1S("content-type")))
        mimeType = metaData.value(QL1S("content-type"));

    if (!mimeType.isEmpty())
        return;

    if (!reply->hasRawHeader("Content-Type"))
        return;

    const QString value(QL1S(reply->rawHeader("Content-Type").simplified().constData()));
    const int index = value.indexOf(QL1C(';'));
    if (index == -1)
        mimeType = value;
    else
        mimeType = value.left(index);
}


// ---------------------------------------------------------------------------------


WebPage::WebPage(QWidget *parent)
    : KWebPage(parent, KWalletIntegration)
    , _networkAnalyzer(false)
    , _isOnRekonqPage(false)
{
    // ----- handling unsupported content...
    setForwardUnsupportedContent(true);
    connect(this, SIGNAL(unsupportedContent(QNetworkReply*)), this, SLOT(handleUnsupportedContent(QNetworkReply*)));

    // ----- rekonq Network Manager
    NetworkAccessManager *manager = new NetworkAccessManager(this);

    // disable QtWebKit cache to just use KIO one..
    manager->setCache(0);

    // set cookieJar window..
    if (parent && parent->window())
        manager->setWindow(parent->window());

    // set network reply object to emit readyRead when it receives meta data
    manager->setEmitReadyReadOnMetaDataChange(true);

    setNetworkAccessManager(manager);

    // activate ssl warnings
    setSessionMetaData(QL1S("ssl_activate_warnings"), QL1S("TRUE"));

    // ----- Web Plugin Factory
    setPluginFactory(new WebPluginFactory(this));

    // ----- last stuffs
    connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(manageNetworkErrors(QNetworkReply*)));

    connect(this, SIGNAL(downloadRequested(QNetworkRequest)), this, SLOT(downloadRequest(QNetworkRequest)));
    connect(this, SIGNAL(loadStarted()), this, SLOT(loadStarted()));
    connect(this, SIGNAL(loadFinished(bool)), this, SLOT(loadFinished(bool)));

    // protocol handler signals
    connect(&_protHandler, SIGNAL(downloadUrl(KUrl)), this, SLOT(downloadUrl(KUrl)));

    connect(rApp->iconManager(), SIGNAL(iconChanged()), mainFrame(), SIGNAL(iconChanged()));
}


WebPage::~WebPage()
{
    disconnect();

    QPixmap preview = WebSnap::renderPagePreview(*this);
    QString path = WebSnap::imagePathFromUrl(mainFrame()->url().toString());
    QFile::remove(path);
    preview.save(path);
}


bool WebPage::acceptNavigationRequest(QWebFrame *frame, const QNetworkRequest &request, NavigationType type)
{
    if (_isOnRekonqPage)
    {
        WebView *view = qobject_cast<WebView *>(parent());
        WebTab *tab = qobject_cast<WebTab *>(view->parent());
        _isOnRekonqPage = false;
        tab->setPart(0, KUrl());     // re-enable the view page
    }

    // reset webpage values
    _suggestedFileName.clear();
    _loadingUrl = request.url();

    const bool isMainFrameRequest = (frame == mainFrame());

    if (frame)
    {
        if (_protHandler.preHandling(request, frame))
        {
            return false;
        }

        switch (type)
        {
        case QWebPage::NavigationTypeLinkClicked:
            if (_sslInfo.isValid())
            {
                setRequestMetaData("ssl_was_in_use", "TRUE");
            }
            break;

        case QWebPage::NavigationTypeFormSubmitted:
            break;

        case QWebPage::NavigationTypeFormResubmitted:
            if (KMessageBox::warningContinueCancel(view(),
                                                   i18n("Are you sure you want to send your data again?"),
                                                   i18n("Resend form data")
                                                  )
                    == KMessageBox::Cancel)
            {
                return false;
            }
            break;

        case QWebPage::NavigationTypeReload:
            setRequestMetaData( QL1S("cache"), QL1S("reload") );
            break;
            
        case QWebPage::NavigationTypeBackOrForward:
        case QWebPage::NavigationTypeOther:
            break;

        default:
            ASSERT_NOT_REACHED();
            break;
        }
    }

    // Get the SSL information sent, if any...
    KIO::AccessManager *manager = qobject_cast<KIO::AccessManager*>(networkAccessManager());
    KIO::MetaData metaData = manager->requestMetaData();
    if (metaData.contains(QL1S("ssl_in_use")))
    {
        WebSslInfo info;
        info.restoreFrom(metaData.toVariant(), request.url());
        info.setUrl(request.url());
        _sslInfo = info;
    }

    if (isMainFrameRequest)
    {
        setRequestMetaData(QL1S("main_frame_request"), QL1S("TRUE"));
        if (_sslInfo.isValid() && !domainSchemeMatch(request.url(), _sslInfo.url()))
        {
            _sslInfo = WebSslInfo();
        }
    }
    else
    {
        setRequestMetaData(QL1S("main_frame_request"), QL1S("FALSE"));
    }

    return KWebPage::acceptNavigationRequest(frame, request, type);
}


WebPage *WebPage::createWindow(QWebPage::WebWindowType type)
{
    // added to manage web modal dialogs
    if (type == QWebPage::WebModalDialog)
        kDebug() << "Modal Dialog";

    WebTab *w = 0;
    if (ReKonfig::openLinksInNewWindow())
    {
        w = rApp->newMainWindow()->mainView()->currentWebTab();
    }
    else
    {
        w = rApp->mainWindow()->mainView()->newWebTab(!ReKonfig::openNewTabsInBackground());
    }
    return w->page();
}


void WebPage::handleUnsupportedContent(QNetworkReply *reply)
{
    Q_ASSERT(reply);

    if (!reply)
        return;

    // handle protocols WebKit cannot handle...
    if (_protHandler.postHandling(reply->request(), mainFrame()))
    {
        return;
    }

    if (reply->error() != QNetworkReply::NoError)
        return;

    KIO::Integration::AccessManager::putReplyOnHold(reply);

    // Get mimeType...
    extractMimeType(reply, _mimeType);

    // Convert executable text files to plain text...
    if (KParts::BrowserRun::isTextExecutable(_mimeType))
        _mimeType = QL1S("text/plain");

    // Get suggested file name...
    const KIO::MetaData& data = reply->attribute(static_cast<QNetworkRequest::Attribute>(KIO::AccessManager::MetaData)).toMap();
    _suggestedFileName = data.value(QL1S("content-disposition-filename"));

    kDebug() << "Detected MimeType = " << _mimeType;
    kDebug() << "Suggested File Name = " << _suggestedFileName;
    // ------------------------------------------------

    KService::Ptr appService = KMimeTypeTrader::self()->preferredService(_mimeType);

    KUrl replyUrl = reply->url();
    bool isLocal = replyUrl.isLocalFile();

    if (appService.isNull())  // no service can handle this. We can just download it..
    {
        isLocal
        ? KMessageBox::sorry(view(), i18n("No service can handle this file."))
        : downloadUrl(reply->url());

        return;
    }

    if (isLocal)
    {
        // Load outside local files
        KRun::run(*appService, replyUrl, 0, false, _suggestedFileName);
    }
    else
    {
        KParts::BrowserOpenOrSaveQuestion dlg(rApp->mainWindow(), replyUrl, _mimeType);

        if (!_suggestedFileName.isEmpty())
            dlg.setSuggestedFileName(_suggestedFileName);

        // read askEmbedOrSave preferences. If we don't have to show dialog and rekonq settings are
        // to automatically choose download dir, we won't show local dir choose dialog
        KConfigGroup cg = KConfigGroup(KSharedConfig::openConfig("filetypesrc", KConfig::NoGlobals), QL1S("Notification Messages"));
        bool hideDialog = cg.readEntry(QL1S("askEmbedOrSave") + _mimeType, false);

        kDebug() << "Hide dialog for " << _mimeType << "? " << hideDialog;

        switch (dlg.askEmbedOrSave())
        {
        case KParts::BrowserOpenOrSaveQuestion::Save:
            rApp->downloadManager()->downloadResource(reply->url(), KIO::MetaData(), view(), !hideDialog, _suggestedFileName);
            return;

        case KParts::BrowserOpenOrSaveQuestion::Cancel:
            return;

        default: // Can happen when "Open with.." is set and "don't ask again" is checked
            break;
        }
    }

    // Handle Post operations that return content...
    if (reply->operation() == QNetworkAccessManager::PostOperation)
    {
        kDebug() << "POST OPERATION: downloading file...";
        QFileInfo finfo(_suggestedFileName.isEmpty() ? _loadingUrl.fileName() : _suggestedFileName);
        KTemporaryFile tempFile;
        tempFile.setSuffix(QL1C('.') + finfo.suffix());
        tempFile.setAutoRemove(false);
        tempFile.open();
        KUrl destUrl;
        destUrl.setPath(tempFile.fileName());
        KIO::Job *job = KIO::file_copy(_loadingUrl, destUrl, 0600, KIO::Overwrite);
        job->ui()->setWindow(rApp->mainWindow());
        connect(job, SIGNAL(result(KJob*)), this, SLOT(copyToTempFileResult(KJob*)));
        return;
    }

    // case KParts::BrowserRun::Embed
    KParts::ReadOnlyPart *pa = KMimeTypeTrader::createPartInstanceFromQuery<KParts::ReadOnlyPart>(_mimeType, view(), this, QString());
    if (pa)
    {
        _isOnRekonqPage = true;

        WebView *view = qobject_cast<WebView *>(parent());
        WebTab *tab = qobject_cast<WebTab *>(view->parent());
        tab->setPart(pa, replyUrl);

        UrlBar *bar = tab->urlBar();
        bar->setQUrl(replyUrl);

        rApp->mainWindow()->updateHistoryActions();
    }
    else
    {
        // No parts, just app services. Load it!
        // If the app is a KDE one, publish the slave on hold to let it use it.
        // Otherwise, run the app and remove it (the io slave...)
        KRun::run(*appService, replyUrl, 0, false, _suggestedFileName);
    }

    return;
}


void WebPage::loadStarted()
{
}


void WebPage::loadFinished(bool ok)
{
    Q_UNUSED(ok);

    // set zoom factor
    QString val;
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group(config, "Zoom");
    val = group.readEntry(_loadingUrl.host(), QString("10"));

    int value = val.toInt();
    mainFrame()->setZoomFactor(QVariant(value).toReal() / 10);  // Don't allox max +1 values

    // Provide site icon. Can this be moved to loadStarted??
    rApp->iconManager()->provideIcon(mainFrame(), _loadingUrl);

    // Apply adblock manager hiding rules
    rApp->adblockManager()->applyHidingRules(this);

    // KWallet Integration
    QStringList list = ReKonfig::walletBlackList();
    if (wallet()
            && !list.contains(mainFrame()->url().toString())
       )
    {
        wallet()->fillFormData(mainFrame());
    }
}


void WebPage::manageNetworkErrors(QNetworkReply *reply)
{
    Q_ASSERT(reply);

    QWebFrame* frame = qobject_cast<QWebFrame *>(reply->request().originatingObject());
    if (!frame)
        return;
    
    const bool isMainFrameRequest = (frame == mainFrame());

    // Only deal with non-redirect responses...
    const QVariant redirectVar = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
    if (redirectVar.isValid())
    {
        _sslInfo.restoreFrom(reply->attribute(static_cast<QNetworkRequest::Attribute>(KIO::AccessManager::MetaData)),
                              reply->url());
        return;
    }
    
    // We are just managing loading URLs errors
    if (reply->request().url() != _loadingUrl)
        return;
    
    // NOTE: These are not all networkreply errors,
    // but just that supported directly by KIO
    switch (reply->error())
    {

    case QNetworkReply::NoError:                             // no error. Simple :)
        if (isMainFrameRequest)
        {
            // Obtain and set the SSL information if any...
            _sslInfo.restoreFrom(reply->attribute(static_cast<QNetworkRequest::Attribute>(KIO::AccessManager::MetaData)), reply->url());
            _sslInfo.setUrl(reply->url());
        }
        break;

    case QNetworkReply::OperationCanceledError:              // operation canceled via abort() or close() calls
        // ignore this..
        return;

    // WARNING: This is also typical adblocked element error: IGNORE THIS!
    case QNetworkReply::ContentAccessDenied:                 // access to remote content denied
        break;

    case QNetworkReply::UnknownNetworkError:                 // unknown network-related error detected
    case QNetworkReply::ConnectionRefusedError:              // remote server refused connection
    case QNetworkReply::HostNotFoundError:                   // invalid hostname
    case QNetworkReply::TimeoutError:                        // connection time out
    case QNetworkReply::ProxyNotFoundError:                  // invalid proxy hostname
    case QNetworkReply::ContentOperationNotPermittedError:   // operation requested on remote content not permitted
    case QNetworkReply::ContentNotFoundError:                // remote content not found on server (similar to HTTP error 404)
    case QNetworkReply::ProtocolUnknownError:                // Unknown protocol
    case QNetworkReply::ProtocolInvalidOperationError:       // requested operation is invalid for this protocol
    default:
        kDebug() << "ERROR " << reply->error() << ": " << reply->errorString();
        if (reply->url() == _loadingUrl)
        {
            frame->setHtml(errorPage(reply));
            if (isMainFrameRequest)
            {
                _isOnRekonqPage = true;

                WebView *view = qobject_cast<WebView *>(parent());
                WebTab *tab = qobject_cast<WebTab *>(view->parent());
                UrlBar *bar = tab->urlBar();
                bar->setQUrl(_loadingUrl);

                rApp->mainWindow()->updateHistoryActions();
            }
        }
        break;

    }
}


QString WebPage::errorPage(QNetworkReply *reply)
{
    // display "not found" page
    QString notfoundFilePath =  KStandardDirs::locate("data", "rekonq/htmls/rekonqinfo.html");
    QFile file(notfoundFilePath);

    bool isOpened = file.open(QIODevice::ReadOnly);
    if (!isOpened)
    {
        return QString("Couldn't open the rekonqinfo.html file");
    }

    QString title = i18n("There was a problem while loading the page");

    // NOTE:
    // this, to take care about XSS (see BUG 217464)...
    QString urlString = Qt::escape(reply->url().toString());

    QString iconPath = QString("file://") + KIconLoader::global()->iconPath("dialog-warning" , KIconLoader::Small);
    iconPath.replace(QL1S("16"), QL1S("128"));

    QString msg;
    msg += QL1S("<table>");
    msg += QL1S("<tr><td>");
    msg += QL1S("<img src=\"") + iconPath + QL1S("\" />");
    msg += QL1S("</td><td>");
    msg += QL1S("<h1>") + reply->errorString() + QL1S("</h1>");
    msg += QL1S("<h2>") + i18nc("%1=an URL, e.g.'kde.org'", "When connecting to: <b>%1</b>", urlString) + QL1S("</h2>");
    msg += QL1S("</td></tr></table>");

    msg += QL1S("<ul><li>") + i18n("Check the address for errors such as <b>ww</b>.kde.org instead of <b>www</b>.kde.org");
    msg += QL1S("</li><li>") + i18n("If the address is correct, try to check the network connection.") + QL1S("</li><li>") ;
    msg += i18n("If your computer or network is protected by a firewall or proxy, make sure that rekonq is permitted to access the network.");
    msg += QL1S("</li><li>") + i18n("Of course, if rekonq does not work properly, you can always say it is a programmer error ;)");
    msg += QL1S("</li></ul><br/><br/>");
    msg += QL1S("<input type=\"button\" id=\"reloadButton\" onClick=\"document.location.href='") + urlString + QL1S("';\" value=\"");
    msg += i18n("Try Again") + QL1S("\" />");

    //Default SearchEngine
    KService::Ptr defaultEngine = SearchEngine::defaultEngine();

    if (defaultEngine)
    {
        msg += i18n("or");
        msg += QL1S(" <a href=\"") + SearchEngine::buildQuery(defaultEngine, urlString) + QL1S("\">");
        msg += i18n("Search with %1", defaultEngine->name()) + QL1S("</a>");
    }

    QString html = QString(QL1S(file.readAll()))
                   .arg(title)
                   .arg(msg)
                   ;
    return html;
}


void WebPage::downloadRequest(const QNetworkRequest &request)
{
    rApp->downloadManager()->downloadResource(request.url(),
            request.attribute(static_cast<QNetworkRequest::Attribute>(KIO::AccessManager::MetaData)).toMap(),
            view());
}


void WebPage::downloadUrl(const KUrl &url)
{
    rApp->downloadManager()->downloadResource(url, KIO::MetaData(), view());
}


void WebPage::downloadAllContentsWithKGet()
{
    QSet<QString> contents;
    KUrl baseUrl(currentFrame()->url());
    KUrl relativeUrl;

    QWebElementCollection images = mainFrame()->documentElement().findAll("img");
    Q_FOREACH(const QWebElement & img, images)
    {
        relativeUrl.setEncodedUrl(img.attribute("src").toUtf8(), KUrl::TolerantMode);
        contents << baseUrl.resolved(relativeUrl).toString();
    }

    QWebElementCollection links = mainFrame()->documentElement().findAll("a");
    Q_FOREACH(const QWebElement & link, links)
    {
        relativeUrl.setEncodedUrl(link.attribute("href").toUtf8(), KUrl::TolerantMode);
        contents << baseUrl.resolved(relativeUrl).toString();
    }

    rApp->downloadManager()->downloadLinksWithKGet(QVariant(contents.toList()));
}


void WebPage::showSSLInfo(QPoint pos)
{
    if (mainFrame()->url().scheme() == QL1S("https"))
    {
        SSLWidget *widget = new SSLWidget(mainFrame()->url(), _sslInfo, view());
        widget->showAt(pos);
    }
    else
    {
        KMessageBox::information(view(),
                                 i18n("This site does not contain SSL information."),
                                 i18nc("Secure Sockets Layer", "SSL")
                                );
    }
}


void WebPage::copyToTempFileResult(KJob* job)
{
    if (job->error())
        job->uiDelegate()->showErrorMessage();
    else
        (void)KRun::runUrl(static_cast<KIO::FileCopyJob *>(job)->destUrl(), _mimeType, rApp->mainWindow());
}


bool WebPage::hasSslValid() const
{
    QList<QSslCertificate> certList = _sslInfo.certificateChain();

    if (certList.isEmpty())
        return false;

    Q_FOREACH(const QSslCertificate & cert, certList)
    {
        if (!cert.isValid())
            return false;
    }

    QList<QStringList> errorList = SslInfoDialog::errorsFromString(_sslInfo.certificateErrors());
    Q_FOREACH(const QStringList & list, errorList)
    {
        if (!list.isEmpty())
            return false;
    }
    return true;
}
