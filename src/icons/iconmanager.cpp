/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2010-2014 by Andrea Diamantini <adjam7 at gmail dot com>
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
#include <KFileItem>
#include <KStandardDirs>

// Qt Includes
#include <QDir>
#include <QWebSettings>
#include <QUrl>


QWeakPointer<IconManager> IconManager::s_iconManager;


IconManager *IconManager::self()
{
    if (s_iconManager.isNull())
    {
        s_iconManager = new IconManager(qApp);
    }
    return s_iconManager.data();
}


// ----------------------------------------------------------------------------------------------


IconManager::IconManager(QObject *parent)
    : QObject(parent)
{
    _faviconsDir = KStandardDirs::locateLocal("cache" , "favicons/" , true);
    _tempIconsDir = KStandardDirs::locateLocal("tmp", "favicons/", true);
    
    // Use webkit icon database path
    QWebSettings::setIconDatabasePath(_faviconsDir);
}


KIcon IconManager::iconForUrl(const QUrl &url)
{
    // first things first.. avoid infinite loop at startup
    if (url.isEmpty() || (rApp->rekonqWindowList().isEmpty() && rApp->webAppList().isEmpty()))
        return KIcon("text-html");

    QByteArray encodedUrl = url.toEncoded();
    // rekonq icons..
    if (encodedUrl == QByteArray("rekonq:home"))
        return KIcon("go-home");
    if (encodedUrl == QByteArray("rekonq:closedtabs"))
        return KIcon("tab-close");
    if (encodedUrl == QByteArray("rekonq:history"))
        return KIcon("view-history");
    if (encodedUrl == QByteArray("rekonq:bookmarks"))
        return KIcon("bookmarks");
    if (encodedUrl == QByteArray("rekonq:favorites"))
        return KIcon("emblem-favorite");
    if (encodedUrl == QByteArray("rekonq:downloads"))
        return KIcon("download");
    if (encodedUrl == QByteArray("rekonq:tabs"))
        return KIcon("tab-duplicate");

    // TODO: return other mimetype icons
    if (url.isLocalFile())
    {
        KFileItem item(KFileItem::Unknown, KFileItem::Unknown, url);
        QString iconName = item.iconName();
        return KIcon(iconName);
    }

    QIcon icon = QWebSettings::iconForUrl(url);
    if (!icon.isNull())
        return KIcon(icon);

    // Not found icon. Return default one.
    return KIcon("text-html");
}


void IconManager::clearIconCache()
{
    QDir d(_faviconsDir);
    QStringList favicons = d.entryList();
    Q_FOREACH(const QString & fav, favicons)
    {
        d.remove(fav);
    }

    // delete webkit icon cache
    QWebSettings::clearIconDatabase();
}


void IconManager::saveDesktopIconForUrl(const QUrl &u)
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
QString IconManager::iconPathForUrl(const QUrl &url)
{
    // first things first.. avoid infinite loop at startup
    if (url.isEmpty() || rApp->rekonqWindowList().isEmpty())
    {
        QString icon = QL1S("file://") + KGlobal::dirs()->findResource("icon", "oxygen/16x16/mimetypes/text-html.png");
        return icon;
    }

    QByteArray encodedUrl = url.toEncoded();
    // rekonq icons..
    if (encodedUrl == QByteArray("rekonq:home"))
    {
        QString icon = QL1S("file://") + KGlobal::dirs()->findResource("icon", "oxygen/16x16/actions/go-home.png");
        return icon;
    }
    if (encodedUrl == QByteArray("rekonq:closedtabs"))
    {
        QString icon = QL1S("file://") + KGlobal::dirs()->findResource("icon", "oxygen/16x16/actions/tab-close.png");
        return icon;
    }
    if (encodedUrl == QByteArray("rekonq:history"))
    {
        QString icon = QL1S("file://") + KGlobal::dirs()->findResource("icon", "oxygen/16x16/actions/view-history.png");
        return icon;
    }
    if (encodedUrl == QByteArray("rekonq:bookmarks"))
    {
        QString icon = QL1S("file://") + KGlobal::dirs()->findResource("icon", "oxygen/16x16/places/bookmarks.png");
        return icon;
    }
    if (encodedUrl == QByteArray("rekonq:favorites"))
    {
        QString icon = QL1S("file://") + KGlobal::dirs()->findResource("icon", "oxygen/16x16/emblems/emblem-favorite.png");
        return icon;
    }
    if (encodedUrl == QByteArray("rekonq:downloads"))
    {
        QString icon = QL1S("file://") + KGlobal::dirs()->findResource("icon", "oxygen/16x16/actions/download.png");
        return icon;
    }

    if (url.isLocalFile())
    {
        KFileItem item(KFileItem::Unknown, KFileItem::Unknown, url);
        QString iconName = item.iconName();
        QString icon = QString("file://") + KIconLoader::global()->iconPath(iconName, KIconLoader::Small);    
        return icon;
    }

    QIcon ic = QWebSettings::iconForUrl(url);
    if (!ic.isNull())
    {
        QPixmap px = ic.pixmap(16, 16);
        QString tempIconPath = _tempIconsDir + url.host() + QL1S(".png");
        bool b = px.save(tempIconPath);
        if (b)
            return QL1S("file://") + tempIconPath;
    }

    // Not found icon. Return default one.
    QString icon = QL1S("file://") + KGlobal::dirs()->findResource("icon", "oxygen/16x16/mimetypes/text-html.png");
    return icon;
}


KIcon IconManager::engineFavicon(const QUrl &url)
{
    QString h = url.host();
    if (QFile::exists(_faviconsDir + h + QL1S(".png")))
    {
        _engineFaviconHosts.removeAll(h);
        return KIcon(QIcon(_faviconsDir + h + QL1S(".png")));
    }

    // if engine favicon is NOT found, download it
    // will autodelete itself when done
    if (!_engineFaviconHosts.contains(h))
    {
        _engineFaviconHosts << h;
        new WebIcon(url);
    }

    qDebug() << "NO ENGINE FAVICON";
    return KIcon("text-html");
}
