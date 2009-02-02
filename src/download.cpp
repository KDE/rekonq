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

// local Includes
#include "download.h"
#include "download.moc"

// KDE Includes
#include <KDebug>

// Qt Includes
#include <QFile>
#include <QFileInfo>

Download::Download(const KUrl &srcUrl, const KUrl &destUrl)
  : m_srcUrl(srcUrl),
    m_destUrl(destUrl)
{
    kWarning() << "DownloadFile: " << m_srcUrl.url() << " to dest: " << m_destUrl.url();
    m_copyJob = KIO::get(m_srcUrl);
    connect(m_copyJob, SIGNAL(data(KIO::Job*,const QByteArray &)), SLOT(slotData(KIO::Job*, const QByteArray&)));
    connect(m_copyJob, SIGNAL(result(KJob *)), SLOT(slotResult(KJob *)));
}

Download::~Download()
{
}

void Download::slotData(KIO::Job *job, const QByteArray& data)
{
    Q_UNUSED(job);
    m_data.append(data);
}

void Download::slotResult(KJob * job)
{
    switch (job->error())
    {
        case 0://The download has finished
        {
            kDebug(5001) << "Downloading successfully finished: " << m_destUrl.url();
            QFile destFile(m_destUrl.path());
            int n = 1;
            QString fn = destFile.fileName();
            while( destFile.exists() )
            {
                destFile.setFileName( fn + "." + QString::number(n) );
                n++;
            }
            if ( destFile.open(QIODevice::WriteOnly | QIODevice::Text) )
            {
                destFile.write(m_data);
                destFile.close();
            }
            m_data = 0;
            break;
        }
        case KIO::ERR_FILE_ALREADY_EXIST:
        {
            kWarning() << "ERROR - File already exists";
            m_data = 0;
            break;
        }
        default:
            kWarning() << "We are sorry to say you, that there were errors while downloading :(";
            m_data = 0;
            break;
    }
}
