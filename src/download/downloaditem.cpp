/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2008-2014 by Andrea Diamantini <adjam7 at gmail dot com>
* Copyright (C) 2011 by Pierre Rossi <pierre dot rossi at gmail dot com>
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


// Self includes
#include "downloaditem.h"

// Qt Includes
#include <QDBusInterface>
#include <QTimer>

// KDE Includes
#include <KIconLoader>
#include <KJob>
#include <QUrl>


DownloadItem::DownloadItem(const QString &srcUrl, const QString &destUrl, const QDateTime &d, QObject *parent)
    : QObject(parent)
    , m_srcUrlString(srcUrl)
    , m_destUrl(QUrl(destUrl))
    , m_dateTime(d)
    , m_job(0)
    , m_state(0)
{
}


DownloadItem::DownloadItem(KIO::CopyJob *job, const QDateTime &d, QObject *parent)
    : QObject(parent)
    , m_srcUrlString(job->srcUrls().at(0).url())
    , m_destUrl(job->destUrl())
    , m_dateTime(d)
    , m_job(job)
    , m_state(0)
{
    QObject::connect(job, SIGNAL(percent(KJob*,ulong)), this, SLOT(updateProgress(KJob*,ulong)));
    QObject::connect(job, SIGNAL(finished(KJob*)), this, SLOT(onFinished(KJob*)));
    QObject::connect(job, SIGNAL(suspended(KJob*)), this, SLOT(onSuspended(KJob*)));
}


QUrl DownloadItem::destUrl() const
{
    return m_destUrl;
}


QString DownloadItem::originUrl() const
{
    return m_srcUrlString;
}


QString DownloadItem::fileDirectory() const
{
    QUrl u = destUrl();
    return (QL1S("file://") + u.path());
}


QString DownloadItem::fileName() const
{
    return destUrl().fileName();
}


QString DownloadItem::destinationUrlString() const
{
    return destUrl().url(QUrl::StripTrailingSlash);
}


QString DownloadItem::icon() const
{
    KIconLoader *loader = KIconLoader::global();
    QString iconForMimeType = KIO::iconNameForUrl(destUrl());
    return (QL1S("file://") + loader->iconPath(iconForMimeType, KIconLoader::Desktop));
}


void DownloadItem::setIsKGetDownload()
{
    m_state = KGetManaged;
}


// update progress for the plain KIO::Job backend
void DownloadItem::updateProgress(KJob *job, unsigned long value)
{
    Q_UNUSED(job);

    if (value > 0 && value < 100)
        m_state = Downloading;

    emit downloadProgress(value);
}


// emit downloadFinished signal in KJob case
void DownloadItem::onFinished(KJob *job)
{
    if (job->error())
    {
        m_state = Errors;
        m_errorString = job->errorString();
    }
    else
    {
        m_state = Done;
        emit downloadProgress(100);
    }

    emit downloadFinished(!job->error());
}


void DownloadItem::onSuspended(KJob *job)
{
    Q_UNUSED(job);

    m_state = Suspended;

    // TODO:
    // connect to job->resume() to let rekonq resume it
}


QString DownloadItem::errorString() const
{
    return m_errorString;
}
