/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2007 Lukas Appelhans <l.appelhans@gmx.de>
* Copyright (C) 2008-2009 by Andrea Diamantini <adjam7 at gmail dot com>
* Copyright (C) 2009 by Paweł Prażak <pawelprazak at gmail dot com>
* Copyright (C) 2009 by Domrachev Alexandr <alexandr.domrachev@gmail.com>
*
*
* This program is free software; you can redistribute it
* and/or modify it under the terms of the GNU General
* Public License as published by the Free Software Foundation;
* either version 2, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* ============================================================ */


// local Includes
#include "download.h"
#include "download.moc"

// KDE Includes
#include <KDebug>
#include <KFileDialog>
#include <KGlobalSettings>
#include <KMessageBox>
#include <KMimeType>
#include <KStandardDirs>

// Qt Includes
#include <QFile>
#include <QFileInfo>

// Local Includes
#include "application.h"
#include "mainwindow.h"


DownloadManager::DownloadManager()
        : QObject()
{
}


DownloadManager::~DownloadManager()
{
    foreach(Download *download, m_downloads)
    {
        // cancel all unfinished downloads
        download->cancel();
        delete download;
    }
}


void DownloadManager::newDownload(const KUrl &srcUrl, const KUrl &destUrl)
{
    KUrl destination = destUrl;
    Download::DownloadType type;

    KSharedPtr<KMimeType> mimeType = KMimeType::findByPath(srcUrl.prettyUrl());

    QString typeText = KMimeType::extractKnownExtension(srcUrl.fileName());
    typeText += " (" + mimeType->name() + ")";

    int answer = KMessageBox::questionYesNoCancel(NULL,
                 i18n("Download '%1'?\n""Type: %2", srcUrl.prettyUrl(), typeText),
                 i18n("Download '%1'...", srcUrl.fileName()),
                 KStandardGuiItem::save(),
                 KStandardGuiItem::open(),
                 KStandardGuiItem::cancel(),
                 "showOpenSaveDownloadDialog"
                                                 );

    switch (answer)
    {
    case KMessageBox::Cancel:
        return;
        break;

    case KMessageBox::Yes:  // ----- SAVE
        // if destination is empty than ask for download path (if showOpenSaveDownloadDialog setting enabled)
        if (destination.isEmpty())
        {
            destination = downloadDestination(srcUrl.fileName());
        }
        type = Download::Save;
        break;

    case KMessageBox::No:   // ----- OPEN
        // Download file to tmp dir
        destination.setDirectory(KStandardDirs::locateLocal("tmp", "", true));
        destination.addPath(srcUrl.fileName());
        type = Download::Open;
        break;

    default:
        // impossible
        break;
    };

    // if user canceled download than abort
    if (destination.isEmpty())
        return;

    Download *download = new Download(srcUrl, destination, type);
    connect(download, SIGNAL(downloadFinished(int)), this, SLOT(slotDownloadFinished(int)));
    m_downloads.append(download);
}


const QList<Download *> &DownloadManager::downloads() const
{
    return m_downloads;
}


KUrl DownloadManager::downloadDestination(const QString &filename)
{
    KUrl destination = ReKonfig::downloadDir();
    if (destination.isEmpty())
        destination = KGlobalSettings::downloadPath();
    destination.addPath(filename);

    if (!ReKonfig::downloadToDefaultDir())
    {
        destination = KFileDialog::getSaveUrl(destination);
        // if user canceled the download return empty url
        if (destination.isEmpty())
            return KUrl();
    }
    return destination;
}


void DownloadManager::slotDownloadFinished(int errorCode)
{
    Q_UNUSED(errorCode)

    // if sender exists and list contains it - (open and) delete it
    Download *download = qobject_cast<Download *>(sender());
    if (download && m_downloads.contains(download))
    {
        if (download->type() == Download::Open)
        {
            KSharedPtr<KMimeType> mimeType = KMimeType::findByPath(download->destUrl().prettyUrl());
            KRun::runUrl(download->destUrl(), mimeType->name(), NULL, true);
        }
        disconnect(download, SIGNAL(downloadFinished(int)), this, SLOT(slotDownloadFinished(int)));
        int index = m_downloads.indexOf(download);
        delete m_downloads.takeAt(index);
        return;
    }
    kWarning() << "Could not find download or invalid sender. Sender:" << sender();
}


//----

#include <KJob>
#include <KIO/Job>
#include <KIO/CopyJob>


Download::Download(const KUrl &srcUrl, const KUrl &destUrl, DownloadType type)
        : QObject()
        , m_srcUrl(srcUrl)
        , m_destUrl(destUrl)
        , m_type(type)
{
    Q_ASSERT(!m_srcUrl.isEmpty());
    Q_ASSERT(!m_destUrl.isEmpty());
    kDebug() << "DownloadFile: " << m_srcUrl.url() << " to dest: " << m_destUrl.url();

    m_copyJob = KIO::file_copy(m_srcUrl, m_destUrl);
    connect(m_copyJob, SIGNAL(result(KJob *)), SLOT(slotResult(KJob *)));
}


Download::~Download()
{
}


void Download::cancel()
{
    bool result = m_copyJob->kill(KJob::EmitResult);
    Q_ASSERT(result);
}


void Download::slotResult(KJob *job)
{
    switch (job->error())
    {
    case 0:  //The download has finished
    {
        kDebug() << "Downloading successfully finished: " << m_destUrl.url();
        break;
    }
    case KIO::ERR_FILE_ALREADY_EXIST:
    {
        kWarning() << "ERROR - File already exists";
        break;
    }
    case KIO::ERR_USER_CANCELED:
    {
        kWarning() << "ERROR - User canceled the downlaod";
        break;
    }
    default:
        kWarning() << "We are sorry to say you, that there were errors while downloading :(";
        break;
    }

    // inform the world
    emit downloadFinished(job->error());
}
