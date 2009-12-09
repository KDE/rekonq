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


// Self Includes
#include "adblockmanager.h"
#include "adblockmanager.moc"

// Local Includes
#include "adblocknetworkreply.h"

// KDE Includes
#include <KSharedConfig>
#include <KConfigGroup>
#include <KDebug>

// Qt Includes
#include <QUrl>


AdBlockManager::AdBlockManager(QObject *parent)
    : QObject(parent)
    , _isAdblockEnabled(false)
    , _isHideAdsEnabled(false)
{
    loadSettings();
}


AdBlockManager::~AdBlockManager()
{
}


void AdBlockManager::loadSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig("khtmlrc", KConfig::NoGlobals);
    KConfigGroup cg( config, "Filter Settings" );

    if ( cg.exists() )
    {
        _isAdblockEnabled = cg.readEntry("Enabled", false);
        _isHideAdsEnabled = cg.readEntry("Shrink", false);

        filterList.clear();
        
        // no need to load filters if adblock is not enabled :)
        if(!_isAdblockEnabled)
            return;
        
        QMap<QString,QString> entryMap = cg.entryMap();
        QMap<QString,QString>::ConstIterator it;
        for( it = entryMap.constBegin(); it != entryMap.constEnd(); ++it )
        {
            QString name = it.key();
            QString url = it.value();

            if (name.startsWith(QLatin1String("Filter")))
            {
                AdBlockRule filter(url);
                filterList << filter;
            }
        }
    }
}


QNetworkReply *AdBlockManager::block(const QNetworkRequest &request)
{
    if (!_isAdblockEnabled)
        return 0;
    
    // we (ad)block just http traffic
    if(request.url().scheme() != QLatin1String("http"))
        return 0;
    
    QString urlString = request.url().toString();

    foreach(const AdBlockRule &filter, filterList)
    {
        if(filter.match(urlString))
        {
            kDebug() << "****ADBLOCK: Matched: ***********" << urlString;
            AdBlockNetworkReply *reply = new AdBlockNetworkReply(request, urlString, this);
            return reply;        
        }
    }
    return 0;
}
