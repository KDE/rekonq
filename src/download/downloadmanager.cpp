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


// Self Includes
#include "downloadmanager.h"

// Auto Includes
#include "rekonq.h"

// KDE Includes
#include <KToolInvocation>

#include <kio/scheduler.h>

#include <KIO/Job>
#include <KIO/CopyJob>
#include <KIO/JobUiDelegate>

// Qt Includes
#include <QApplication>
#include <QDataStream>
#include <QDateTime>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusInterface>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QNetworkReply>
#include <QStandardPaths>
#include <QString>
#include <QWebSettings>


QPointer<DownloadManager> DownloadManager::s_downloadManager;


DownloadManager *DownloadManager::self()
{
    if (s_downloadManager.isNull())
    {
        s_downloadManager = new DownloadManager(qApp);
    }
    return s_downloadManager.data();
}


// ----------------------------------------------------------------------------------------------


DownloadManager::DownloadManager(QObject *parent)
    : QObject(parent)
    , m_needToSave(false)
{
    init();
}


DownloadManager::~DownloadManager()
{
    if (!m_needToSave)
        return;

    QString downloadFilePath = QStandardPaths::writableLocation(QStandardPaths::DataLocation) + QL1S("/downloads");
    QFile downloadFile(downloadFilePath);

    if (!downloadFile.open(QFile::WriteOnly))
    {
        qDebug() << "Unable to open download file (WRITE mode)..";
        return;
    }

    QDataStream out(&downloadFile);
    Q_FOREACH(DownloadItem * item, m_downloadList)
    {
        out << item->originUrl();
        out << item->destinationUrlString();
        out << item->dateTime();
    }

    downloadFile.close();
}


void DownloadManager::init()
{
    QString downloadFilePath = QStandardPaths::writableLocation(QStandardPaths::DataLocation) + QL1S("/downloads");
    QFile downloadFile(downloadFilePath);
    if (!downloadFile.open(QFile::ReadOnly))
    {
        qDebug() << "Unable to open download file (READ mode)..";
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


DownloadItem* DownloadManager::addDownload(KIO::CopyJob *job)
{
    KIO::CopyJob *cJob = qobject_cast<KIO::CopyJob *>(job);

    QString downloadFilePath = QStandardPaths::writableLocation(QStandardPaths::DataLocation) + QL1S("/downloads");
    QFile downloadFile(downloadFilePath);
    if (!downloadFile.open(QFile::WriteOnly | QFile::Append))
    {
        qDebug() << "Unable to open download file (WRITE mode)..";
        return 0;
    }
    QDataStream out(&downloadFile);
    out << cJob->srcUrls().at(0).url();
    out << cJob->destUrl().url();
    out << QDateTime::currentDateTime();
    downloadFile.close();
    DownloadItem *item = new DownloadItem(job, QDateTime::currentDateTime(), this);
    m_downloadList.append(item);
    emit newDownloadAdded(item);
    return item;
}


bool DownloadManager::clearDownloadsHistory()
{
    m_downloadList.clear();
    QString downloadFilePath = QStandardPaths::writableLocation(QStandardPaths::DataLocation) + QL1S("/downloads");
    QFile downloadFile(downloadFilePath);
    return downloadFile.remove();
}


void DownloadManager::downloadLinksWithKGet(const QVariant &contentList)
{
    if (!QDBusConnection::sessionBus().interface()->isServiceRegistered( QL1S("org.kde.kget") ))
    {
        KToolInvocation::kdeinitExecWait( QL1S("kget") );
    }
    QDBusInterface kget( QL1S("org.kde.kget"), QL1S("/KGet"), QL1S("org.kde.kget.main") );
    if (kget.isValid())
    {
        kget.call( QL1S("importLinks"), contentList);
    }
}


void DownloadManager::removeDownloadItem(int index)
{
    DownloadItem *item = m_downloadList.takeAt(index);
    delete item;

    m_needToSave = true;
}


// NOTE
// These 2 functions have been copied from the KWebPage class to implement a local version of the downloadResponse method.
// In this way, we can easily provide the extra functionality we need:
// 1. KGet Integration
// 2. Save downloads history
bool DownloadManager::downloadResource(const QUrl &srcUrl, const KIO::MetaData &metaData,
                                       QWidget *parent, bool forceDirRequest, const QString &suggestedName, bool registerDownload)
{
    // manage downloads with KGet if found
    if (ReKonfig::kgetDownload() && !QStandardPaths::findExecutable( QL1S("kget") ).isNull())
    {
        //KGet integration:
        if (!QDBusConnection::sessionBus().interface()->isServiceRegistered( QL1S("org.kde.kget") ))
        {
            KToolInvocation::kdeinitExecWait( QL1S("kget") );
        }
        QDBusInterface kget( QL1S("org.kde.kget"), QL1S("/KGet"), QL1S("org.kde.kget.main") );
        if (!kget.isValid())
            return false;

        QDBusMessage transfer = kget.call(QL1S("addTransfer"), srcUrl.url(), QString(), true);

        return true;
    }

    QUrl destUrl;

    const QString fileName((suggestedName.isEmpty() ? srcUrl.fileName() : suggestedName));

    if (forceDirRequest || ReKonfig::askDownloadPath())
    {
        destUrl = QFileDialog::getSaveFileUrl(parent);
    }
    else
    {
        destUrl = QUrl(ReKonfig::downloadPath().path() + QL1C('/') + fileName);
    }

    qDebug() << "DEST URL: " << destUrl;

    if (!destUrl.isValid())
        return false;

    KIO::CopyJob *job = KIO::copy(srcUrl, destUrl);

    if (!metaData.isEmpty())
        job->setMetaData(metaData);

    job->addMetaData(QL1S("MaxCacheSize"), QL1S("0"));      // Don't store in http cache.
    job->addMetaData(QL1S("cache"), QL1S("cache"));         // Use entry from cache if available.
    KIO::JobUiDelegate *delegate = qobject_cast<KIO::JobUiDelegate *>(job->ui());
    delegate->setWindow((parent ? parent->window() : 0));
    delegate->setAutoErrorHandlingEnabled(true);

    if (registerDownload)
        addDownload(job);

    return true;
}
