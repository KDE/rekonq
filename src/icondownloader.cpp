/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2012 by Andrea Diamantini <adjam7 at gmail dot com>
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


#include "icondownloader.h"
#include "icondownloader.moc"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QFile>
#include <QPixmap>


IconDownloader::IconDownloader(const KUrl &srcUrl, const KUrl &destUrl, QObject *parent)
    : QObject(parent)
    , m_srcUrl(srcUrl)
    , m_destUrl(destUrl)
{
    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));
    manager->get(QNetworkRequest(srcUrl));
}


void IconDownloader::replyFinished(QNetworkReply *reply)
{
    if (reply->error())
    {
        kDebug() << "FAVICON JOB ERROR";
        emit iconReady();
        this->deleteLater();
        return;
    }

    QString s = m_destUrl.url().remove(QL1S("file://"));
    QFile favicon(s);
    if (!favicon.open(QIODevice::WriteOnly))
    {
        kDebug() << "FAVICON FILE NOT OPENED";
        emit iconReady();
        this->deleteLater();
        return;
    }

    favicon.write(reply->readAll());
    favicon.close();

    if (favicon.size() == 0)
    {
        kDebug() << "SIZE ZERO FAVICON";
        favicon.remove();
        emit iconReady();
        this->deleteLater();
        return;
    }

    QPixmap px;
    if (!px.load(s))
    {
        kDebug() << "PIXMAP NOT LOADED";
        emit iconReady();
        this->deleteLater();
        return;
    }

    if (px.isNull())
    {
        kDebug() << "PIXMAP IS NULL";
        favicon.remove();
        emit iconReady();
        this->deleteLater();
        return;
    }

    px = px.scaled(16, 16);
    if (!px.save(s + QL1S(".png"), "PNG"))
    {
        kDebug() << "PIXMAP NOT SAVED";
        emit iconReady();
        this->deleteLater();
        return;
    }

    QFile::remove(s);
    emit iconReady();
    this->deleteLater();
}
