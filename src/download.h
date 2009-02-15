/* ============================================================
 *
 * This file is a part of the rekonq project
 *
 * Copyright (C) 2007 Lukas Appelhans <l.appelhans@gmx.de>
 * Copyright (C) 2008 by Andrea Diamantini <adjam7 at gmail dot com>
 *
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef DOWNLOAD_H
#define DOWNLOAD_H

// KDE Includes
#include <KUrl>
#include <kio/job.h>

// Qt Includes
#include <QObject>
#include <QByteArray>

/*
 * This class lets rekonq to download an object from the network.
 * Creating a new object, you can continue downloading a file also 
 * when rekonq is closed.
 */ 
class Download : public QObject
{
    Q_OBJECT
    public:
        Download(const KUrl &srcUrl, const KUrl &destUrl);
        ~Download();

    private slots:
        void slotResult(KJob * job);
        void slotData(KIO::Job *job, const QByteArray& data);

    private:
        KIO::TransferJob *m_copyJob;
        KUrl m_srcUrl;
        KUrl m_destUrl;
        KUrl m_destFile;
        QByteArray m_data;
};

#endif
