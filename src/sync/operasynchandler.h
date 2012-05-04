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


#ifndef OPERA_SYNC_HANDLER_H
#define OPERA_SYNC_HANDLER_H

#include <QWebPage>

// Local Includes
#include "synchandler.h"

// KDE Includes
#include <KUrl>
#include <KBookmarkGroup>
#include <KIO/Job>

// Qt Includes
#include <QtOAuth/QtOAuth>

// Forward Declarations
class QNetworkReply;
class BookmarkManager;
class QWebView;

class OperaSyncHandler : public SyncHandler
{
    Q_OBJECT

public:
    OperaSyncHandler(QObject *parent = 0);

    void syncHistory();
    void syncBookmarks();
    void syncPasswords();

    void initialLoadAndCheck();

private Q_SLOTS:
    void loadFinished(bool);
/*    void fetchingBookmarksFinished();
    void updateBookmarkFinished();
*/
    void dataSlot(KIO::Job*, QByteArray);
    void resultSlot(KJob*);

    void bookmarkDataSlot(KIO::Job*, QByteArray);
    void bookmarkResultSlot(KJob*);

    void bookmarkFolderDataSlot(KIO::Job*, QByteArray);
    void bookmarkFolderResultSlot(KJob*);

Q_SIGNALS:
    void syncBookmarksFinished(bool);
    void syncHistoryFinished(bool);
    void syncPasswordsFinished(bool);

private:
    enum {SEND_CHANGES, RECEIVE_CHANGES} _mode;
    enum BookmarkType {BOOKMARK, BOOKMARK_FOLDER};

    bool syncRelativeEnabled(bool);
    void startLogin();
    void getBookmarks();
    void handleBookmark(const QDomElement &item, KBookmarkGroup root);
    void handleBookmarkFolder(const QDomElement &item, KBookmarkGroup &root);
    void handleResource(const QDomNode &item, KBookmarkGroup &root);

//    void handleLocalBookmark(const KBookmarkGroup &root, const QDomElement &item);
    void handleLocalGroup(const KBookmarkGroup &root, const QDomElement &item, QString parentId);

    void addBookmarkOnServer(QString, QString, QString parent = QString());
    KJob *addBookmarkFolderOnServer(QString, QString parent = QString());
    void deleteResourceOnServer(QString id);

    static QString getUrlFromResourceProperties(const QDomElement &item);
    static QString getTitleFromResourceProperties(const QDomElement &item);
    static QString getIdFromResource(const QDomElement &item);
    static QString getChildString(const QDomNode &node, const QString &name);
    static QDomElement getChildElement(const QDomNode &node, const QString &name);

    static KBookmarkGroup findLocalGroup(const KBookmarkGroup &root, const QString &name);
    static KBookmark findLocalBookmark(const KBookmarkGroup &root, const KUrl &url);

    static QDomElement findOperaFolder(const QDomElement &root, const QString &name);
    static QDomElement findOperaBookmark(const QDomElement &root, const KUrl &url);

/*    void checkToAddGB(const KBookmarkGroup &root, const QDomNodeList &);
    void checkToDeleteGB(BookmarkManager *, const QDomNodeList &);
    QString getChildElement(const QDomNode &node, QString name);
    void checkRequestCount();*/


    //    QUrl _remoteBookmarksUrl;
    bool _doLogin;

    QWebPage _webPage;
/*    QNetworkReply *_reply;
    QSet<KUrl> _bookmarksToAdd;
    QSet<QString> _bookmarksToDelete;
    unsigned int _requestCount;*/

    bool _isSyncing;

    QOAuth::Interface *qoauth;
    QOAuth::ParamMap requestParam, resultParam, authParams;
    QByteArray requestToken, requestTokenSecret;
    QByteArray authToken, authTokenSecret;

    QUrl url;
//    QSslSocket *socket;
    QByteArray urlParams;
    QWebView *webView;
    QByteArray xmlData;
    QMap<KJob*, QByteArray> jobToResponseMap;
    QMap<KJob*, KBookmarkGroup> jobToGroupMap;
};

#endif // OPERA_SYNC_HANDLER_H
