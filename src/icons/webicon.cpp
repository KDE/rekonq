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
#include "webicon.h"
#include "webicon.moc"

// Local Includes
#include "iconmanager.h"
#include "icondownloader.h"

#include "knetworkaccessmanager.h"

// KDE Includes
#include <KStandardDirs>

// Qt Includes
#include <QTimer>
#include <QWebFrame>
#include <QWebElement>


WebIcon::WebIcon(const KUrl& url, QObject *parent)
    : QObject(parent)
    , m_url(url)
{
    m_page.setNetworkAccessManager(new KNetworkAccessManager);
    
    m_page.settings()->setAttribute(QWebSettings::PluginsEnabled, false);
    m_page.settings()->setAttribute(QWebSettings::JavascriptEnabled, false);
    m_page.settings()->setAttribute(QWebSettings::PrivateBrowsingEnabled, true);

    connect(&m_page, SIGNAL(loadFinished(bool)), this, SLOT(saveIcon(bool)));
    QTimer::singleShot(0, this, SLOT(load()));
}


void WebIcon::load()
{
    m_page.mainFrame()->load(m_url);
}


void WebIcon::saveIcon(bool b)
{
    if (!b)
    {
        this->deleteLater();
        return;
    }

    // the simplest way..
    const QString rootUrlString = m_url.scheme() + QL1S("://") + m_url.host();

    // find favicon url
    KUrl faviconUrl(rootUrlString + QL1S("/favicon.ico"));


    QWebElement root = m_page.mainFrame()->documentElement();
    QWebElement e = root.findFirst(QL1S("link[rel~=\"icon\"]"));
    QString relUrlString = e.attribute(QL1S("href"));
    if (relUrlString.isEmpty())
    {
        e = root.findFirst(QL1S("link[rel~=\"shortcut icon\"]"));
        relUrlString = e.attribute(QL1S("href"));
    }

    // remove eventual initial //
    if (relUrlString.startsWith(QL1S("//")))
    {
        relUrlString.remove(0, 2);
        relUrlString.prepend(QL1S("http://"));
    }
    
    if (!relUrlString.isEmpty())
    {
        faviconUrl = KUrl(relUrlString);
        
        if (!faviconUrl.isValid())
        {            
            faviconUrl = KUrl(rootUrlString + QL1C('/') + relUrlString);
        }
        
        if (faviconUrl.host().isEmpty())
        {
            faviconUrl = KUrl(rootUrlString + relUrlString);           
        }
    }
    
    kDebug() << "FAVICON RETRIEVING URL: " << faviconUrl;
       
    QString faviconsDir = KStandardDirs::locateLocal("cache" , "favicons/" , true);

    // dest url
    KUrl destUrl(faviconsDir + m_url.host());

    // will autodelete itself when done
    new IconDownloader(faviconUrl, destUrl, this);
}
