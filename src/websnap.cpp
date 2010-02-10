/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2009 Nokia Corporation <qt-info@nokia.com>
* Copyright (C) 2009-2010 by Andrea Diamantini <adjam7 at gmail dot com>
* Copyright (C) 2010 by Matthieu Gicquel <matgic78 at gmail dot com>
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
#include "websnap.h"
#include "websnap.moc"

// Local Includes
#include "newtabpage.h"

// KDE Includes
#include <KDebug>
#include <KStandardDirs>

// Qt Includes
#include <QSize>
#include <QWebFrame>
#include <QWebSettings>
#include <QPainter>
#include <QTimer>
#include <QFile>


WebSnap::WebSnap(const QUrl& url, QWebPage* originatingPage, int previewIndex)
    : QObject()
{
    m_url = url;
    m_originatingPage = originatingPage;
    m_previewIndex = previewIndex;

    // this to not register websnap history
    m_page.settings()->setAttribute(QWebSettings::PrivateBrowsingEnabled, true);
    
    // this to not let this page open other windows
    m_page.settings()->setAttribute(QWebSettings::PluginsEnabled, false);
    m_page.settings()->setAttribute(QWebSettings::JavascriptEnabled, false);

    connect(&m_page, SIGNAL(loadFinished(bool)), this, SLOT(saveResult(bool)));
    QTimer::singleShot(0, this, SLOT(load()));
}


WebSnap::~WebSnap()
{
}


void WebSnap::load()
{
    m_page.mainFrame()->load(m_url);
}


QPixmap WebSnap::renderPreview(const QWebPage &page,int w, int h)
{
    // prepare page
    page.mainFrame()->setScrollBarPolicy(Qt::Vertical, Qt::ScrollBarAlwaysOff); // Why it doesn't work with one setScrollBarPolicy?
    page.mainFrame()->setScrollBarPolicy(Qt::Vertical, Qt::ScrollBarAlwaysOff); // bug in qtwebkit ?
    page.mainFrame()->setScrollBarPolicy(Qt::Horizontal, Qt::ScrollBarAlwaysOff);
    page.mainFrame()->setScrollBarPolicy(Qt::Horizontal, Qt::ScrollBarAlwaysOff);

    // find the best size
    QSize size;
    int width = page.mainFrame()->contentsSize().width();
    if (width < 640) width = 640;
    size = QSize(width,width*((0.0+h)/w));
    page.setViewportSize(size);
    
    // create the page image
    QImage pageImage = QImage(size, QImage::Format_ARGB32_Premultiplied);
    pageImage.fill(Qt::transparent); 
    // render it
    QPainter p(&pageImage);
    page.mainFrame()->render(&p);
    p.end();
    pageImage = pageImage.scaled(w, h, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);

    // restore page settings
    page.mainFrame()->setScrollBarPolicy(Qt::Horizontal, Qt::ScrollBarAsNeeded);
    page.mainFrame()->setScrollBarPolicy(Qt::Horizontal, Qt::ScrollBarAsNeeded);
    page.mainFrame()->setScrollBarPolicy(Qt::Vertical, Qt::ScrollBarAsNeeded);
    page.mainFrame()->setScrollBarPolicy(Qt::Vertical, Qt::ScrollBarAsNeeded);

    return QPixmap::fromImage(pageImage);
}


void WebSnap::savePreview(QPixmap pm, KUrl url)
{
    kDebug() << "saving preview";
    QFile::remove(fileForUrl(url).toLocalFile());
    pm.save(fileForUrl(url).toLocalFile());
}


KUrl WebSnap::fileForUrl(KUrl url)
{
    QString filePath = 
            KStandardDirs::locateLocal("cache", QString("thumbs/") + WebSnap::guessNameFromUrl(url) + ".png", true);
    return KUrl(filePath);
}


QString WebSnap::guessNameFromUrl(QUrl url)
{
    QString name = url.toString( QUrl::RemoveScheme | QUrl::RemoveUserInfo | QUrl::StripTrailingSlash );
    
    // TODO learn Regular Expressions :)
    // and implement something better here..
    name.remove('/');
    name.remove('&');
    name.remove('.');
    name.remove('-');
    name.remove('_');
    name.remove('?');
    name.remove('=');
    name.remove('+');
    
    return name;
}


void WebSnap::saveResult(bool ok)
{
    // crude error-checking
    if (!ok) 
    {
        kDebug() << "Error loading site..";
        m_snapTitle = "Error...";
        m_image = QPixmap();
    }
    else
    {
        m_image = renderPreview(m_page, WIDTH, HEIGHT);
        m_snapTitle = m_page.mainFrame()->title();
    }
    QFile::remove(fileForUrl(m_url).toLocalFile());
    m_image.save(fileForUrl(m_url).toLocalFile());
    
    //m_originatingPage->mainFrame()->load(KUrl("about:preview/replace/" + QVariant(m_previewIndex).toString()));
    NewTabPage p(m_originatingPage->mainFrame());
    p.snapFinished(m_previewIndex, m_url, m_snapTitle);
    
    deleteLater();
}



QString WebSnap::snapTitle()
{
    return m_page.mainFrame()->title();
}


QUrl WebSnap::snapUrl()
{
    return m_url;
}


QPixmap WebSnap::previewImage()
{
    return m_image;
}
