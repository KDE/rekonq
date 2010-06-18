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
#include <KStandardDirs>

// Qt Includes
#include <QtCore/QSize>
#include <QtCore/QTimer>
#include <QtCore/QFile>

#include <QtGui/QPainter>
#include <QtGui/QAction>

#include <QtWebKit/QWebFrame>
#include <QtWebKit/QWebSettings>


WebSnap::WebSnap(const KUrl& url, QObject *parent)
        : QObject(parent)
        , m_url(url)
{
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
    kDebug() << "oh oh..";
    m_page.action(QWebPage::Stop)->trigger();
    m_page.deleteLater();
}


void WebSnap::load()
{
    m_page.mainFrame()->load(m_url);
}


// NOTE please, be careful modifying this.
// You are playing with fire..
QPixmap WebSnap::renderPreview(const QWebPage &page, int w, int h, bool save)
{
    // prepare page
    QSize oldSize = page.viewportSize();

    // find the best size
    QSize size;
    int width = page.mainFrame()->contentsSize().width();
    if (width < 640)
    {
        width = 640;
    }
    size = QSize(width, width * ((0.0 + h) / w));
    page.setViewportSize(size);

    // create the page image
    QImage pageImage = QImage(size, QImage::Format_ARGB32_Premultiplied);
    pageImage.fill(Qt::transparent);

    // render it
    QPainter p(&pageImage);
    page.mainFrame()->render(&p, QWebFrame::ContentsLayer);
    p.end();
    pageImage = pageImage.scaled(w, h, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);

    // restore page settings
    page.setViewportSize(oldSize);

    QPixmap pm = QPixmap::fromImage(pageImage);
    
    if(save)
    {
        KUrl url(page.mainFrame()->url());
        kDebug() << "saving preview";
        QString path = imagePathFromUrl(url);
        QFile::remove(path);
        pm.save(path);
    }
    
    return pm;
}


QString WebSnap::imagePathFromUrl(const KUrl &url)
{
    QUrl temp = QUrl(url.url());
    QString name = temp.toString(QUrl::RemoveScheme | QUrl::RemoveUserInfo | QUrl::StripTrailingSlash);

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

    return KStandardDirs::locateLocal("cache", QString("thumbs/") + name + ".png", true);
}


void WebSnap::saveResult(bool ok)
{
    if (ok)
    {
        QPixmap image = renderPreview(m_page, WIDTH, HEIGHT);
        QString path = imagePathFromUrl(m_url);
        QFile::remove(path);
        image.save(path);
    }

    emit snapDone(ok);
    kDebug() << "SAVE RESULTS: " << ok << " URL: " << m_url;

    this->deleteLater();
}


bool WebSnap::existsImage(const KUrl &u)
{
    return QFile::exists(imagePathFromUrl(u));
}
