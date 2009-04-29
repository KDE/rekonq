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


#ifndef DOWNLOAD_H
#define DOWNLOAD_H

// Auto Includes
#include "rekonq.h"

// KDE Includes
#include <KIO/FileCopyJob>

// Qt Includes
#include <QObject>

// Forward Declarations
class KJob;

namespace KIO
{
class Job;
}


/**
 * This class lets rekonq to download an object from the network.
 * Creating a new object, you can continue downloading a file also
 * when rekonq is closed.
 *
 */
class Download : public QObject
{
    Q_OBJECT

public:
    enum DownloadType { Save, Open };

    /**
     * Class constructor. This is the unique method we need to
     * use this class. In fact Download class needs to know just
     * "where" catch the file to download and where it has to put it
     *
     * @param srcUrl the source url
     * @param destUrl the destination url
     *
     */
    Download(const KUrl &srcUrl, const KUrl &destUrl, DownloadType type);

    /**
     * class destructor
     */
    ~Download();

    KUrl srcUrl() const
    {
        return m_srcUrl;
    }
    KUrl destUrl() const
    {
        return m_destUrl;
    }
    DownloadType type() const
    {
        return m_type;
    }
    void cancel();

signals:
    void downloadFinished(int errorCode);

private slots:
    void slotResult(KJob *job);

private:
    KIO::FileCopyJob *m_copyJob;
    KUrl m_srcUrl;
    KUrl m_destUrl;
    KUrl m_destFile;
    QByteArray m_data;
    DownloadType m_type;
};


// ----------------------


class DownloadManager : public QObject
{
    Q_OBJECT

public:
    DownloadManager();
    ~DownloadManager();

    /**
    * @short Creates new download job.
    * This method lets you to download a file from a remote source url
    * to a local destination url.
    *
    * @param srcUrl the source url
    * @param destUrl the destination url (default value is your default download destination setting)
    *
    */
    void newDownload(const KUrl &srcUrl, const KUrl &destUrl = KUrl());

    const QList<Download *> &downloads() const;

public slots:
    void slotDownloadFinished(int errorCode);

private:
    KUrl downloadDestination(const QString &filename);

    QList<Download *> m_downloads;
};


//--


#endif
