/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2010-2011 by Andrea Diamantini <adjam7 at gmail dot com>
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
#include "iconmanager.h"
#include "iconmanager.moc"

// Local Includes
#include "application.h"
#include "webicon.h"

// KDE Includes
#include <KIO/Job>

#include <KIcon>
#include <KStandardDirs>
#include <KUrl>

// Qt Includes
#include <QtCore/QDir>

#include <QtWebKit/QWebElement>
#include <QtWebKit/QWebFrame>
#include <QtWebKit/QWebSettings>


IconManager::IconManager(QObject *parent)
    : QObject(parent)
{
    _faviconsDir = KStandardDirs::locateLocal("cache" , "favicons/" , true);
}


KIcon IconManager::iconForUrl(const KUrl &url)
{
    // first things first.. avoid infinite loop at startup
    if (url.isEmpty() || rApp->mainWindowList().isEmpty())
        return KIcon("text-html");

    QByteArray encodedUrl = url.toEncoded();
    // rekonq icons..
    if (encodedUrl == QByteArray("about:home"))
        return KIcon("go-home");
    if (encodedUrl == QByteArray("about:closedTabs"))
        return KIcon("tab-close");
    if (encodedUrl == QByteArray("about:history"))
        return KIcon("view-history");
    if (encodedUrl == QByteArray("about:bookmarks"))
        return KIcon("bookmarks");
    if (encodedUrl == QByteArray("about:favorites"))
        return KIcon("emblem-favorite");
    if (encodedUrl == QByteArray("about:downloads"))
        return KIcon("download");

    // TODO: return other mimetype icons
    if (url.isLocalFile())
    {
        return KIcon("folder");
    }

    QString i = favIconForUrl(url);
    if (!i.isEmpty())
    {
        return KIcon(QIcon(_faviconsDir + i));
    }

    // Not found icon. Return default one.
    return KIcon("text-html");
}


void IconManager::provideIcon(QWebPage *page, const KUrl &url, bool notify)
{
    // provide icons just for http/https sites
    if (!url.scheme().startsWith(QL1S("http")))
    {
        if (notify)
            emit iconChanged();
        return;
    }

    // do not load new icons in private browsing..
    if (QWebSettings::globalSettings()->testAttribute(QWebSettings::PrivateBrowsingEnabled))
    {
        if (notify)
            emit iconChanged();
        return;
    }

    // check if icon exists
    if (!favIconForUrl(url).isEmpty())
    {
        if (notify)
            emit iconChanged();
        return;
    }

    // the simplest way..
    const QString rootUrlString = url.scheme() + QL1S("://") + url.host();

    // find favicon url
    KUrl faviconUrl;

    QWebElement root = page->mainFrame()->documentElement();
    QWebElement e = root.findFirst(QL1S("link[rel~=\"icon\"]"));
    QString relUrlString = e.attribute(QL1S("href"));
    if (relUrlString.isEmpty())
    {
        e = root.findFirst(QL1S("link[rel~=\"shortcut icon\"]"));
        relUrlString = e.attribute(QL1S("href"));
    }

    if (!relUrlString.isEmpty())
    {
        faviconUrl = relUrlString.startsWith(QL1S("http"))
                     ? KUrl(relUrlString)
                     : KUrl(rootUrlString + QL1C('/') + relUrlString);
    }

    if (faviconUrl.isEmpty())
        return;

    // dest url
    KUrl destUrl(_faviconsDir + url.host());

    // download icon
    KIO::FileCopyJob *job = KIO::file_copy(faviconUrl, destUrl, -1, KIO::HideProgressInfo);
    if (notify)
        connect(job, SIGNAL(result(KJob*)), this, SLOT(notifyLastStuffs(KJob *)));
    else
        connect(job, SIGNAL(result(KJob*)), this, SLOT(doLastStuffs(KJob *)));
}


void IconManager::downloadIconFromUrl(const KUrl &url)
{
    new WebIcon(url, this);
}


void IconManager::clearIconCache()
{
    QDir d(_faviconsDir);
    QStringList favicons = d.entryList();
    Q_FOREACH(const QString & fav, favicons)
    {
        d.remove(fav);
    }
}


void IconManager::doLastStuffs(KJob *j)
{
    if (j->error())
    {
        kDebug() << "FAVICON JOB ERROR";
        return;
    }

    KIO::FileCopyJob *job = static_cast<KIO::FileCopyJob *>(j);
    KUrl dest = job->destUrl();

    QString s = dest.url().remove(QL1S("file://"));
    QFile fav(s);
    if (!fav.exists())
    {
        kDebug() << "FAVICON DOES NOT EXISTS";
        fav.remove();
        return;
    }

    if (fav.size() == 0)
    {
        kDebug() << "SIZE ZERO FAVICON";
        fav.remove();
        return;
    }

    QPixmap px;
    if (!px.load(s))
    {
        kDebug() << "PIXMAP NOT LOADED";
        return;
    }

    if (px.isNull())
    {
        kDebug() << "PIXMAP IS NULL";
        fav.remove();
        return;
    }

    px = px.scaled(16, 16);
    if (!px.save(s + QL1S(".png"), "PNG"))
    {
        kDebug() << "PIXMAP NOT SAVED";
        return;
    }
    QFile::remove(s);
}


void IconManager::notifyLastStuffs(KJob *j)
{
    doLastStuffs(j);
    emit iconChanged();
}


void IconManager::saveDesktopIconForUrl(const KUrl &u)
{
    KIcon icon = iconForUrl(u);
    QString destPath = _faviconsDir + u.host() + QL1S("_WEBAPPICON.png");

    QPixmap pix = icon.pixmap(16, 16);
    int s = KIconLoader::global()->currentSize(KIconLoader::Desktop);
    pix = pix.scaled(s, s);

    pix.save(destPath);
}


// NOTE: this function is builded "around" the iconForurl one. It basically returns the same things
// with an important difference: this one returns paths while the other one returns KIcons
QString IconManager::iconPathForUrl(const KUrl &url)
{
    // first things first.. avoid infinite loop at startup
    if (url.isEmpty() || rApp->mainWindowList().isEmpty())
    {
        QString icon = QL1S("file://") + KGlobal::dirs()->findResource("icon", "oxygen/16x16/mimetypes/text-html.png");
        return icon;
    }

    QByteArray encodedUrl = url.toEncoded();
    // rekonq icons..
    if (encodedUrl == QByteArray("about:home"))
    {
        QString icon = QL1S("file://") + KGlobal::dirs()->findResource("icon", "oxygen/16x16/actions/go-home.png");
        return icon;
    }
    if (encodedUrl == QByteArray("about:closedTabs"))
    {
        QString icon = QL1S("file://") + KGlobal::dirs()->findResource("icon", "oxygen/16x16/actions/tab-close.png");
        return icon;
    }
    if (encodedUrl == QByteArray("about:history"))
    {
        QString icon = QL1S("file://") + KGlobal::dirs()->findResource("icon", "oxygen/16x16/actions/view-history.png");
        return icon;
    }
    if (encodedUrl == QByteArray("about:bookmarks"))
    {
        QString icon = QL1S("file://") + KGlobal::dirs()->findResource("icon", "oxygen/16x16/places/bookmarks.png");
        return icon;
    }
    if (encodedUrl == QByteArray("about:favorites"))
    {
        QString icon = QL1S("file://") + KGlobal::dirs()->findResource("icon", "oxygen/16x16/emblems/emblem-favorite.png");
        return icon;
    }
    if (encodedUrl == QByteArray("about:downloads"))
    {
        QString icon = QL1S("file://") + KGlobal::dirs()->findResource("icon", "oxygen/16x16/actions/download.png");
        return icon;
    }

    // TODO: return other mimetype icons
    if (url.isLocalFile())
    {
        QString icon = QL1S("file://") + KGlobal::dirs()->findResource("icon", "oxygen/16x16/places/folder.png");
        return icon;
    }

    QString i = favIconForUrl(url);
    if (!i.isEmpty())
    {
        return QL1S("file://") + _faviconsDir + i;
    }

    // Not found icon. Return default one.
    QString icon = QL1S("file://") + KGlobal::dirs()->findResource("icon", "oxygen/16x16/mimetypes/text-html.png");
    return icon;
}


QString IconManager::favIconForUrl(const KUrl &url)
{
    if (url.isLocalFile()
            || !url.protocol().startsWith(QL1S("http")))
        return QString();

    if (QFile::exists(_faviconsDir + url.host() + QL1S(".png")))
        return url.host() + QL1S(".png");
    else
        return QString();
}
