/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2009 Nokia Corporation <qt-info@nokia.com>
* Copyright (C) 2009 by Andrea Diamantini <adjam7 at gmail dot com>
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

#ifndef WEB_SNAP_H
#define WEB_SNAP_H


#include <KUrl>

#include <QtCore/QObject>
#include <QPixmap>
#include <QWebPage>


/**
 * This class renders a site producing an image based
 * on that.
 * Heavily based on Graphics-Dojo WebSnap example (thanks!)
 */
class WebSnap : public QObject
{
    Q_OBJECT

public:
    WebSnap(const QString &url, const QString &pos);
    
    QPixmap previewImage();
    static QPixmap renderPreview(const QWebPage &page, int w, int h);

signals:
    void finished();

private slots:
    void load();
    void saveResult(bool ok);

private:
    QWebPage m_page;
    QPixmap m_image;

    QString m_url;
    QString m_pos;
};

#endif // WEB_SNAP_H
