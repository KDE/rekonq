/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2012 by Siteshwar Vashisht <siteshwar at gmail dot com>
* Copyright (C) 2011-2013 by Andrea Diamantini <adjam7 at gmail dot com>
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
#include "googlesynchandler.h"
#include "googlesynchandler.moc"

// Auto Includes
#include "rekonq.h"

// Local Includes
#include "bookmarkmanager.h"

// KDE Includes
#include <KStandardDirs>
#include <KLocalizedString>
#include <KBookmarkManager>

#include <QList>
#include <QWebPage>
#include <QWebFrame>
#include <QWebElement>
#include <QUrl>
#include <QWebSettings>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QDomDocument>


GoogleSyncHandler::GoogleSyncHandler(QObject *parent)
    : SyncHandler(parent)
    , _mode(RECEIVE_CHANGES)
    , _doLogin(false)
    , _isSyncing(false)
    , _reply(0)
    , _requestCount(0)
{
    kDebug() << "Creating Google Bookmarks handler...";
    _webPage.settings()->setAttribute(QWebSettings::AutoLoadImages, false);
    _webPage.settings()->setAttribute(QWebSettings::PrivateBrowsingEnabled, true);
    connect(&_webPage, SIGNAL(loadFinished(bool)), this, SLOT(loadFinished(bool)));
}


void GoogleSyncHandler::initialLoadAndCheck()
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
        emit syncStatus(Rekonq::History, false, i18n("Not supported"));
    }

    if (ReKonfig::syncHistory())
    {
        emit syncStatus(Rekonq::Passwords, false, i18n("Not supported"));
    }
}


bool GoogleSyncHandler::syncRelativeEnabled(bool check)
{
    if (!ReKonfig::syncEnabled())
        return false;

    if (!_firstTimeSynced)
        return false;

    return check;
}


// ---------------------------------------------------------------------------------------


void GoogleSyncHandler::syncHistory()
{
    kDebug() << "Syncing history not supported!";
    emit syncStatus(Rekonq::History, false, i18n("Syncing history not supported"));
    emit syncHistoryFinished(false);
}


void GoogleSyncHandler::syncPasswords()
{
    kDebug() << "Syncing passwords not supported!";
    emit syncStatus(Rekonq::Passwords, false, i18n("Syncing passwords not supported"));
    emit syncPasswordsFinished(false);
}


void GoogleSyncHandler::syncBookmarks()
{

    if (_isSyncing)
    {
        kDebug() << "Sync already in progress!";
        return;
    }
    _mode = SEND_CHANGES;
    startLogin();
}

void GoogleSyncHandler::startLogin()
{
    if (ReKonfig::syncUser().isEmpty() || ReKonfig::syncPass().isEmpty())
    {
        kDebug() << "No username or password!";
        emit syncStatus(Rekonq::Bookmarks, false, i18n("No username or password"));
        emit syncBookmarksFinished(false);
        return;
    }

    _isSyncing = true;

    _doLogin = true;

    kDebug() << "Loading login page...";
    _webPage.mainFrame()->load(QUrl("http://bookmarks.google.com/"));
}

//Loading a webpage finished, what action to take is decided based on url we have loaded.
void GoogleSyncHandler::loadFinished(bool ok)
{
    kDebug() << "Load Finished" << ok;
    if (!ok)
    {
        kDebug() << "Error loading: " << _webPage.mainFrame()->url();
        emit syncStatus(Rekonq::Bookmarks, false, i18n("Error loading: %1", _webPage.mainFrame()->url().toString()));

        _isSyncing = false;
        return;
    }

    kDebug() << _webPage.mainFrame()->url();
    kDebug() << "Path : " << _webPage.mainFrame()->url().path();

    QString path = _webPage.mainFrame()->url().path();

    if ( (path == QL1S("/ServiceLogin") || path == QL1S("/Login")) && _doLogin == true)
    {
        // Let's login to our Google account
        QWebFrame *frame = _webPage.mainFrame();

        QWebElement email = frame->findFirstElement( QL1S("#Email") );
        QWebElement passwd = frame->findFirstElement( QL1S("#Passwd") );
        QWebElement form = frame->findFirstElement( QL1S("#gaia_loginform") );

        email.setAttribute( QL1S("value"), ReKonfig::syncUser());
        passwd.setAttribute( QL1S("value"), ReKonfig::syncPass());
        form.evaluateJavaScript( QL1S("this.submit();") );
        emit syncStatus(Rekonq::Bookmarks, true, i18n("Signing in..."));

        // Login only once
        _doLogin = false;
    }
    else if (path == QL1S("/bookmarks/") )
    {
        // We get to this page after successful login, let's fetch the bookmark list in Xml format.
        QNetworkAccessManager *qnam = _webPage.networkAccessManager();
        QNetworkRequest request;
        request.setUrl(QUrl("http://www.google.com/bookmarks/?output=xml"));
        _reply = qnam->get(request);
        emit syncStatus(Rekonq::Bookmarks, true, i18n("Fetching bookmarks from server..."));
        connect(_reply, SIGNAL(finished()), this, SLOT(fetchingBookmarksFinished()));
    }
    else if (path == QL1S("/ServiceLoginAuth") )
    {
        emit syncStatus(Rekonq::Bookmarks, false, i18n("Login failed"));
        _isSyncing = false;
    }
    else if (path == QL1S("/bookmarks/mark") )
    {
        QWebFrame *frame = _webPage.mainFrame();

        QString sigKey = frame->findFirstElement( QL1S("input[name=sig]") ).attribute( QL1S("value") );
        kDebug() << "Signature Key is : " << sigKey;

        QNetworkAccessManager *qnam = _webPage.networkAccessManager();

        if (!_bookmarksToDelete.isEmpty())
        {

            for (QSet<QString>::const_iterator iter = _bookmarksToDelete.constBegin(); iter != _bookmarksToDelete.end(); ++iter)
            {
                QNetworkRequest request;
                request.setUrl(QUrl( QL1S("https://www.google.com/bookmarks/mark?dlq=") + *iter + QL1S("&sig=") + sigKey));

                kDebug() << "Delete url is : " << request.url();
                QNetworkReply *r = qnam->get(request);
                connect(r, SIGNAL(finished()), this, SLOT(updateBookmarkFinished()));
                ++_requestCount;
            }
        }

        if (!_bookmarksToAdd.isEmpty())
        {
            emit syncStatus(Rekonq::Bookmarks, true, i18n("Adding bookmarks on server..."));
            for (QSet<KUrl>::const_iterator iter = _bookmarksToAdd.constBegin(); iter != _bookmarksToAdd.end(); ++iter)
            {
                KBookmark bookmark = BookmarkManager::self()->bookmarkForUrl(*iter);
                QByteArray postData;
                postData.append("bkmk=" + QUrl::toPercentEncoding(bookmark.url().url().toUtf8()));
                postData.append("&title=" + QUrl::toPercentEncoding(bookmark.text().toUtf8()));
                postData.append("&annotation=");
                postData.append("&labels=");
                postData.append("&prev=/lookup");
                postData.append("&sig=" + sigKey.toUtf8());

                QNetworkRequest request;
                request.setUrl(QUrl("https://www.google.com/bookmarks/mark?sig=" + sigKey + QL1S("&btnA") ));
                request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
                kDebug() << "Url: " << request.url();
                kDebug() << "Post data is :" << postData;
                QNetworkReply *r = qnam->post(request, postData);
                connect(r, SIGNAL(finished()), this, SLOT(updateBookmarkFinished()));
                ++_requestCount;
            }
        }

        _bookmarksToDelete.clear();
        _bookmarksToAdd.clear();

    }
    else if (path == QL1S("/Logout") )
    {
        //Session finished
        emit syncStatus(Rekonq::Bookmarks, true, i18n("Done"));
        emit syncBookmarksFinished(true);
        _isSyncing = false;
    }
    else
    {
        kDebug() << "Unknown Response!";
        _isSyncing = false;
    }

}

//We received bookmarks stored on server in xml format, now take the action based on which mode we are in.
void GoogleSyncHandler::fetchingBookmarksFinished()
{
    QString data = _reply->readAll();

    QDomDocument doc( QL1S("bookmarks") );
    doc.setContent(data);

    QDomNodeList bookmarksOnServer = doc.elementsByTagName( QL1S("bookmark") );
    emit syncStatus(Rekonq::Bookmarks, true, i18n("Reading bookmarks..."));

    BookmarkManager *manager = BookmarkManager::self();
    KBookmarkGroup root = manager->rootGroup();

    if (_mode == RECEIVE_CHANGES)
    {

        for (int i = 0; i < bookmarksOnServer.size(); ++i)
        {
            QString title = getChildElement(bookmarksOnServer.at(i), QL1S("title") );
            QString url = getChildElement(bookmarksOnServer.at(i), QL1S("url") );

            KBookmark bookmark = manager->bookmarkForUrl(KUrl(url));
            if (bookmark.isNull())
            {
                //Add bookmark
                kDebug() << "Add bookmark";
                emit syncStatus(Rekonq::Bookmarks, true, i18n("Adding bookmark"));
                root.addBookmark(title.isEmpty() ? url : title, KUrl(url));
                manager->manager()->emitChanged(root);
            }

        }

        // After receiving changes, we compare local bookmarks with Google bookmarks and if some bookmarks exist locally but not on Google Bookmarks, we add them.
        checkToAddGB(root, bookmarksOnServer);

        if (!_bookmarksToAdd.isEmpty())
        {
            kDebug() << "Getting sigkey";
            _webPage.mainFrame()->load(QUrl("https://www.google.com/bookmarks/mark?op=add&hl=en"));
        }
        else
        {
            _webPage.mainFrame()->load(QUrl("https://accounts.google.com/Logout?hl=en"));
            emit syncStatus(Rekonq::Bookmarks, true, i18n("Signing out..."));
        }
    }
    else
    {
        checkToAddGB(root, bookmarksOnServer);
        checkToDeleteGB(manager, bookmarksOnServer);

        if (!_bookmarksToAdd.isEmpty() || !_bookmarksToDelete.isEmpty())
        {
            kDebug() << "Getting sigkey";
            _webPage.mainFrame()->load(QUrl("https://www.google.com/bookmarks/mark?op=add&hl=en"));
        }
        else
        {
            _webPage.mainFrame()->load(QUrl("https://accounts.google.com/Logout?hl=en"));
            emit syncStatus(Rekonq::Bookmarks, true, i18n("Signing out..."));
        }
    }

    _reply->deleteLater();
}

//Get value of a child element of a dom node
QString GoogleSyncHandler::getChildElement(const QDomNode &node, QString name)
{
    QDomNodeList nodes = node.childNodes();

    for (int j = 0; j < nodes.size(); ++j)
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

//This method checks whether we have any other bookmarks than the ones which exist on the server
void GoogleSyncHandler::checkToAddGB(const KBookmarkGroup &root, const QDomNodeList &bookmarksOnServer)
{
    KBookmark current = root.first();

    while (!current.isNull())
    {
        kDebug() << "Checking Url to add on Google Bookmarks: " << current.url();
        bool found = false;
        for (int i = 0; i < bookmarksOnServer.count(); ++i)
        {
            if (current.isGroup())
            {
                kDebug() << "Checking group" << current.text();
                checkToAddGB(current.toGroup(), bookmarksOnServer);
                //skip adding a blank in _bookmarksToAdd
                found = true;
                break;
            }
            else if (current.url().url() == getChildElement(bookmarksOnServer.at(i), QL1S("url")) )
            {
                found = true;
            }
        }

        if (!found)
        {
            kDebug() <<  "Adding to Google Bookmarks: " << current.url().url();
            _bookmarksToAdd.insert(current.url());
        }
        current = root.next(current);
    }
}

//Check whether we need to delete bookmarks while sending changes to Google Bookmarks
void GoogleSyncHandler::checkToDeleteGB(BookmarkManager *manager, const QDomNodeList &bookmarksOnServer)
{

    for (int i = 0; i < bookmarksOnServer.count(); ++i)
    {
        QString url = getChildElement(bookmarksOnServer.at(i), QL1S("url") );

        KBookmark result = manager->bookmarkForUrl(KUrl(url));
        if (result.isNull())
        {
            kDebug() <<  "Deleting from Google Bookmarks: " << url;
            _bookmarksToDelete.insert(getChildElement(bookmarksOnServer.at(i), QL1S("id") ));
        }
    }

}


//Added or deleted a bookmark on server, check whether we succeed here, and logout when all requests are done!
void GoogleSyncHandler::updateBookmarkFinished()
{
    --_requestCount;
    QNetworkReply *reply = dynamic_cast<QNetworkReply*>(sender());
    if (reply->error() != QNetworkReply::NoError)
        kDebug() << "Network Error while adding bookmark to server, code is: " << reply->error();
    else if (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute) != 302)
        kDebug() << "Unexpected reply : " << reply->readAll();
    else
        kDebug() << "Success!";

    if (_requestCount <= 0)
    {
        _webPage.mainFrame()->load(QUrl("https://accounts.google.com/Logout?hl=en"));
        emit syncStatus(Rekonq::Bookmarks, true, i18n("Signing out..."));
    }

}
