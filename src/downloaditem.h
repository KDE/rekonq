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


#ifndef DOWNLOADITEM_H
#define DOWNLOADITEM_H


// Rekonq Includes
#include "rekonq_defines.h"

// Qt Includes
#include <QDateTime>
#include <QObject>
#include <QString>

// KDE Includes
#include <KLocalizedString>
#include <KUrl>
#include <KIO/CopyJob>


class DownloadItem : public QObject
{
    Q_OBJECT

public:

    enum JobState
    {
        Done            = 0,
        Downloading     = 1,
        Errors          = 2,
        Suspended       = 3,
        KGetManaged     = 4
    };

    explicit DownloadItem(const QString &srcUrl, const QString &destUrl, const QDateTime &d, QObject *parent = 0);

    // This is used to add a DownloadItem managed with KIO
    explicit DownloadItem(KIO::CopyJob *job, const QDateTime &d, QObject *parent = 0);


    inline QDateTime dateTime() const
    {
        return m_dateTime;
    }

    KUrl destUrl() const;

    QString originUrl() const;
    QString destinationUrlString() const;
    QString fileName() const;
    QString fileDirectory() const;
    QString icon() const;
    QString errorString() const;

    inline int state() const
    {
        return m_state;
    }

    void setIsKGetDownload();


Q_SIGNALS:
    void downloadProgress(int percent);
    void downloadFinished(bool success);

public Q_SLOTS:
    void updateProgress(KJob *job, unsigned long value);
    void onFinished(KJob *job);
    void onSuspended(KJob*);

private:
    QString m_srcUrlString;
    KUrl m_destUrl;

    QDateTime m_dateTime;

    KIO::CopyJob *m_job;
    int m_state;

    QString m_errorString;
};


Q_DECLARE_METATYPE(DownloadItem*)


typedef QList<DownloadItem*> DownloadList;


#endif //DOWNLAODITEM_H
