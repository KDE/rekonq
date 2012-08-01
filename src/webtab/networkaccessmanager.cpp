/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2007-2008 Trolltech ASA. All rights reserved
* Copyright (C) 2008-2012 by Andrea Diamantini <adjam7 at gmail dot com>
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
#include "networkaccessmanager.h"
#include "networkaccessmanager.moc"

// Local Includes
#include "adblockmanager.h"

// KDE Includes
#include <KLocale>
#include <KProtocolManager>
#include <KRun>

// Qt Includes
#include <QNetworkReply>
#include <QTimer>
#include <QWebElement>
#include <QWebFrame>
#include <QWidget>


class NullNetworkReply : public QNetworkReply
{
public:
    NullNetworkReply(const QNetworkRequest &req, QObject* parent = 0)
        : QNetworkReply(parent)
    {
        setRequest(req);
        setUrl(req.url());
        setHeader(QNetworkRequest::ContentLengthHeader, 0);
        setHeader(QNetworkRequest::ContentTypeHeader, "text/plain");
        setError(QNetworkReply::ContentAccessDenied, i18n("Blocked by ad filter"));
        setAttribute(QNetworkRequest::User, QNetworkReply::ContentAccessDenied);
        QTimer::singleShot(0, this, SIGNAL(finished()));
    }

    virtual void abort() {}
    virtual qint64 bytesAvailable() const
    {
        return 0;
    }

protected:
    virtual qint64 readData(char*, qint64)
    {
        return -1;
    }
};


// ----------------------------------------------------------------------------------------------


#define     HIDABLE_ELEMENTS    QL1S("audio,img,embed,object,iframe,frame,video")


static void hideBlockedElements(const QUrl& url, QWebElementCollection& collection)
{
    for (QWebElementCollection::iterator it = collection.begin(); it != collection.end(); ++it)
    {
        const QUrl baseUrl ((*it).webFrame()->baseUrl());
        QString src = (*it).attribute(QL1S("src"));
        
        if (src.isEmpty())
            src = (*it).evaluateJavaScript(QL1S("this.src")).toString();

        if (src.isEmpty())
            continue;
        const QUrl resolvedUrl (baseUrl.resolved(src));
        if (url == resolvedUrl)
        {
            //kDebug() << "*** HIDING ELEMENT: " << (*it).tagName() << resolvedUrl;
            (*it).removeFromDocument();
        }
    }
}


// ----------------------------------------------------------------------------------------------


NetworkAccessManager::NetworkAccessManager(QObject *parent)
    : AccessManager(parent)
{
    QString c = KGlobal::locale()->language();

    if (c == QL1S("C"))
        c = QL1S("en-US");
    else
        c = c.replace(QL1C('_') , QL1C('-'));

    c.append(QL1S(", en-US; q=0.8, en; q=0.6"));

    _acceptLanguage = c.toLatin1();
}


QNetworkReply *NetworkAccessManager::createRequest(Operation op, const QNetworkRequest &req, QIODevice *outgoingData)
{
    bool blocked = false;
    
    // Handle GET operations with AdBlock
    if (op == QNetworkAccessManager::GetOperation)
        blocked = AdBlockManager::self()->blockRequest(req);
    
    if (!blocked)
    {
        if (KProtocolInfo::isHelperProtocol(req.url()))
        {
            (void) new KRun(req.url(), qobject_cast<QWidget*>(req.originatingObject()));
            return new NullNetworkReply(req, this);
        }

        // set our "nice" accept-language header...
        QNetworkRequest request = req;
        request.setRawHeader("Accept-Language", _acceptLanguage);

        return KIO::AccessManager::createRequest(op, request, outgoingData);
    }

    QWebFrame* frame = qobject_cast<QWebFrame*>(req.originatingObject());
    if (frame)
    {
        if (!m_blockedRequests.contains(frame))
            connect(frame, SIGNAL(loadFinished(bool)), this, SLOT(slotFinished(bool)));
        m_blockedRequests.insert(frame, req.url());
    }

    return new NullNetworkReply(req, this);
}


void NetworkAccessManager::slotFinished(bool ok)
{
    if (!ok)
        return;

    if(!AdBlockManager::self()->isEnabled())
        return;

    if(!AdBlockManager::self()->isHidingElements())
        return;

    QWebFrame* frame = qobject_cast<QWebFrame*>(sender());
    if (!frame)
        return;

    QList<QUrl> urls = m_blockedRequests.values(frame);
    if (urls.isEmpty())
        return;

   QWebElementCollection collection = frame->findAllElements(HIDABLE_ELEMENTS);
   if (frame->parentFrame())
        collection += frame->parentFrame()->findAllElements(HIDABLE_ELEMENTS);

    Q_FOREACH(const QUrl& url, urls)
        hideBlockedElements(url, collection);
}
