/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2012 by Siteshwar Vashisht <siteshwar at gmail dot com>
* Copyright (C) 2011 by Andrea Diamantini <adjam7 at gmail dot com>
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
#include "operasynchandler.h"
#include "operasynchandler.moc"

// Auto Includes
#include "rekonq.h"

// Local Includes
#include "application.h"
#include "bookmarkmanager.h"

// KDE Includes
#include <KStandardDirs>
#include <klocalizedstring.h>
#include <kbookmarkmanager.h>


#include <QList>
#include <QWebPage>
#include <QWebFrame>
#include <QWebElement>
#include <QUrl>
#include <QWebSettings>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QDomDocument>


OperaSyncHandler::OperaSyncHandler(QObject *parent)
    : SyncHandler(parent)
    , _mode(RECEIVE_CHANGES)
    , _doLogin(false)
    , _requestCount(0)
    , _isSyncing(false)
{
    kDebug() << "Creating Opera Bookmarks handler...";
    _webPage.settings()->setAttribute(QWebSettings::AutoLoadImages, false);
    _webPage.settings()->setAttribute(QWebSettings::PrivateBrowsingEnabled, true);
    connect(&_webPage, SIGNAL(loadFinished(bool)), this, SLOT(loadFinished(bool)));

    _qoauth.setConsumerKey("zCuj9aUcehaHsfKtcHcg2YYLX42CkxDX");
    _qoauth.setConsumerSecret("xApuyHdDd9DSbTXLDRXuZzwKI2lOYSsl");
}


void OperaSyncHandler::initialLoadAndCheck()
{
    if (!ReKonfig::syncEnabled())
    {
        _firstTimeSynced = false;
        return;
    }

    // Bookmarks
    if (ReKonfig::syncBookmarks())
    {
        _mode = RECEIVE_CHANGES;
        startLogin();
    }

    if (ReKonfig::syncHistory())
    {
        emit syncStatus(Rekonq::History, false, i18n("Not supported!"));
    }

    if (ReKonfig::syncHistory())
    {
        emit syncStatus(Rekonq::Passwords, false, i18n("Not supported!"));
    }
}


bool OperaSyncHandler::syncRelativeEnabled(bool check)
{
    if (!ReKonfig::syncEnabled())
        return false;

    if (!_firstTimeSynced)
        return false;

    return check;
}


// ---------------------------------------------------------------------------------------


void OperaSyncHandler::syncHistory()
{
    kDebug() << "Syncing history not supported!";
    emit syncStatus(Rekonq::History, false, i18n("Syncing history not supported!"));
    emit syncHistoryFinished(false);
}


void OperaSyncHandler::syncPasswords()
{
    kDebug() << "Syncing passwords not supported!";
    emit syncStatus(Rekonq::Passwords, false, i18n("Syncing passwords not supported!"));
    emit syncPasswordsFinished(false);
}


void OperaSyncHandler::syncBookmarks()
{

    if (_isSyncing)
    {
        kDebug() << "Sync already in progress!";
        return;
    }
    _mode = SEND_CHANGES;
    startLogin();
}

void OperaSyncHandler::startLogin()
{
    if (ReKonfig::syncUser().isEmpty() || ReKonfig::syncPass().isEmpty())
    {
        kDebug() << "No username or password!";
        emit syncStatus(Rekonq::Bookmarks, false, i18n("No username or password!"));
        emit syncBookmarksFinished(false);
        return;
    }

    _isSyncing = true;

    _doLogin = true;

    kDebug() << "Loading login page...";

    _qoauth.setRequestTimeout(20000);
    _qoauth.ignoreSslErrors();
    QOAuth::ParamMap requestMap;
    requestMap.insert("oauth_callback", "oob");

    QOAuth::ParamMap requestParam = _qoauth.requestToken("https://auth.opera.com/service/oauth/request_token", QOAuth::POST, QOAuth::HMAC_SHA1, requestMap);
    qDebug() << _qoauth.error();
    qDebug() << requestParam;

    _requestToken = requestParam.value("oauth_token");
    _requestTokenSecret = requestParam.value("oauth_token_secret");

    qDebug() << _requestToken;
    qDebug() << _requestTokenSecret;


    qDebug() << QUrl("https://auth.opera.com/service/oauth/authorize?oauth_token=" + QString(_requestToken) + "&oauth_callback=oob");

    _webPage.mainFrame()->load(QUrl("https://auth.opera.com/service/oauth/authorize?oauth_token=" + QString(_requestToken) + "&oauth_callback=oob" ));

}

//Loading a webpage finished, what action to take is decided based on url we have loaded.
void OperaSyncHandler::loadFinished(bool ok)
{
    kDebug() << "Load Finished" << ok;
    if (!ok)
    {
        kDebug() << "Error loading: " << _webPage.mainFrame()->url();
        emit syncStatus(Rekonq::Bookmarks, false, i18n( "Error loading: " + _webPage.mainFrame()->url().toEncoded()));

        _isSyncing = false;
        return;
    }

    QString path = _webPage.mainFrame()->url().path();

    qDebug() << path;
    if (path == "/service/oauth/authorize")
    {
        QWebFrame *mainFrame = _webPage.mainFrame();
        QString html = mainFrame->toHtml();

        if (_doLogin == true && html.contains("login-form"))
        {
            qDebug() << "Login page";
            QWebElement form = mainFrame->findFirstElement("#login-form");
            if (form.isNull())
            {
                qDebug() << "form is null";
            }

            QWebElement username = form.findFirst("#username");
            QWebElement password = form.findFirst("#password");
            QWebElement button = form.findFirst("input[name=\"grant\"]");

            qDebug() << username.isNull();
            qDebug() << password.isNull();
            qDebug() << button.isNull();
            username.setAttribute("value", ReKonfig::syncUser());
            password.setAttribute("value", ReKonfig::syncPass());

            qDebug() << button.toPlainText();
            button.evaluateJavaScript("this.click();");

            emit syncStatus(Rekonq::Bookmarks, true, i18n("Signing in..."));
            _doLogin = false;
        }
        else if (html.contains("verifier"))
        {
            QOAuth::ParamMap authParams;
            QWebElement authkey = mainFrame->findFirstElement("#verifier");
            QByteArray verifier = authkey.toPlainText().toUtf8();

            qDebug() << verifier;
            authParams.insert("oauth_verifier", verifier);
            QOAuth::ParamMap resultParam = _qoauth.accessToken("https://auth.opera.com/service/oauth/access_token", QOAuth::POST, _requestToken, _requestTokenSecret, QOAuth::HMAC_SHA1, authParams);

            qDebug() << _qoauth.error();
            qDebug() << resultParam;

            _authToken = resultParam.value("oauth_token");
            _authTokenSecret = resultParam.value("oauth_token_secret");

            _requestToken.clear();
            _requestTokenSecret.clear();

            getBookmarks();
        }
        else if (_doLogin == false)
        {
            //Login failed
            emit syncStatus(Rekonq::Bookmarks, false, i18n("Login failed!"));
            _isSyncing = false;
        }
    }
}

//Fetch all the bookmarks from server
void OperaSyncHandler::getBookmarks()
{
    emit syncStatus(Rekonq::Bookmarks, true, i18n("Fetching bookmarks from server..."));

    QOAuth::ParamMap requestMap;

    requestMap.insert("api_output", "xml");
    qDebug() << "Auth Token : " << _authToken;
    qDebug() << "Auth Token Secret : " << _authTokenSecret;



    QByteArray fetchBookmarksUrl = "https://link.api.opera.com/rest/bookmark/descendants/";
    QByteArray urlParams = _qoauth.createParametersString(fetchBookmarksUrl, QOAuth::GET, _authToken, _authTokenSecret, QOAuth::HMAC_SHA1, requestMap, QOAuth::ParseForInlineQuery);

    QNetworkRequest request;

    qDebug() << "URL Params: " << urlParams;


    fetchBookmarksUrl.append(urlParams);
    //kDebug() << urlstr;
    KIO::TransferJob *job = KIO::get(KUrl(fetchBookmarksUrl), KIO::Reload, KIO::HideProgressInfo);

    connect(job, SIGNAL(result(KJob*)), this, SLOT(fetchBookmarksResultSlot(KJob*)));
    connect(job, SIGNAL(data(KIO::Job*, QByteArray)), this, SLOT(fetchBookmarksDataSlot(KIO::Job*, QByteArray)));
}

void OperaSyncHandler::fetchBookmarksDataSlot(KIO::Job* job, QByteArray data)
{
    kDebug() << data;
    _xmlData += data;
}

//We have received all the bookmarks which exist on server, now we need to compare them with local bookmarks.
void OperaSyncHandler::fetchBookmarksResultSlot(KJob* job)
{
    if (job->error() != 0)
    {
        _xmlData = "";
        kDebug() << "Some error!";
        return;
    }

    kDebug() << "No Error!";
    kDebug() << _xmlData;

    QDomDocument doc("bookmarks");
    doc.setContent(_xmlData);
    QDomNodeList responseList = doc.elementsByTagName("response");

    qDebug() << responseList.size();
    KBookmarkGroup root = rApp->bookmarkManager()->rootGroup();

    if (_mode == RECEIVE_CHANGES)
    {
        handleResponse(responseList, root);
        emit syncStatus(Rekonq::Bookmarks, true, i18n("Done!"));
//        _isSyncing = false;
        _mode = SEND_CHANGES;
    }

    handleResponse(responseList, root);

    QDomElement item = responseList.at(0).toElement();
    KBookmark current = root.first();

    while(!current.isNull())
    {
        if (current.isGroup())
        {
            QString groupName = current.fullText();
            QDomElement child = findOperaFolder(item, groupName);

            if (child.isNull())
            {
                //Add Opera group here
                kDebug() << "Add group " << groupName;
                KJob *job = addBookmarkFolderOnServer(current.fullText());
                _jobToGroupMap.insert(job, current.toGroup());
            }
            else
            {
                kDebug() << "Handling group " << groupName;
                QDomElement grandChild = getChildElement(child, "children");

                QString id = getChildString(child, "id");

                kDebug() << grandChild.tagName() << id;

                if (grandChild.isNull())
                {
                    kDebug() << "Grand child is null";
                    handleLocalGroup(current.toGroup(), grandChild, id);
                }
                else{
                    //kDebug() << "Grand child " << getTitleFromResourceProperties(grandChild);
                    handleLocalGroup(current.toGroup(), grandChild, id);
                }
            }
        }

        else
        {
            KUrl url = current.url();

            QDomElement child = findOperaBookmark(item, url);

            if (child.isNull())
            {
                //Add bookmark on server
                kDebug() << "Add bookmark :" << url;
                addBookmarkOnServer(current.fullText(), current.url().url());
            }
            else
            {
                kDebug() << "Bookmark exists :" << url;
            }
        }

        current = root.next(current);
    }

    decreaseRequestCount();


    _xmlData = "";
}

void OperaSyncHandler::createBookmarkDataSlot(KIO::Job* job, QByteArray data)
{
    kDebug() << data;
    //_xmlData += data;
}

//Check whether bookmark creation was succesful on server
void OperaSyncHandler::createBookmarkResultSlot(KJob* job)
{
    decreaseRequestCount();

    if (job->error() != 0)
    {
        kDebug() << "Some error!" << job->error();
        return;
    }
}

void OperaSyncHandler::createBookmarkFolderDataSlot(KIO::Job* job, QByteArray data)
{
    kDebug() << data;

    QByteArray &value = _jobToResponseMap[job];
    value.append(data);
}

//If bookmark folder (it's empty) was creted succesfully on server, we need to add all it's children (which exists in local bookmarks) on server.
void OperaSyncHandler::createBookmarkFolderResultSlot(KJob* job)
{
    decreaseRequestCount();

    QByteArray value = _jobToResponseMap[job];
    KBookmarkGroup root = _jobToGroupMap[job];

    _jobToResponseMap.remove(job);
    _jobToGroupMap.remove(job);

    if (job->error() != 0)
    {
        kDebug() << "Some error!";
        return;
    }

    QDomDocument doc("new bookmark");
    doc.setContent(value);
    QDomNodeList responseList = doc.elementsByTagName("response");

    qDebug() << responseList.size();

    if (responseList.size() > 0)
    {
        QDomElement item = responseList.at(0).firstChildElement();

        QString parentId = getIdFromResource(item);
        kDebug() << "Resource id is : " << parentId;

        handleLocalGroup(root, item, parentId);

    }
}

void OperaSyncHandler::deleteResourceResultSlot(KJob* job)
{
    decreaseRequestCount();

    if (job->error() != 160)
    {
        kDebug() << "Some error!" << job->error();
        return;
    }
}

void OperaSyncHandler::deleteResourceDataSlot(KIO::Job* job, QByteArray data)
{
    kDebug() << data;
}

//Checks whether a bookmark exists locally or not, and either add it locally or delete from server
void OperaSyncHandler::handleBookmark(const QDomElement &item, KBookmarkGroup root)
{
    QString url = getUrlFromResourceProperties(item);
    QString title = getTitleFromResourceProperties(item);
    QString id = getChildString(item, "id");

    KBookmark bookmark = findLocalBookmark(root, KUrl(url));

    if (bookmark.isNull())
    {
        if (_mode == RECEIVE_CHANGES)
        {
            root.addBookmark(title, KUrl(url));
            rApp->bookmarkManager()->manager()->emitChanged(root);
        }
        else
        {
            //Delete bookmark from server
            kDebug() << "Deleting bookmark from server : " << title;
            deleteResourceOnServer(id);
        }
    }

}

//Traverse through all the children of a bookmark folder
void OperaSyncHandler::handleBookmarkFolder(const QDomElement &item, KBookmarkGroup &root)
{
    qDebug() << "Title : " << getTitleFromResourceProperties(item);

    QDomNode child = item.firstChild();

    while (!child.isNull())
    {
        QDomNode resource = child.firstChild();

        while (!resource.isNull())
        {
            handleResource(resource, root);
            resource = resource.nextSibling();
        }
        child = child.nextSibling();
    }
}

//Handle resource tag of xml replied from server
void OperaSyncHandler::handleResource(const QDomNode &item, KBookmarkGroup &root)
{
    //        qDebug() << item.nodeName();
    QDomElement element = item.toElement();

    QString itemType = getChildString(item, "item_type");
    if (itemType == "bookmark")
    {
        handleBookmark(element, root);
    }
    else if (itemType == "bookmark_folder")
    {
        QString title = getTitleFromResourceProperties(item.toElement());
        QString id = getChildString(item.toElement(), "id");
        if (title == "Trash") return;


        KBookmarkGroup childGroup = findLocalGroup(root, title);
        kDebug() << childGroup.isNull();

        if (_mode == RECEIVE_CHANGES)
        {
            if (childGroup.isNull())
            {
                childGroup = root.createNewFolder(title);
                rApp->bookmarkManager()->manager()->emitChanged(root);
            }

            handleBookmarkFolder(element, childGroup);
        }
        else
        {
            if (childGroup.isNull())
            {
                //Delete bookmark folder on server
                kDebug() << "Deleting bookmark folder from server : " << title;
                deleteResourceOnServer(id);
            }
            else
            {
                handleBookmarkFolder(element, childGroup);
            }
        }
    }
}

//Start traversing the response received from server from <response> tag
void OperaSyncHandler::handleResponse(const QDomNodeList &responseList, KBookmarkGroup &root)
{
    if (responseList.size() > 0)
    {
        QDomNode item = responseList.at(0).firstChild();

        //        qDebug() << item.nodeName();

        do
        {
            handleResource(item, root);
            item = item.nextSibling();
        }while (!item.isNull());
    }
}

//This method checks whether we need to add a bookmark or bookmark folder on server which exists only locally
void OperaSyncHandler::handleLocalGroup(const KBookmarkGroup &root, const QDomElement &item, QString parentId)
{
    KBookmark current = root.first();

    kDebug() << "Handling " << parentId;
    while(!current.isNull())
    {
        kDebug() << "Looping";
        if (current.isGroup())
        {
            QString groupName = current.fullText();
            QDomElement child = findOperaFolder(item, groupName);

            if (child.isNull())
            {
                //Add Opera group here
                kDebug() << "Add group " << groupName;
                kDebug() << "Parent is " << item.text();
                //QString parentId = getIdFromResource(item);
                kDebug() << "Parent id is " << parentId;
                KJob *job = addBookmarkFolderOnServer(current.fullText(), parentId);
                _jobToGroupMap.insert(job, current.toGroup());
            }
            else
            {
                kDebug() << "Handling group " << groupName;

                QDomElement grandChild = getChildElement(child, "children");

                QString id = getChildString(child, "id");

                if (grandChild.isNull())
                {
                    handleLocalGroup(current.toGroup(), grandChild, id);
                }
                else
                {
                    handleLocalGroup(current.toGroup(), grandChild, id);
                }
            }
        }
        else
        {
            KUrl url = current.url();

            QDomElement child = findOperaBookmark(item, url);

            if (child.isNull())
            {
                //Add bookmark on server
                kDebug() << "Add bookmark :" << url;
                kDebug() << "Parent is : " << item.text();
                //QString parentId = getIdFromResource(item);
                kDebug() << "Parent id is " << parentId;
                addBookmarkOnServer(current.fullText(), current.url().url(), parentId);
            }
            else
            {
                kDebug() << "Bookmark exists :" << url;
            }
        }

        current = root.next(current);
    }
}

//Add a bookmark on server
void OperaSyncHandler::addBookmarkOnServer(QString title, QString url, QString parent)
{
    QOAuth::ParamMap requestMap;
    requestMap.insert("api_output", "xml");
    requestMap.insert("api_method","create");
    requestMap.insert("item_type","bookmark");
    requestMap.insert("title", QUrl::toPercentEncoding(title.toUtf8()));
    requestMap.insert("uri", QUrl::toPercentEncoding(url.toUtf8()));

    QByteArray requestUrl = "https://link.api.opera.com/rest/bookmark/";

    if (!parent.isNull())
    {
        requestUrl.append(parent.toUtf8());
    }

    QByteArray postData = _qoauth.createParametersString(requestUrl, QOAuth::POST, _authToken, _authTokenSecret, QOAuth::HMAC_SHA1, requestMap, QOAuth::ParseForRequestContent);

    kDebug() << "Post data is : " << postData;

    kDebug() << "Request Url is : " << requestUrl;

    KIO::TransferJob *job = KIO::http_post( KUrl(requestUrl), postData, KIO::HideProgressInfo );
    job->addMetaData("Content-Type", "application/x-www-form-urlencoded" );

    connect(job, SIGNAL(result(KJob*)), this, SLOT(createBookmarkResultSlot(KJob*)));
    connect(job, SIGNAL(data(KIO::Job*, QByteArray)), this, SLOT(createBookmarkDataSlot(KIO::Job*, QByteArray)));

    ++_requestCount;
}

//Add a bookmark folder on server
KJob *OperaSyncHandler::addBookmarkFolderOnServer(QString title, QString parent)
{
    QOAuth::ParamMap requestMap;
    requestMap.insert("api_output", "xml");
    requestMap.insert("api_method","create");
    requestMap.insert("item_type","bookmark_folder");
    requestMap.insert("title", QUrl::toPercentEncoding(title.toUtf8()));

    QByteArray requestUrl = "https://link.api.opera.com/rest/bookmark/";
    if (!parent.isNull())
    {
        requestUrl.append(parent.toUtf8());
    }

    QByteArray postData = _qoauth.createParametersString(requestUrl, QOAuth::POST, _authToken, _authTokenSecret, QOAuth::HMAC_SHA1, requestMap, QOAuth::ParseForRequestContent);

    kDebug() << "Post data is : " << postData;

    kDebug() << "Request Url is : " << requestUrl;

    KIO::TransferJob *job = KIO::http_post( KUrl(requestUrl), postData, KIO::HideProgressInfo );
    job->addMetaData("Content-Type", "application/x-www-form-urlencoded" );
    _jobToResponseMap.insert(job, "");

    connect(job, SIGNAL(result(KJob*)), this, SLOT(createBookmarkFolderResultSlot(KJob*)));
    connect(job, SIGNAL(data(KIO::Job*, QByteArray)), this, SLOT(createBookmarkFolderDataSlot(KIO::Job*, QByteArray)));

    ++_requestCount;
    return job;
}

//Resource could be either a bookmark folder or bookmark.
void OperaSyncHandler::deleteResourceOnServer(QString id)
{
    QOAuth::ParamMap requestMap;
    requestMap.insert("api_method","delete");

    QByteArray requestUrl = "https://link.api.opera.com/rest/bookmark/";

    if (id.isEmpty())
    {
        qDebug() << "Id is empty!";
        return;
    }

    requestUrl.append(id.toUtf8());
    QByteArray postData = _qoauth.createParametersString(requestUrl, QOAuth::POST, _authToken, _authTokenSecret, QOAuth::HMAC_SHA1, requestMap, QOAuth::ParseForRequestContent);

    kDebug() << "Deleting Resource : " << id;
    kDebug() << "Post data is : " << postData;

    kDebug() << "Request Url is : " << requestUrl;

    KIO::TransferJob *job = KIO::http_post( KUrl(requestUrl), postData, KIO::HideProgressInfo );
    job->addMetaData("Content-Type", "application/x-www-form-urlencoded" );

    connect(job, SIGNAL(result(KJob*)), this, SLOT(deleteResourceResultSlot(KJob*)));
    connect(job, SIGNAL(data(KIO::Job*, QByteArray)), this, SLOT(deleteResourceDataSlot(KIO::Job*, QByteArray)));

    ++_requestCount;
}

//Get url for a bookmark from xml element of Opera bookmarks
QString OperaSyncHandler::getUrlFromResourceProperties(const QDomElement &item)
{
    if (item.tagName() != "resource")   return QString();
    QDomNodeList propertiesList = item.elementsByTagName("properties");

    if (propertiesList.size() > 0)
    {
        QDomElement properties = propertiesList.at(0).toElement();
        QDomNodeList uriList = properties.elementsByTagName("uri");
        if (uriList.size() > 0)
            return uriList.at(0).toElement().text();
    }

    return QString();
}

//Get title for a bookmark or folder from xml element of Opera bookmarks
QString OperaSyncHandler::getTitleFromResourceProperties(const QDomElement &item)
{
    if (item.tagName() != "resource")   return QString();

    QDomNodeList propertiesList = item.elementsByTagName("properties");

    if (propertiesList.size() > 0)
    {
        QDomElement properties = propertiesList.at(0).toElement();
        QDomNodeList titleList = properties.elementsByTagName("title");
        if (titleList.size() > 0)
            return titleList.at(0).toElement().text();
    }

    return QString();
}

//Get id for a bookmark or folder from xml element of Opera bookmarks
QString OperaSyncHandler::getIdFromResource(const QDomElement &item)
{
    if (item.tagName() != "resource")   return QString();

    QDomNodeList idList = item.elementsByTagName("id");

    if (idList.size() > 0)
    {
        return idList.at(0).toElement().text();
    }

    return QString();
}

//Get value of a child element of a dom node
QString OperaSyncHandler::getChildString(const QDomNode &node, const QString &name)
{
    QDomNodeList nodes = node.childNodes();

    for (int j=0; j<nodes.size(); ++j)
    {
        QDomElement element = nodes.at(j).toElement();

        if (nodes.at(j).nodeName() == name)
        {
            //kDebug() << "Url : " << element.text();
            return element.text();
        }
    }
    return NULL;
}

//Get value of a child element of a dom node
QDomElement OperaSyncHandler::getChildElement(const QDomNode &node, const QString &name)
{
    QDomNodeList nodes = node.childNodes();

    for (int j=0; j<nodes.size(); ++j)
    {
        QDomElement element = nodes.at(j).toElement();

        if (nodes.at(j).nodeName() == name)
        {
            //kDebug() << "Url : " << element.text();
            return element;
        }
    }
    return QDomElement();
}

//Find a bookmark group in a specifiec bookmark group of client
KBookmarkGroup OperaSyncHandler::findLocalGroup(const KBookmarkGroup &root, const QString &name)
{
    KBookmark child = root.first();

    while (!child.isNull())
    {
        kDebug() << child.fullText();
        if (child.isGroup() && name == child.fullText())
        {
            break;
        }
        child = root.next(child);
    }

    return child.toGroup();
}

//Find a bookmark in a specifiec bookmark group of client
KBookmark OperaSyncHandler::findLocalBookmark(const KBookmarkGroup &root, const KUrl &url)
{
    KBookmark child = root.first();

    kDebug() << "finding bookmark " << url << " in : " << root.text();

    while (!child.isNull())
    {
        kDebug() << child.url();
        if (!child.isGroup() && url == child.url())
        {
            break;
        }
        child = root.next(child);
    }

    return child;
}

//Find bookmark folder in xml returned by server
QDomElement OperaSyncHandler::findOperaFolder(const QDomElement &root, const QString &name)
{
    QDomElement current = root.firstChild().toElement();

    kDebug() << "Finding group " << name;
    while (!current.isNull())
    {
//        kDebug() << "in " <<  getTitleFromResourceProperties(current);
        if ((getChildString(current, "item_type") == "bookmark_folder") && getTitleFromResourceProperties(current) == name)
            break;
        current = current.nextSibling().toElement();
    }

    return current;
}

//Find bookmark in xml returned by server
QDomElement OperaSyncHandler::findOperaBookmark(const QDomElement &root, const KUrl &url)
{
    QDomElement current = root.firstChild().toElement();

    kDebug() << "Finding bookmark " << url;
    while (!current.isNull()  )
    {
//        kDebug() << "in " <<  getUrlFromResourceProperties(current);
        if ((getChildString(current, "item_type") == "bookmark") && KUrl(getUrlFromResourceProperties(current)) == url)
            break;
        current = current.nextSibling().toElement();
    }

    return current;
}

/*While sending changes to server, we need to keep track of requests which have been sent to server
to check whether syncing is finished.
*/
void OperaSyncHandler::decreaseRequestCount()
{
    if (_requestCount > 0)
    {
        --_requestCount;
    }

    if (_requestCount <= 0)
    {
        emit syncStatus(Rekonq::Bookmarks, true, i18n("Done!"));
        _isSyncing = false;
    }
}
