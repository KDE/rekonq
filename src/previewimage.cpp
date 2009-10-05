/* ============================================================
*
* This file is a part of the rekonq project
*
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

#include "previewimage.h"
#include "previewimage.moc"

#include "application.h"

#include <QFile>
#include <QMovie>
#include <QMouseEvent>

#include <KUrl>
#include <KStandardDirs>
#include <KDebug>


PreviewImage::PreviewImage(const QUrl &url)
    : QLabel()
    , ws(0)
    , m_url(url)
{   
    m_savePath = KStandardDirs::locateLocal("cache", QString("thumbs/") + guessNameFromUrl(m_url) + ".png", true);
    
    if(QFile::exists(m_savePath))
    {
        m_pixmap.load(m_savePath);
        setPixmap( m_pixmap );
    }
    else
    {
        ws = new WebSnap( url );
        connect(ws, SIGNAL(finished()), this, SLOT(setSiteImage()));
        
        QString path = KStandardDirs::locate("appdata", "pics/busywidget.gif");

        QMovie *movie = new QMovie(path, QByteArray(), this);
        movie->setSpeed(50);
        setMovie(movie);
        movie->start();
    }
}


PreviewImage::~PreviewImage()
{
}


void PreviewImage::setSiteImage()
{
    QMovie *m = movie();
    delete m;
    setMovie(0);
    
    m_pixmap = ws->previewImage();
    setPixmap(m_pixmap);

    m_pixmap.save(m_savePath);
}


void PreviewImage::mousePressEvent(QMouseEvent *event)
{
    switch(event->button())
    {
    case Qt::LeftButton:
        Application::instance()->loadUrl(m_url);
        break;
    case Qt::RightButton:
        // TODO
        break;
    default:
        QLabel::mousePressEvent(event);
    };
}


QString PreviewImage::guessNameFromUrl(QUrl url)
{
    QString name = url.toString( QUrl::RemoveScheme | QUrl::RemoveUserInfo | QUrl::StripTrailingSlash );

    // TODO learn Regular Expressions :)
    // and implement something better here..
    name.remove('/');
    name.remove('&');
    name.remove('.');
    name.remove('-');
    name.remove('_');
    
    return name;
}
