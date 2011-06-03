/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2008-2010 by Andrea Diamantini <adjam7 at gmail dot com>
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
#include "downloaditem.moc"

// Qt Includes
#include <QDBusInterface>
#include <QTimer>

// KDE Includes
#include <KIconLoader>
#include <KJob>
#include <KMimeType>
#include <KUrl>


DownloadItem::DownloadItem(const QString &srcUrl, const QString &destUrl, const QDateTime &d, QObject *parent)
    : QObject(parent)
    , m_srcUrlString(srcUrl)
    , m_destUrl(KUrl(destUrl))
    , m_dateTime(d)
    , m_shouldAbort(false)
{
}


QString DownloadItem::fileDirectory() const
{
    return (QL1S("file://") + m_destUrl.directory());
}


QString DownloadItem::fileName() const
{
    return m_destUrl.fileName();
}


QString DownloadItem::destinationUrl() const
{
    return m_destUrl.url(KUrl::RemoveTrailingSlash);
}


QString DownloadItem::icon() const
{
    KIconLoader *loader = KIconLoader::global();
    QString iconForMimeType = KMimeType::iconNameForUrl(m_destUrl);
    return (QL1S("file://") + loader->iconPath(iconForMimeType, KIconLoader::Desktop));
}


// update progress for the plain KIO::Job backend
void DownloadItem::updateProgress(KJob *job, unsigned long value)
{
    if (m_shouldAbort)
        job->kill(KJob::EmitResult);
    emit downloadProgress(value);
}


// emit downloadFinished signal in KJob case
void DownloadItem::onFinished(KJob *job)
{
    if (!job->error())
        emit downloadProgress(100);
    emit downloadFinished(!job->error());
}


// sets up progress handling for the KGet backend
void DownloadItem::setKGetTransferDbusPath(const QString &path)
{
    m_kGetPath = path;
    QTimer *updateTimer = new QTimer(this);
    updateTimer->setInterval(300);
    updateTimer->setSingleShot(false);
    connect(updateTimer, SIGNAL(timeout()), SLOT(updateProgress()));
    updateTimer->start();
}


/*
* update progress (polling in KGet case)
*
* Notes for KGet dbus interface:
* status values:
*  - 0 running
*  - 2 stopped
*  - 4 finished
*/
void DownloadItem::updateProgress()
{
    if (m_kGetPath.isEmpty())
        return;
    QDBusInterface kgetTransfer(QL1S("org.kde.kget"), m_kGetPath, QL1S("org.kde.kget.transfer"));
    if (!kgetTransfer.isValid())
        return;
    // Fetch percent from DBus
    QDBusMessage percentRes = kgetTransfer.call(QL1S("percent"));
    if (percentRes.arguments().isEmpty())
        return;
    bool ok = false;
    const int percent = percentRes.arguments().first().toInt(&ok);
    if (!ok)
        return;
    // Fetch status from DBus
    QDBusMessage statusRes = kgetTransfer.call(QL1S("status"));
    if (statusRes.arguments().isEmpty())
        return;
    ok = false;
    const int status = statusRes.arguments().first().toInt(&ok);
    if (!ok)
        return;
    emit downloadProgress(percent);
    // TODO: expose resume if stopped
    // special case for status 2 will come later when we have a way to support resume.
    if (percent == 100 || status == 4 || status == 2) {
        emit downloadFinished(true);
        QTimer *timer = qobject_cast<QTimer *>(sender());
        if (timer)
            timer->stop();
    }
}


void DownloadItem::abort() const
{
    if (!m_kGetPath.isEmpty())
    {
        QDBusInterface kgetTransfer(QL1S("org.kde.kget"), m_kGetPath, QL1S("org.kde.kget.transfer"));
        if (kgetTransfer.isValid())
            kgetTransfer.call(QL1S("stop"));
    }
    else
    {
        // using KIO::JOB, kill at the next update :)
        m_shouldAbort = true;
    }
}

// TODO: ability to remove single items from the page...
