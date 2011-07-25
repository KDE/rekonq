/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2009-2010 by Richard J. Moore <rich@kde.org>
* Copyright (C) 2010-2011 by Andrea Diamantini <adjam7 at gmail dot com>
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
#include "networkanalyzer.h"
#include "networkanalyzer.moc"

// KDE Includes
#include <KAction>
#include <klocalizedstring.h>
#include <KMenu>
#include <KIcon>
#include <KPassivePopup>

// Qt Includes
#include <QtGui/QTreeWidget>
#include <QtGui/QVBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QApplication>
#include <QSignalMapper>
#include <QClipboard>


NetworkAnalyzer::NetworkAnalyzer(QWidget *parent)
    : QWidget(parent)
    , _mapper(new QSignalMapper(this))
    , _requestList(new QTreeWidget(this))
{
    QStringList headers;
    headers << i18n("Method") << i18n("URL") << i18n("Response") << i18n("Length") << i18n("Content Type") << i18n("Info");
    _requestList->setHeaderLabels(headers);

    _requestList->header()->setResizeMode(0, QHeaderView::Interactive);
    _requestList->header()->setResizeMode(1, QHeaderView::Interactive);
    _requestList->header()->setResizeMode(2, QHeaderView::Interactive);
    _requestList->header()->setResizeMode(3, QHeaderView::Interactive);
    _requestList->header()->setResizeMode(4, QHeaderView::Interactive);

    _requestList->setAlternatingRowColors(true);

    QVBoxLayout *lay = new QVBoxLayout(this);
    lay->addWidget(_requestList);

    _requestList->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(_mapper, SIGNAL(mapped(QObject *)), this, SLOT(requestFinished(QObject *)));
    connect(_requestList, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(showItemDetails(QTreeWidgetItem *)));
    connect(_requestList, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(popupContextMenu(QPoint)));
}


void NetworkAnalyzer::popupContextMenu(const QPoint& pos)
{
    if (_requestList->topLevelItemCount() >= 1)
    {
        KMenu menu(_requestList);
        KAction *copy;
        copy = new KAction(KIcon("edit-copy"), i18n("Copy URL"), this);
        connect(copy, SIGNAL(triggered(bool)), this, SLOT(copyURL()));
        menu.addAction(copy);
        menu.exec(mapToGlobal(pos));
    }
}

void NetworkAnalyzer::copyURL()
{
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(_requestList->currentItem()->text(1));
}

void NetworkAnalyzer::addRequest(QNetworkAccessManager::Operation op, const QNetworkRequest &req, QNetworkReply *reply)
{
    // Add to list of requests
    QStringList cols;
    switch (op)
    {
    case QNetworkAccessManager::HeadOperation:
        cols << QL1S("HEAD");
        break;
    case   QNetworkAccessManager::GetOperation:
        cols << QL1S("GET");
        break;
    case QNetworkAccessManager::PutOperation:
        cols << QL1S("PUT");
        break;
    case QNetworkAccessManager::PostOperation:
        cols << QL1S("POST");
        break;
    case QNetworkAccessManager::DeleteOperation:
        cols << QL1S("DELETE");
        break;
    case QNetworkAccessManager::CustomOperation:
        cols << QL1S("CUSTOM");
        break;

    default:
        kDebug() << "Unknown network operation";
    }
    cols << req.url().toString();
    cols << i18n("Pending");

    QTreeWidgetItem *item = new QTreeWidgetItem(cols);
    _requestList->addTopLevelItem(item);

    // Add to maps
    _requestMap.insert(reply, req);
    _itemMap.insert(reply, item);
    _itemRequestMap.insert(item, req);

    _mapper->setMapping(reply, reply);
    connect(reply, SIGNAL(finished()), _mapper, SLOT(map()));

    _requestList->header()->resizeSections(QHeaderView::ResizeToContents);
}


void NetworkAnalyzer::clear()
{
    _requestMap.clear();
    _itemMap.clear();
    _itemReplyMap.clear();
    _itemRequestMap.clear();
    _requestList->clear();
}


void NetworkAnalyzer::requestFinished(QObject *replyObject)
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(replyObject);
    if (!reply)
    {
        kDebug() << "Failed to downcast reply";
        return;
    }

    QTreeWidgetItem *item = _itemMap[reply];
    if (!item)
    {
        kDebug() << "No Item mapped. Returning...";
        return;
    }

    // Record the reply headers
    QList<QByteArray> headerValues;
    foreach(const QByteArray & header, reply->rawHeaderList())
    {
        headerValues += reply->rawHeader(header);
    }

    QPair< QList<QByteArray>, QList<QByteArray> > replyHeaders;
    replyHeaders.first = reply->rawHeaderList();
    replyHeaders.second = headerValues;
    _itemReplyMap[item] = replyHeaders;

    // Display the request
    int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QString reason = reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString();
    item->setText(2, i18n("%1 %2", status, reason));

    QString length = reply->header(QNetworkRequest::ContentLengthHeader).toString();
    item->setText(3, length);

    QString contentType = reply->header(QNetworkRequest::ContentTypeHeader).toString();
    item->setText(4, contentType);

    if (status == 302)
    {
        QUrl target = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
        item->setText(5, i18n("Redirect: %1", target.toString()));
    }
}


void NetworkAnalyzer::showItemDetails(QTreeWidgetItem *item)
{
    // Show request details
    QString details;

    QNetworkRequest req = _itemRequestMap[item];
    details += i18n("<h3>Request Details</h3>");
    details += QL1S("<ul>");
    foreach(const QByteArray & header, req.rawHeaderList())
    {
        details += QL1S("<li>");
        details += QL1S(header);
        details += QL1S(": ");
        details += QL1S(req.rawHeader(header));
        details += QL1S("</li>");
    }
    details += QL1S("</ul>");

    QPair< QList<QByteArray>, QList<QByteArray> > replyHeaders = _itemReplyMap[item];
    details += i18n("<h3>Response Details</h3>");
    details += QL1S("<ul>");
    for (int i = 0; i < replyHeaders.first.count(); i++)
    {
        details += QL1S("<li>");
        details += QL1S(replyHeaders.first[i]);
        details += QL1S(": ");
        details += QL1S(replyHeaders.second[i]);
        details += QL1S("</li>");
    }
    details += QL1S("</ul>");

//     QLabel *label = new QLabel(details, this);
//     KPassivePopup *popup = new KPassivePopup(this);
//     popup->setView(label);
//     popup->show(_requestList->mapToGlobal(_requestList->pos()));
    KPassivePopup::message(details, this);
}
