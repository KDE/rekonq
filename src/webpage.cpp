/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2008 Benjamin C. Meyer <ben@meyerhome.net>
* Copyright (C) 2008 Dirk Mueller <mueller@kde.org>
* Copyright (C) 2008 Urs Wolfer <uwolfer @ kde.org>
* Copyright (C) 2008 Michael Howell <mhowell123@gmail.com>
* Copyright (C) 2008-2011 by Andrea Diamantini <adjam7 at gmail dot com>
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
#include "newtabpage.h"
#include "urlbar.h"
#include "webpluginfactory.h"
#include "webtab.h"

#include "sslinfodialog_p.h"

// KDE Includes
#include <KIO/Job>
#include <KIO/RenameDialog>
#include <KIO/JobUiDelegate>

#include <KTemporaryFile>
#include <KStandardDirs>
#include <KFileDialog>
#include <KJobUiDelegate>
#include <KLocalizedString>
#include <KMessageBox>
#include <KMimeTypeTrader>
#include <KService>
#include <KToolInvocation>
#include <KWebWallet>
#include <KProtocolInfo>
#include <KRun>

#include <kparts/browseropenorsavequestion.h>

#include <kio/scheduler.h>

// Qt Includes
#include <QtCore/QFileInfo>

#include <QtGui/QTextDocument>

#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusConnectionInterface>
#include <QtDBus/QDBusInterface>


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


// NOTE
// These 2 functions have been copied from the KWebPage class to implement a local version of the downloadResponse method.
// In this way, we can easily provide the extra functionality we need:
// 1. KGet Integration
// 2. Save downloads history
static void extractSuggestedFileName(const QNetworkReply* reply, QString& fileName)
{
    fileName.clear();
    const KIO::MetaData& metaData = reply->attribute(static_cast<QNetworkRequest::Attribute>(KIO::AccessManager::MetaData)).toMap();
    if (metaData.value(QL1S("content-disposition-type")).compare(QL1S("attachment"), Qt::CaseInsensitive) == 0)
        fileName = metaData.value(QL1S("content-disposition-filename"));

    if (!fileName.isEmpty())
        return;

    if (!reply->hasRawHeader("Content-Disposition"))
        return;

    const QString value(QL1S(reply->rawHeader("Content-Disposition").simplified().constData()));
    if (value.startsWith(QL1S("attachment"), Qt::CaseInsensitive) || value.startsWith(QL1S("inline"), Qt::CaseInsensitive))
    {
        const int length = value.size();
        int pos = value.indexOf(QL1S("filename"), 0, Qt::CaseInsensitive);
        if (pos > -1)
        {
            pos += 9;
            while (pos < length && (value.at(pos) == QL1C(' ') || value.at(pos) == QL1C('=') || value.at(pos) == QL1C('"')))
                pos++;

            int endPos = pos;
            while (endPos < length && value.at(endPos) != QL1C('"') && value.at(endPos) != QL1C(';'))
                endPos++;

            if (endPos > pos)
                fileName = value.mid(pos, (endPos - pos)).trimmed();
        }
    }
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


static bool downloadResource(const KUrl& srcUrl, const KIO::MetaData& metaData = KIO::MetaData(),
                             QWidget * parent = 0, const QString & suggestedName = QString())
{
    KUrl destUrl;

    int result = KIO::R_OVERWRITE;
    const QString fileName((suggestedName.isEmpty() ? srcUrl.fileName() : suggestedName));

    do
    {
        // follow bug:184202 fixes
        destUrl = KFileDialog::getSaveFileName(KUrl(KGlobalSettings::downloadPath().append(fileName)), QString(), parent);

        if (destUrl.isEmpty())
            return false;

        if (destUrl.isLocalFile())
        {
            QFileInfo finfo(destUrl.toLocalFile());
            if (finfo.exists())
            {
                QDateTime now = QDateTime::currentDateTime();
                QPointer<KIO::RenameDialog> dlg = new KIO::RenameDialog(parent,
                        i18n("Overwrite File?"),
                        srcUrl,
                        destUrl,
                        KIO::RenameDialog_Mode(KIO::M_OVERWRITE | KIO::M_SKIP),
                        -1,
                        finfo.size(),
                        now.toTime_t(),
                        finfo.created().toTime_t(),
                        now.toTime_t(),
                        finfo.lastModified().toTime_t()
                                                                       );
                result = dlg->exec();
                delete dlg;
            }
        }
    }
    while (result == KIO::R_CANCEL && destUrl.isValid());

    // Save download history
    DownloadItem *item = rApp->downloadManager()->addDownload(srcUrl.pathOrUrl(), destUrl.pathOrUrl());

    if (!KStandardDirs::findExe("kget").isNull() && ReKonfig::kgetDownload())
    {
        //KGet integration:
        if (!QDBusConnection::sessionBus().interface()->isServiceRegistered("org.kde.kget"))
        {
            KToolInvocation::kdeinitExecWait("kget");
        }
        QDBusInterface kget("org.kde.kget", "/KGet", "org.kde.kget.main");
        if (!kget.isValid())
            return false;

        QDBusMessage transfer = kget.call(QL1S("addTransfer"), srcUrl.prettyUrl(), destUrl.prettyUrl(), true);
        if (transfer.arguments().isEmpty())
            return true;
        
        const QString transferPath = transfer.arguments().first().toString();
        item->setKGetTransferDbusPath(transferPath);
        return true;
    }

    KIO::Job *job = KIO::file_copy(srcUrl, destUrl, -1, KIO::Overwrite);
    if (item)
    {
        QObject::connect(job, SIGNAL(percent(KJob *,unsigned long)), item, SLOT(updateProgress(KJob *,unsigned long)));
        QObject::connect(job, SIGNAL(finished(KJob *)), item, SLOT(onFinished(KJob*)));
    }
    
    if (!metaData.isEmpty())
        job->setMetaData(metaData);

    job->addMetaData(QL1S("MaxCacheSize"), QL1S("0")); // Don't store in http cache.
    job->addMetaData(QL1S("cache"), QL1S("cache")); // Use entry from cache if available.
    job->uiDelegate()->setAutoErrorHandlingEnabled(true);
    return true;
}


// ---------------------------------------------------------------------------------


WebPage::WebPage(QWidget *parent)
        : KWebPage(parent, KWalletIntegration)
        , _networkAnalyzer(false)
        , _isOnRekonqPage(false)
{
    // ----- handling unsupported content...
    setForwardUnsupportedContent(true);
    connect(this, SIGNAL(unsupportedContent(QNetworkReply *)), this, SLOT(handleUnsupportedContent(QNetworkReply *)));

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
    setSessionMetaData( QL1S("ssl_activate_warnings"), QL1S("TRUE") );

    // ----- Web Plugin Factory
    setPluginFactory(new WebPluginFactory(this));

    // ----- last stuffs
    connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(manageNetworkErrors(QNetworkReply*)));
    
    connect(this, SIGNAL(downloadRequested(const QNetworkRequest &)), this, SLOT(downloadRequest(const QNetworkRequest &)));
    connect(this, SIGNAL(loadStarted()), this, SLOT(loadStarted()));
    connect(this, SIGNAL(loadFinished(bool)), this, SLOT(loadFinished(bool)));

    // protocol handler signals
    connect(&_protHandler, SIGNAL(downloadUrl(const KUrl &)), this, SLOT(downloadUrl(const KUrl &)));

    connect(rApp->iconManager(), SIGNAL(iconChanged()), mainFrame(), SIGNAL(iconChanged()));
}


WebPage::~WebPage()
{
    disconnect();
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

    KIO::AccessManager *manager = qobject_cast<KIO::AccessManager*>(networkAccessManager());
    KIO::MetaData metaData = manager->requestMetaData();

    // Get the SSL information sent, if any...
    if (metaData.contains(QL1S("ssl_in_use")))
    {
        WebSslInfo info;
        info.restoreFrom(metaData.toVariant(), request.url());
        info.setUrl(request.url());
        _sslInfo = info;
    }

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
        case QWebPage::NavigationTypeBackOrForward:
        case QWebPage::NavigationTypeOther:
            break;

        default:
            break;
        }
    }
    return KWebPage::acceptNavigationRequest(frame, request, type);
}


WebPage *WebPage::createWindow(QWebPage::WebWindowType type)
{
    // added to manage web modal dialogs
    if (type == QWebPage::WebModalDialog)
        kDebug() << "Modal Dialog";

    WebTab *w = 0;
    if (ReKonfig::openTabNoWindow())
    {
        w = rApp->mainWindow()->mainView()->newWebTab(!ReKonfig::openTabsBack());
    }
    else
    {
        w = rApp->newMainWindow()->mainView()->currentWebTab();
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
        kDebug() << "POST HANDLING the unsupported...";
        return;
    }

    if (reply->error() != QNetworkReply::NoError)
        return;

    KIO::Integration::AccessManager::putReplyOnHold(reply);

    // Get suggested file name...
    extractSuggestedFileName(reply, _suggestedFileName);

    // Get mimeType...
    extractMimeType(reply, _mimeType);

    // Convert executable text files to plain text...
    if (KParts::BrowserRun::isTextExecutable(_mimeType))
        _mimeType = QL1S("text/plain");

    kDebug() << "Detected MimeType = " << _mimeType;
    kDebug() << "Suggested File Name = " << _suggestedFileName;
    // ------------------------------------------------

    KService::Ptr appService = KMimeTypeTrader::self()->preferredService(_mimeType);

    KUrl replyUrl = reply->url();
    bool isLocal = replyUrl.isLocalFile();
    
    if (appService.isNull())  // no service can handle this. We can just download it..
    {
        kDebug() << "no service can handle this. We can just download it..";

        isLocal
        ? KMessageBox::sorry(view(), i18n("No service can handle this file."))
        : downloadReply(reply, _suggestedFileName);

        return;
    }

    if (!isLocal)
    {
        KParts::BrowserOpenOrSaveQuestion dlg(rApp->mainWindow(), replyUrl, _mimeType);
        if (!_suggestedFileName.isEmpty())
            dlg.setSuggestedFileName(_suggestedFileName);

        switch (dlg.askEmbedOrSave())
        {
        case KParts::BrowserOpenOrSaveQuestion::Save:
            kDebug() << "user choice: no services, just download!";
            downloadReply(reply, _suggestedFileName);
            return;

        case KParts::BrowserOpenOrSaveQuestion::Cancel:
            return;

        default: // non extant case
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
        kDebug() << "First save content to" << destUrl;
        KIO::Job *job = KIO::file_copy(_loadingUrl, destUrl, 0600, KIO::Overwrite);
        job->ui()->setWindow(rApp->mainWindow());
        connect(job, SIGNAL(result(KJob *)), this, SLOT(copyToTempFileResult(KJob*)));
        return;
    }

    // case KParts::BrowserRun::Embed
    KParts::ReadOnlyPart *pa = KMimeTypeTrader::createPartInstanceFromQuery<KParts::ReadOnlyPart>(_mimeType, view(), this, QString());
    if (pa)
    {
        kDebug() << "EMBEDDING CONTENT...";

        _isOnRekonqPage = true;

        WebView *view = qobject_cast<WebView *>(parent());
        WebTab *tab = qobject_cast<WebTab *>(view->parent());
        tab->setPart(pa, replyUrl);

        UrlBar *bar = tab->urlBar();
        bar->setQUrl(replyUrl);

        rApp->mainWindow()->updateActions();
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
    kDebug() << "ZOOM VALUE: " << _loadingUrl.host() << value;
    mainFrame()->setZoomFactor(QVariant(value).toReal() / 10);  // Don't allox max +1 values

    // Provide site icon. Can this be moved to loadStarted??
    rApp->iconManager()->provideIcon(this, _loadingUrl);

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

    // check suggested file name
    if (_suggestedFileName.isEmpty())
        extractSuggestedFileName(reply, _suggestedFileName);

    QWebFrame* frame = qobject_cast<QWebFrame *>(reply->request().originatingObject());
    const bool isMainFrameRequest = (frame == mainFrame());

    if (isMainFrameRequest
            && _sslInfo.isValid()
            && !domainSchemeMatch(reply->url(), _sslInfo.url())
       )
    {
        // Reseting cached SSL info...
        _sslInfo = WebSslInfo();
    }

    // NOTE: These are not all networkreply errors,
    // but just that supported directly by KIO
    switch (reply->error())
    {

    case QNetworkReply::NoError:                             // no error. Simple :)
        if (isMainFrameRequest && !_sslInfo.isValid())
        {
            // Obtain and set the SSL information if any...
            _sslInfo.restoreFrom(reply->attribute(static_cast<QNetworkRequest::Attribute>(KIO::AccessManager::MetaData)), reply->url());
            _sslInfo.setUrl(reply->url());
        }
        break;

    case QNetworkReply::OperationCanceledError:              // operation canceled via abort() or close() calls
        // ignore this..
        return;

    case QNetworkReply::ContentAccessDenied:                 // access to remote content denied (similar to HTTP error 401)
        kDebug() << "We (hopefully) are managing this through the adblock :)";
        break;

    case QNetworkReply::UnknownNetworkError:                 // unknown network-related error detected
        kDebug() << "------------------ DO WE REALLY NEED THIS??? --------------------";
        _protHandler.postHandling(reply->request(), frame);
        return;

    case QNetworkReply::ConnectionRefusedError:              // remote server refused connection
    case QNetworkReply::HostNotFoundError:                   // invalid hostname
    case QNetworkReply::TimeoutError:                        // connection time out
    case QNetworkReply::ProxyNotFoundError:                  // invalid proxy hostname
    case QNetworkReply::ContentOperationNotPermittedError:   // operation requested on remote content not permitted
    case QNetworkReply::ContentNotFoundError:                // remote content not found on server (similar to HTTP error 404)
    case QNetworkReply::ProtocolUnknownError:                // Unknown protocol
    case QNetworkReply::ProtocolInvalidOperationError:       // requested operation is invalid for this protocol

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

                rApp->mainWindow()->updateActions();
            }
        }
        break;

    default:
        // Nothing to do here..
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
    msg += "<table>";
    msg += "<tr><td>";
    msg += "<img src=\"" + iconPath + "\" />";
    msg += "</td><td>";
    msg += "<h1>" + reply->errorString() + "</h1>";
    msg += "<h2>" + i18nc("%1=an URL, e.g.'kde.org'", "When connecting to: <b>%1</b>", urlString) + "</h2>";
    msg += "</td></tr></table>";

    msg += "<ul><li>" + i18n("Check the address for errors such as <b>ww</b>.kde.org instead of <b>www</b>.kde.org");
    msg += "</li><li>" + i18n("If the address is correct, try to check the network connection.") + "</li><li>" ;
    msg += i18n("If your computer or network is protected by a firewall or proxy, make sure that rekonq is permitted to access the network.");
    msg += "</li><li>" + i18n("Of course, if rekonq does not work properly, you can always say it is a programmer error ;)");
    msg += "</li></ul><br/><br/>";
    msg += "<input type=\"button\" id=\"reloadButton\" onClick=\"document.location.href='" + urlString + "';\" value=\"";
    msg += i18n("Try Again") + "\" />";

    QString html = QString(QL1S(file.readAll()))
                   .arg(title)
                   .arg(msg)
                   ;
    return html;
}


void WebPage::downloadReply(const QNetworkReply *reply, const QString &suggestedFileName)
{
    downloadResource(reply->url(), KIO::MetaData(), view(), suggestedFileName);
}


void WebPage::downloadRequest(const QNetworkRequest &request)
{
    downloadResource(request.url(),
                     request.attribute(static_cast<QNetworkRequest::Attribute>(KIO::AccessManager::MetaData)).toMap(),
                     view());
}


void WebPage::downloadUrl(const KUrl &url)
{
    downloadResource(url, KIO::MetaData(), view());
}


void WebPage::downloadAllContentsWithKGet(QPoint)
{
    QSet<QString> contents;
    KUrl baseUrl(currentFrame()->url());
    KUrl relativeUrl;

    QWebElementCollection images = mainFrame()->documentElement().findAll("img");
    foreach(const QWebElement &img, images)
    {
        relativeUrl.setEncodedUrl(img.attribute("src").toUtf8(), KUrl::TolerantMode);
        contents << baseUrl.resolved(relativeUrl).toString();
    }

    QWebElementCollection links = mainFrame()->documentElement().findAll("a");
    foreach(const QWebElement &link, links)
    {
        relativeUrl.setEncodedUrl(link.attribute("href").toUtf8(), KUrl::TolerantMode);
        contents << baseUrl.resolved(relativeUrl).toString();
    }

    if (!QDBusConnection::sessionBus().interface()->isServiceRegistered("org.kde.kget"))
    {
        KToolInvocation::kdeinitExecWait("kget");
    }
    QDBusInterface kget("org.kde.kget", "/KGet", "org.kde.kget.main");
    if (kget.isValid())
    {
        kget.call("importLinks", QVariant(contents.toList()));
    }
}


void WebPage::showSSLInfo(QPoint)
{
    if (_sslInfo.isValid())
    {
        QPointer<KSslInfoDialog> dlg = new KSslInfoDialog(view());
        dlg->setSslInfo(_sslInfo.certificateChain(),
                        _sslInfo.peerAddress().toString(),
                        mainFrame()->url().host(),
                        _sslInfo.protocol(),
                        _sslInfo.ciphers(),
                        _sslInfo.usedChiperBits(),
                        _sslInfo.supportedChiperBits(),
                        KSslInfoDialog::errorsFromString(_sslInfo.certificateErrors())
                       );

        dlg->exec();
        delete dlg;

        return;
    }

    if (mainFrame()->url().scheme() == QL1S("https"))
    {
        KMessageBox::error(view(),
                           i18n("The SSL information for this site appears to be corrupt."),
                           i18nc("Secure Sockets Layer", "SSL")
                          );
    }
    else
    {
        KMessageBox::information(view(),
                                 i18n("This site does not contain SSL information."),
                                 i18nc("Secure Sockets Layer", "SSL")
                                );
    }
}


void WebPage::updateImage(bool ok)
{
    if (ok)
    {
        NewTabPage p(mainFrame());
        p.snapFinished();
    }
}


void WebPage::copyToTempFileResult(KJob* job)
{
    if (job->error())
        job->uiDelegate()->showErrorMessage();
    else
        (void)KRun::runUrl(static_cast<KIO::FileCopyJob *>(job)->destUrl(), _mimeType, rApp->mainWindow());
}
