/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2008-2012 by Andrea Diamantini <adjam7 at gmail dot com>
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


// Self Includes
#include "downloadmanager.h"
#include "downloadmanager.moc"

// Auto Includes
#include "rekonq.h"

// KDE Includes
#include <KStandardDirs>
#include <KToolInvocation>
#include <KFileDialog>

#include <kio/scheduler.h>

#include <KIO/Job>
#include <KIO/CopyJob>
#include <KIO/JobUiDelegate>

// Qt Includes
#include <QDataStream>
#include <QDateTime>
#include <QFile>
#include <QFileInfo>
#include <QString>
#include <QWebSettings>
#include <QNetworkReply>

#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusInterface>


DownloadManager::DownloadManager(QObject *parent)
    : QObject(parent)
{
    init();
}


void DownloadManager::init()
{
    QString downloadFilePath = KStandardDirs::locateLocal("appdata" , "downloads");
    QFile downloadFile(downloadFilePath);
    if (!downloadFile.open(QFile::ReadOnly))
    {
        kDebug() << "Unable to open download file (READ mode)..";
        return;
    }

    QDataStream in(&downloadFile);
    while (!in.atEnd())
    {
        QString srcUrl;
        in >> srcUrl;
        QString destUrl;
        in >> destUrl;
        QDateTime dt;
        in >> dt;
        DownloadItem *item = new DownloadItem(srcUrl, destUrl, dt, this);
        m_downloadList.append(item);
    }
}


DownloadItem* DownloadManager::addDownload(const QString &srcUrl, const QString &destUrl)
{
    QWebSettings *globalSettings = QWebSettings::globalSettings();
    if (globalSettings->testAttribute(QWebSettings::PrivateBrowsingEnabled))
        return 0;
    QString downloadFilePath = KStandardDirs::locateLocal("appdata" , "downloads");
    QFile downloadFile(downloadFilePath);
    if (!downloadFile.open(QFile::WriteOnly | QFile::Append))
    {
        kDebug() << "Unable to open download file (WRITE mode)..";
        return 0;
    }
    QDataStream out(&downloadFile);
    out << srcUrl;
    out << destUrl;
    out << QDateTime::currentDateTime();
    downloadFile.close();
    DownloadItem *item = new DownloadItem(srcUrl, destUrl, QDateTime::currentDateTime(), this);
    m_downloadList.append(item);
    emit newDownloadAdded(item);
    return item;
}


bool DownloadManager::clearDownloadsHistory()
{
    m_downloadList.clear();
    QString downloadFilePath = KStandardDirs::locateLocal("appdata" , "downloads");
    QFile downloadFile(downloadFilePath);
    return downloadFile.remove();
}


void DownloadManager::downloadLinksWithKGet(const QVariant &contentList)
{
    if (!QDBusConnection::sessionBus().interface()->isServiceRegistered("org.kde.kget"))
    {
        KToolInvocation::kdeinitExecWait("kget");
    }
    QDBusInterface kget("org.kde.kget", "/KGet", "org.kde.kget.main");
    if (kget.isValid())
    {
        kget.call("importLinks", contentList);
    }
}


// NOTE
// These 2 functions have been copied from the KWebPage class to implement a local version of the downloadResponse method.
// In this way, we can easily provide the extra functionality we need:
// 1. KGet Integration
// 2. Save downloads history
bool DownloadManager::downloadResource(const KUrl &srcUrl, const KIO::MetaData &metaData,
                                       QWidget *parent, bool forceDirRequest, const QString &suggestedName)
{
    KUrl destUrl;

    const QString fileName((suggestedName.isEmpty() ? srcUrl.fileName() : suggestedName));

    if (forceDirRequest || ReKonfig::askDownloadPath())
    {
        // follow bug:184202 fixes
        destUrl = KFileDialog::getSaveFileName(KUrl::fromPath(fileName), QString(), parent);
    }
    else
    {
        destUrl = KUrl(ReKonfig::downloadPath().path() + QL1C('/') + fileName);
    }

    kDebug() << "DEST URL: " << destUrl;

    if (!destUrl.isValid())
        return false;

    // Save download history
    DownloadItem *item = addDownload(srcUrl.pathOrUrl(), destUrl.pathOrUrl());
    emit notifyDownload(fileName);

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

    KIO::Job *job = KIO::copy(srcUrl, destUrl, KIO::Overwrite);
    if (item)
    {
        QObject::connect(job, SIGNAL(percent(KJob*,ulong)), item, SLOT(updateProgress(KJob*,ulong)));
        QObject::connect(job, SIGNAL(finished(KJob*)), item, SLOT(onFinished(KJob*)));
    }

    if (!metaData.isEmpty())
        job->setMetaData(metaData);

    job->addMetaData(QL1S("MaxCacheSize"), QL1S("0")); // Don't store in http cache.
    job->addMetaData(QL1S("cache"), QL1S("cache")); // Use entry from cache if available.
    job->uiDelegate()->setAutoErrorHandlingEnabled(true);
    return true;
}
