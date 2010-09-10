/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2010 by Andrea Diamantini <adjam7 at gmail dot com>
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
#include <KMimeType>
#include <KStandardDirs>
#include <KUrl>

// Qt Includes
#include <QtWebKit/QWebElement>
#include <QtWebKit/QWebFrame>
#include <QtWebKit/QWebSettings>


IconManager::IconManager(QObject *parent)
    : QObject(parent)
{
}


IconManager::~IconManager()
{
}


KIcon IconManager::iconForUrl(const KUrl &url)
{
    // first things first.. avoid infinite loop at startup
    if (url.isEmpty() || Application::instance()->mainWindowList().isEmpty())
        return KIcon("text-html");

    // no icons in private browsing..
    if(QWebSettings::globalSettings()->testAttribute(QWebSettings::PrivateBrowsingEnabled))
        return KIcon("view-media-artist");
    
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

    QString i = KMimeType::favIconForUrl(url);
    QString faviconDir = KStandardDirs::locateLocal("cache" , "" , true);
    if(!i.isEmpty())
    {
        return KIcon(faviconDir + i);
    }
    kDebug() << "Icon NOT Found. returning text-html one";

    // TODO: return other mimetype icons
    if(url.isLocalFile())
    {
        return KIcon("folder");
    }
    return KIcon("text-html");
}


void IconManager::provideIcon(QWebPage *page, const KUrl &url, bool notify)
{
    // provide icons just for http/https sites
    if( !url.scheme().startsWith(QL1S("http")) )
    {
        kDebug() << "No http/https site...";
        if(notify)
            emit iconChanged();
        return;
    }

    // no icons in private browsing..
    if(QWebSettings::globalSettings()->testAttribute(QWebSettings::PrivateBrowsingEnabled))
    {
        kDebug() << "Private browsing, private icon...";
        if(notify)
            emit iconChanged();
        return;
    }
    
    QUrl u(url.url());
    QString rootUrlString = u.toString(  QUrl::RemovePassword
                                       | QUrl::RemoveUserInfo
                                       | QUrl::RemovePath
                                       | QUrl::RemoveQuery
                                       | QUrl::StripTrailingSlash);

    // check if icon exists
    if(!KMimeType::favIconForUrl(url).isEmpty())
    {
        kDebug() << "icon JUST present. Aborting...";
        if(notify)
            emit iconChanged();
        return;
    }

    // find ico url
    KUrl iconUrl(rootUrlString + QL1S("/favicon.ico"));

    QWebElement root = page->mainFrame()->documentElement();
    QWebElement e = root.findFirst(QL1S("link[rel~=\"icon\"]"));
    QString relUrlString = e.attribute(QL1S("href"));
    if(relUrlString.isEmpty())
    {
        e = root.findFirst(QL1S("link[rel~=\"shortcut icon\"]"));
        relUrlString = e.attribute(QL1S("href"));
    }

    if(!relUrlString.isEmpty())
    {
        iconUrl = relUrlString.startsWith("http")
                ? KUrl(relUrlString)
                : KUrl(rootUrlString + relUrlString) ;
    }

    kDebug() << "ICON URL: " << iconUrl;

    QString faviconDir = KStandardDirs::locateLocal("cache" , "favicons/" , true);

    int r = rootUrlString.indexOf(':');
    KUrl destUrl(faviconDir + rootUrlString.mid(r+3) + ".png");
    kDebug() << "DEST URL: " << destUrl;

    // download icon
    KIO::FileCopyJob *job = KIO::file_copy(iconUrl, destUrl, -1, KIO::HideProgressInfo);
    if(notify)
        connect(job, SIGNAL(result(KJob*)), this, SIGNAL(iconChanged()));
}


void IconManager::downloadIconFromUrl(const KUrl &url)
{
    new WebIcon(url, this);
}
