/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2009-2014 by Andrea Diamantini <adjam7 at gmail dot com>
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

// Qt Includes
#include <QSize>
#include <QFile>
#include <QStandardPaths>

#include <QCryptographicHash>

#include <QPainter>
#include <QAction>

#include <QWebFrame>
#include <QWebSettings>


WebSnap::WebSnap(const QUrl& url, QObject *parent)
    : QObject(parent)
    , m_url(url)
{
    // this to not register websnap history
    m_page.settings()->setAttribute(QWebSettings::PrivateBrowsingEnabled, true);

    // this to not let this page open other windows
    m_page.settings()->setAttribute(QWebSettings::PluginsEnabled, false);
    m_page.settings()->setAttribute(QWebSettings::JavascriptEnabled, false);

    connect(&m_page, SIGNAL(loadFinished(bool)), this, SLOT(saveResult(bool)));

    QMetaObject::invokeMethod(this, "load", Qt::QueuedConnection);
}


WebSnap::~WebSnap()
{
    m_page.action(QWebPage::Stop)->trigger();
    m_page.deleteLater();
}


void WebSnap::load()
{
    m_page.mainFrame()->load(m_url);
}



QPixmap WebSnap::render(const QWebPage &page, int w, int h)
{
    // create the page image
    QPixmap pageImage = QPixmap(w, h);
    pageImage.fill(Qt::transparent);

    // render it
    QPainter p(&pageImage);
    page.mainFrame()->render(&p, QWebFrame::ContentsLayer);
    p.end();

    return pageImage;
}


// NOTE
// to render page preview in a safe way, you CANNOT work with scrollbars!
// In fact, disabling temporarily them DOES NOT work without reloading a page
// that is something we CANNOT do.
QPixmap WebSnap::renderPagePreview(const QWebPage &page, int w, int h)
{
    // store actual viewportsize
    QSize oldSize = page.viewportSize();

    // prepare page
    // NOTE: I saw some sites with strange CMS and with absurd content size width (eg: 8584553)
    // This usually leads setViewportSize to crash :(
    // So, ensure renderWidth is no more than 2000.
    int renderWidth = page.mainFrame()->contentsSize().width();
    if (renderWidth > 2000)
        renderWidth = 2000;
    int renderHeight = renderWidth * ((0.0 + h) / w);

    page.setViewportSize(QSize(renderWidth, renderHeight));

    // consider scrollbars and render the page
    bool verticalScrollBarActive = !page.mainFrame()->scrollBarGeometry(Qt::Vertical).isEmpty();
    if (verticalScrollBarActive)
        renderWidth -= 15;

    bool horizontalScrollBarActive = !page.mainFrame()->scrollBarGeometry(Qt::Horizontal).isEmpty();
    if (horizontalScrollBarActive)
        renderHeight -= 15;

    QPixmap pageImage = WebSnap::render(page, renderWidth, renderHeight);

    // resize image
    pageImage = pageImage.scaled(w, h, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);

    // restore page state
    page.setViewportSize(oldSize);

    return pageImage;
}


QString WebSnap::imagePathFromUrl(const QUrl &url)
{
    QByteArray name = url.toEncoded(QUrl::RemoveScheme | QUrl::RemoveUserInfo | QUrl::StripTrailingSlash);

    QByteArray hashedName = QCryptographicHash::hash(name, QCryptographicHash::Md5).toHex();

    QString w = QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + QL1C('/') + QL1S("thumbs/") + QL1S(hashedName) + QL1S(".png");
    return w;
}


void WebSnap::saveResult(bool ok)
{
    if (ok)
    {
        QPixmap image = renderPagePreview(m_page, defaultWidth, defaultHeight);
        QString path = imagePathFromUrl(m_url);
        QFile::remove(path);
        image.save(path);
    }

    emit snapDone(ok);

    this->deleteLater();
}


bool WebSnap::existsImage(const QUrl &u)
{
    return QFile::exists(imagePathFromUrl(u));
}
