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
#include "downloadmanager.h"
#include "historymanager.h"
#include "iconmanager.h"

#include "networkaccessmanager.h"
#include "webpluginfactory.h"
#include "websnap.h"
#include "webtab.h"
#include "sslinfodialog.h"

#include "searchengine.h"
#include "webwindow.h"

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
#include <KMimeType>
#include <KProtocolManager>

#include <KIO/Job>
#include <KIO/JobUiDelegate>

#include <kparts/browseropenorsavequestion.h>

#include <solid/networking.h>

// Qt Includes
#include <QTextDocument>
#include <QFileInfo>
#include <QNetworkReply>


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


WebPage::WebPage(bool isPrivateBrowsing, QWidget *parent)
    : KWebPage(parent, KWalletIntegration)
    , _networkAnalyzer(false)
    , _isOnRekonqPage(false)
{
    // handling unsupported content...
    setForwardUnsupportedContent(true);
    connect(this, SIGNAL(unsupportedContent(QNetworkReply*)), this, SLOT(handleUnsupportedContent(QNetworkReply*)));

    if (isPrivateBrowsing)
    {
        // NOTE:
        // I'm sorry I really cannot let KIO work as needed in private browsing mode.
        // The problem is basically cookie related. This way we lose some features in private
        // browsing mode, but we ensure PRIVACY! This change cannot be reverted until a proper
        // fix for KIO (or the right workaround for rekonq) will be found.
        QNetworkAccessManager *manager = NetworkAccessManager::privateAccessManager();
        setNetworkAccessManager(manager);

        // ----- last stuffs
        connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(manageNetworkErrors(QNetworkReply*)));

        settings()->setAttribute(QWebSettings::PrivateBrowsingEnabled, true);
    }
    else
    {
        // rekonq Network Manager
        NetworkAccessManager *manager = new NetworkAccessManager(this);

        // set network reply object to emit readyRead when it receives meta data
        manager->setEmitReadyReadOnMetaDataChange(true);

        // disable QtWebKit cache to just use KIO one..
        manager->setCache(0);

        setNetworkAccessManager(manager);

        // activate ssl warnings
        setSessionMetaData(QL1S("ssl_activate_warnings"), QL1S("TRUE"));

        // ----- last stuffs
        connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(manageNetworkErrors(QNetworkReply*)));
    }
    
    // ----- Web Plugin Factory
    setPluginFactory(new WebPluginFactory(this));

    connect(this, SIGNAL(downloadRequested(QNetworkRequest)), this, SLOT(downloadRequest(QNetworkRequest)));
    connect(this, SIGNAL(loadStarted()), this, SLOT(loadStarted()));
    connect(this, SIGNAL(loadFinished(bool)), this, SLOT(loadFinished(bool)));

    connect(this, SIGNAL(frameCreated(QWebFrame*)), AdBlockManager::self(), SLOT(applyHidingRules(QWebFrame*)));
    
    // protocol handler signals
    connect(&_protHandler, SIGNAL(downloadUrl(KUrl)), this, SLOT(downloadUrl(KUrl)));
}


WebPage::~WebPage()
{
    disconnect();

    QPixmap preview = WebSnap::renderPagePreview(*this);
    QString path = WebSnap::imagePathFromUrl(mainFrame()->url().toString());
    QFile::remove(path);
    preview.save(path);
}


void WebPage::setWindow(QWidget *w)
{
    if (!settings()->testAttribute(QWebSettings::PrivateBrowsingEnabled))
    {
        // set cookieJar window..
        NetworkAccessManager *manager = qobject_cast<NetworkAccessManager *>(networkAccessManager());
        manager->setWindow(w);
    }
    
    _protHandler.setWindow(w);
}


bool WebPage::isOnRekonqPage() const
{
    return _isOnRekonqPage;
};


void WebPage::setIsOnRekonqPage(bool b)
{
    _isOnRekonqPage = b;
};


KUrl WebPage::loadingUrl()
{
    return _loadingUrl;
};


QString WebPage::suggestedFileName()
{
    return _suggestedFileName;
};


bool WebPage::acceptNavigationRequest(QWebFrame *frame, const QNetworkRequest &request, NavigationType type)
{
    if (_isOnRekonqPage)
    {
        WebView *view = qobject_cast<WebView *>(parent());
        WebTab *tab = view->parentTab();
        _isOnRekonqPage = false;
        tab->setPart(0, KUrl());     // re-enable the view page
    }

    const bool isMainFrameRequest = (frame == mainFrame());
    if (isMainFrameRequest)
    {
        // reset webpage values
        _suggestedFileName.clear();
        _loadingUrl = request.url();

        // Set Page URL: This is needed for 2 reasons.
        // 1) WebKit is slot setting url in some case. Having an initial URL set seems snappier ;)
        // 2) When WebKit cannot set URL (eg: network down), urlbar URL is NOT set
        emit initialUrl(_loadingUrl);        
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
            setRequestMetaData(QL1S("cache"), QL1S("reload"));
            break;

        case QWebPage::NavigationTypeBackOrForward:
        case QWebPage::NavigationTypeOther:
            break;

        default:
            ASSERT_NOT_REACHED(unknown NavigationType);
            break;
        }
    }

    // Make sure nothing is cached when private browsing mode is enabled...
    if (!settings()->testAttribute(QWebSettings::PrivateBrowsingEnabled))
    {
        // Get the SSL information sent, if any...
        NetworkAccessManager *manager = qobject_cast<NetworkAccessManager *>(networkAccessManager());
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
    }

    return KWebPage::acceptNavigationRequest(frame, request, type);
}


WebPage *WebPage::createWindow(QWebPage::WebWindowType type)
{
    // added to manage web modal dialogs
    if (type == QWebPage::WebModalDialog)
        kDebug() << "Modal Dialog";

    bool isPrivateBrowsing = settings()->testAttribute(QWebSettings::PrivateBrowsingEnabled);

    // private page links open private pages. See BUG: 320218
    WebPage* p = new WebPage(isPrivateBrowsing);
    emit pageCreated(p);
    return p;
}


// From Konqueror's konqsettings.cpp
static bool alwaysEmbedMimeTypeGroup(const QString &mimeType)
{
    if (mimeType.startsWith("inode") || mimeType.startsWith("Browser"))
        return true;
    return false;
}


// From Konqueror's konqsettings.cpp.
// config and m_embedMap were manually added to make the function static.
// TODO: Probably should be moved into some library for KDE Frameworks...
static bool shouldEmbed(const QString &_mimeType)
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig("filetypesrc", KConfig::NoGlobals);
    QMap<QString, QString> m_embedMap = config->entryMap("EmbedSettings");

    KMimeType::Ptr mime = KMimeType::mimeType(_mimeType, KMimeType::ResolveAliases);
    if (!mime) {
        kWarning() << "Unknown mimetype" << _mimeType;
        return false; // unknown mimetype!
    }
    const QString mimeType = mime->name();

    // First check in user's settings whether to embed or not
    // 1 - in the filetypesrc config file (written by the configuration module)
    QMap<QString, QString>::const_iterator it = m_embedMap.constFind( QString::fromLatin1("embed-")+mimeType );
    if ( it != m_embedMap.constEnd() ) {
        kDebug() << mimeType << it.value();
        return it.value() == QLatin1String("true");
    }
    // 2 - in the configuration for the group if nothing was found in the mimetype
    if (alwaysEmbedMimeTypeGroup(mimeType))
        return true; //always embed mimetype inode/*, Browser/* and Konqueror/*
    const QString mimeTypeGroup = mimeType.left(mimeType.indexOf('/'));
    it = m_embedMap.constFind( QString::fromLatin1("embed-")+mimeTypeGroup );
    if ( it != m_embedMap.constEnd() ) {
        kDebug() << mimeType << "group setting:" << it.value();
        return it.value() == QLatin1String("true");
    }
    // 2 bis - configuration for group of parent mimetype, if different
    if (mimeType[0].isLower()) {
        QStringList parents;
        parents.append(mimeType);
        while (!parents.isEmpty()) {
            const QString parent = parents.takeFirst();
            if (alwaysEmbedMimeTypeGroup(parent)) {
                return true;
            }
            KMimeType::Ptr mime = KMimeType::mimeType(parent);
            Q_ASSERT(mime); // how could the -parent- be null?
            if (mime)
                parents += mime->parentMimeTypes();
        }
    }

    // 3 - if no config found, use default.
    // Note: if you change those defaults, also change keditfiletype/mimetypedata.cpp !
    // Embedding is false by default except for image/* and for zip, tar etc.
    const bool hasLocalProtocolRedirect = !KProtocolManager::protocolForArchiveMimetype(mimeType).isEmpty();
    if (mimeTypeGroup == "image" || mimeTypeGroup == "multipart" || hasLocalProtocolRedirect)
        return true;
    return false;
}


void WebPage::handleUnsupportedContent(QNetworkReply *reply)
{
    Q_ASSERT(reply);

    if (reply->error() != QNetworkReply::NoError)
        return;

    KIO::Integration::AccessManager::putReplyOnHold(reply);

    // handle protocols WebKit cannot handle...
    if (_protHandler.postHandling(reply->request(), mainFrame()))
    {
        return;
    }

    // Get suggested file name...
    const KIO::MetaData& data = reply->attribute(static_cast<QNetworkRequest::Attribute>(KIO::AccessManager::MetaData)).toMap();
    _suggestedFileName = data.value(QL1S("content-disposition-filename"));

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
        if (isLocal)
        {
            KMessageBox::sorry(view(), i18n("No service can handle this file."));
            return;
        }

        DownloadManager::self()->downloadResource(reply->url(),
                KIO::MetaData(),
                view(),
                false,
                _suggestedFileName,
                !settings()->testAttribute(QWebSettings::PrivateBrowsingEnabled));
        return;
    }

    // NOTE
    // This is needed in case rekonq has been associated with something it cannot
    // properly handle (eg: xbel files, see BUG:299056). This way we break an eventual
    // "recall" loop.
    if (appService->exec().trimmed().startsWith(QL1S("rekonq")))
    {
        if (isLocal)
        {
            KMessageBox::sorry(view(), i18n("rekonq cannot properly handle this, sorry"));
            return;
        }

        DownloadManager::self()->downloadResource(reply->url(),
                KIO::MetaData(),
                view(),
                false,
                _suggestedFileName,
                !settings()->testAttribute(QWebSettings::PrivateBrowsingEnabled));
        return;
    }
    
    if (isLocal)
    {
        // Load outside local files
        KRun::run(*appService, replyUrl, 0, false, _suggestedFileName);
        return;
    }

    // else
    KParts::BrowserOpenOrSaveQuestion dlg(view(), replyUrl, _mimeType);

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
        DownloadManager::self()->downloadResource(reply->url(),
                KIO::MetaData(),
                view(),
                !hideDialog,
                _suggestedFileName,
                !settings()->testAttribute(QWebSettings::PrivateBrowsingEnabled));
        return;

    case KParts::BrowserOpenOrSaveQuestion::Cancel:
        return;

    default: // Can happen when "Open with.." is set and "don't ask again" is checked
        break;
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
        job->ui()->setWindow(view());
        connect(job, SIGNAL(result(KJob*)), this, SLOT(copyToTempFileResult(KJob*)));
        return;
    }

    bool webAppMode;
    WebView *v = qobject_cast<WebView *>(view());
    WebWindow *webwin = v->parentTab()->webWindow();
    if (webwin)
        webAppMode = false;
    else
        webAppMode = true;

    // case KParts::BrowserRun::Embed
    KParts::ReadOnlyPart *pa = 0;
    if (shouldEmbed(_mimeType)) // Only create the KPart if we are actually supposed to embed.
        pa = KMimeTypeTrader::createPartInstanceFromQuery<KParts::ReadOnlyPart>(_mimeType, view(), this, QString());
    if (pa && !webAppMode)
    {
        _isOnRekonqPage = true;

        WebView *view = qobject_cast<WebView *>(parent());
        WebTab *tab = view->parentTab();
        tab->setPart(pa, replyUrl);

        // WARNING: Is this enough?
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
    // set zoom factor
    QString val;
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group(config, "Zoom");
    val = group.readEntry(_loadingUrl.host(), QString("10"));

    int value = val.toInt();
    if (value != 10)
        mainFrame()->setZoomFactor(QVariant(value).toReal() / 10);  // Don't allox max +1 values    
}


void WebPage::loadFinished(bool ok)
{
    Q_UNUSED(ok);

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
        _sslInfo.restoreFrom(reply->attribute(static_cast<QNetworkRequest::Attribute>(KIO::AccessManager::MetaData)), reply->url());
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
        // last chance for the strange things (eg: FTP, custom schemes, etc...)
        if (_protHandler.postHandling(reply->request(), mainFrame()))
            return;

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
            frame->setHtml(errorPage(reply), reply->url());
            if (isMainFrameRequest)
            {
                _isOnRekonqPage = true;
                // WARNING: is this enough?
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
        return QString("Couldn't open the rekonqinfo.html file! This probably means you installed rekonq in a bad way.");
    }

    // NOTE:
    // this, to take care about XSS (see BUG 217464)...
    QString urlString = Qt::escape(reply->url().toString());

    // 1. data path
    QString dataPath = QL1S("file://") + notfoundFilePath;
    dataPath.remove(QL1S("/htmls/rekonqinfo.html"));

    // 2. title
    QString title = i18n("There was a problem while loading the page");

    QString msg;

    // test to see if networking is enabled on the system
    if (Solid::Networking::status() != Solid::Networking::Connected)
    {
        msg += QL1S("<h2>") + i18n("Network is not available") + QL1S("</h2>");

        QString faceIconPath = QString("file://") + KIconLoader::global()->iconPath("face-surprise" , -KIconLoader::SizeHuge, false);
        msg += QL1S("<table>");
        msg += QL1S("<tr><td width=\"100px\">");
        msg += QL1S("<img style=\"margin: 0 auto;\" src=\"") + faceIconPath + QL1S("\" />");
        msg += QL1S("</td><td>");

        msg += QL1S("<p>");

        msg += i18n("Maybe you are having problems with your network settings.<br />Try checking your <a href=\"%1\">network connections</a>, your <a href=\"%2\">proxy settings</a> and your <a href=\"%3\">firewall</a>.<br /><br />Then <a href=\"%4\">try again</a>.<br />", QL1S("rekonq:settings/network"), QL1S("rekonq:settings/proxy"), QL1S("rekonq:settings/firewall"), urlString);

        msg += QL1S("</p>");

        msg += QL1S("</td></tr></table>");

        // done. Replace variables and show it
        QString html = QL1S(file.readAll());

        html.replace(QL1S("$DEFAULT_PATH"), dataPath);
        html.replace(QL1S("$PAGE_TITLE"), title);
        html.replace(QL1S("$MAIN_CONTENT"), msg);
        html.replace(QL1S("$GENERAL_FONT"), QWebSettings::globalSettings()->fontFamily(QWebSettings::StandardFont));
        
        return html;
    }

    QString errString = reply->errorString().toLower();
    if (errString.contains(QL1S("proxy")))
    {
        msg += QL1S("<h2>") + i18n("Oops... Proxy problems!") + QL1S("</h2>");

        QString faceIconPath = QString("file://") + KIconLoader::global()->iconPath("face-surprise" , -KIconLoader::SizeHuge, false);
        msg += QL1S("<table>");
        msg += QL1S("<tr><td width=\"100px\">");
        msg += QL1S("<img style=\"margin: 0 auto;\" src=\"") + faceIconPath + QL1S("\" />");
        msg += QL1S("</td><td>");

        msg += QL1S("<p><em>") + reply->errorString() + QL1S("</em></p>");
        
        msg += QL1S("<p>");

        msg += i18n("It seems you are having problems with your <a href=\"%1\">proxy settings</a>. Try checking them, <br /><br />then <a href=\"%2\">try again</a>.<br />", QL1S("rekonq:settings/proxy"), urlString);

        msg += QL1S("</p>");

        msg += QL1S("</td></tr></table>");

        // done. Replace variables and show it
        QString html = QL1S(file.readAll());

        html.replace(QL1S("$DEFAULT_PATH"), dataPath);
        html.replace(QL1S("$PAGE_TITLE"), title);
        html.replace(QL1S("$MAIN_CONTENT"), msg);
        html.replace(QL1S("$GENERAL_FONT"), QWebSettings::globalSettings()->fontFamily(QWebSettings::StandardFont));
        
        return html;
    }
    
    // general error page
    msg += QL1S("<h2>") + i18n("Oops! Cannot load <em>%1</em>", urlString) + QL1S("</h1>");

    QString faceIconPath = QString("file://") + KIconLoader::global()->iconPath("face-surprise" , -KIconLoader::SizeHuge, false);
    msg += QL1S("<table>");
    msg += QL1S("<tr><td width=\"100px\">");
    msg += QL1S("<img src=\"") + faceIconPath + QL1S("\" />");
    msg += QL1S("</td><td>");

    msg += QL1S("<p><em>") + reply->errorString() + QL1S("</em></p>");

    // Default SearchEngine
    KService::Ptr defaultEngine = SearchEngine::defaultEngine();

    msg += QL1S("<p>");
    if (defaultEngine)
    {
        msg += i18n("Ask your default search engine about: "
                    "<a href=\"%1\">search with %2</a>.<br />",
                    SearchEngine::buildQuery(defaultEngine, urlString),
                    defaultEngine->name());
    }
    else
    {
        msg += i18n("You do not have a default search engine set. We will not suggest you one.");
    }
    msg += QL1S("</p>");

    msg += QL1S("<p>");
    msg += i18n("Consult a cached snapshot of the site: "
                "try checking the <a href=\"%1\">Wayback Machine</a>"
                " or the <a href=\"%2\">Google Cache</a>.",
                QL1S("http://wayback.archive.org/web/*/") + urlString,
                QL1S("http://google.com/search?q=cache:") + urlString);
    msg += QL1S("</p>");
    msg += QL1S("<h5>") + i18n("<a href='%1'>Try Again</a>", urlString) + QL1S("</h5>");
    msg += QL1S("</td></tr></table>");

    // done. Replace variables and show it
    QString html = QL1S(file.readAll());

    html.replace(QL1S("$DEFAULT_PATH"), dataPath);
    html.replace(QL1S("$PAGE_TITLE"), title);
    html.replace(QL1S("$MAIN_CONTENT"), msg);
    html.replace(QL1S("$GENERAL_FONT"), QWebSettings::globalSettings()->fontFamily(QWebSettings::StandardFont));
        
    return html;
}


void WebPage::downloadRequest(const QNetworkRequest &request)
{
    DownloadManager::self()->downloadResource(request.url(),
            request.attribute(static_cast<QNetworkRequest::Attribute>(KIO::AccessManager::MetaData)).toMap(),
            view(),
            false,
            _suggestedFileName,
            !settings()->testAttribute(QWebSettings::PrivateBrowsingEnabled));
}


void WebPage::downloadUrl(const KUrl &url)
{
    DownloadManager::self()->downloadResource(url,
            KIO::MetaData(),
            view(),
            false,
            QString(),
            !settings()->testAttribute(QWebSettings::PrivateBrowsingEnabled));
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

    DownloadManager::self()->downloadLinksWithKGet(QVariant(contents.toList()));
}


void WebPage::copyToTempFileResult(KJob* job)
{
    if (job->error())
        job->uiDelegate()->showErrorMessage();
    else
        (void)KRun::runUrl(static_cast<KIO::FileCopyJob *>(job)->destUrl(), _mimeType, view());
}


bool WebPage::hasSslValid() const
{
    QList<QSslCertificate> certList = _sslInfo.certificateChain();

    if (certList.isEmpty())
        return false;

    const QSslCertificate cert = certList.at(0);
    if (!cert.isValid())
        return false;

    QList<QStringList> errorList = SslInfoDialog::errorsFromString(_sslInfo.certificateErrors());
    if (!errorList.isEmpty())
    {
        QStringList list = errorList.at(0);
        if (!list.isEmpty())
            return false;
    }

    return true;
}


WebSslInfo WebPage::sslInfo()
{
    return _sslInfo;
}
