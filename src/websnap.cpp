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


#include "websnap.h"
#include "websnap.moc"

#include <KDebug>
#include <KStandardDirs>

#include <QSize>
#include <QWebFrame>
#include <QWebSettings>
#include <QPainter>
#include <QTimer>
#include <QFile>


#define WIDTH  200
#define HEIGHT 150


WebSnap::WebSnap(const QString &url, const QString &pos)
    : QObject()
{
    m_url = url;
    m_pos = pos;

    // this to not register websnap history
    m_page.settings()->setAttribute(QWebSettings::PrivateBrowsingEnabled, true);
    connect(&m_page, SIGNAL(loadFinished(bool)), this, SLOT(saveResult(bool)));
    QTimer::singleShot(0, this, SLOT(load()));
}


void WebSnap::load()
{
    m_page.mainFrame()->load( QUrl(m_url) );
}

QPixmap WebSnap::renderPreview(const QWebPage &page,int w, int h)
{
    // prepare page
    page.mainFrame()->setScrollBarPolicy(Qt::Vertical, Qt::ScrollBarAlwaysOff); //Why it doesn't work with one setScrollBarPolicy ? bug in qtwebkit ?
    page.mainFrame()->setScrollBarPolicy(Qt::Vertical, Qt::ScrollBarAlwaysOff);
    page.mainFrame()->setScrollBarPolicy(Qt::Horizontal, Qt::ScrollBarAlwaysOff);
    page.mainFrame()->setScrollBarPolicy(Qt::Horizontal, Qt::ScrollBarAlwaysOff);

    // find the best size
    QSize size;
    if (page.viewportSize().width() && page.viewportSize().height())
    {
        size = page.viewportSize();
    }
    else
    {
        int width = page.mainFrame()->contentsSize().width();
        if (width < 640) width = 640;
        size = QSize(width,width*((0.0+h)/w));
        page.setViewportSize(size);
    }

    // create the target surface
    QPixmap image = QPixmap(size);
    image.fill(Qt::transparent);
 
    // render
    QPainter p(&image);
    page.mainFrame()->render(&p);
    p.end();
    image = image.scaled(w, h, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);

    // restore page settings
    page.mainFrame()->setScrollBarPolicy(Qt::Horizontal, Qt::ScrollBarAsNeeded);
    page.mainFrame()->setScrollBarPolicy(Qt::Horizontal, Qt::ScrollBarAsNeeded);
    page.mainFrame()->setScrollBarPolicy(Qt::Vertical, Qt::ScrollBarAsNeeded);
    page.mainFrame()->setScrollBarPolicy(Qt::Vertical, Qt::ScrollBarAsNeeded);

    return image;
}

void WebSnap::saveResult(bool ok)
{
    // crude error-checking
    if (!ok) 
    {
        kDebug() << "Error loading site..";
        return;
    }

    QString path = KStandardDirs::locateLocal("cache", QString("thumbs/rek") + m_pos + ".png", true);
    m_image = renderPreview(m_page, WIDTH, HEIGHT);
    if(m_image.save(path))
    {
        kDebug() << "finished";
        emit finished();
    }
}


QPixmap WebSnap::previewImage()
{
    return m_image;
}
