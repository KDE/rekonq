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
#include "webpage.h"

// KDE Includes
#include <KSharedConfig>
#include <KConfigGroup>
#include <KDebug>

// Qt Includes
#include <QUrl>
#include <QWebElement>


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

        // no need to load filters if adblock is not enabled :)
        if(!_isAdblockEnabled)
            return;

        _whiteList.clear();
        _blackList.clear();
        _hideList.clear();
        
        QMap<QString,QString> entryMap = cg.entryMap();
        QMap<QString,QString>::ConstIterator it;
        for( it = entryMap.constBegin(); it != entryMap.constEnd(); ++it )
        {
            QString name = it.key();
            QString url = it.value();

            if (name.startsWith(QLatin1String("Filter")))
            {
                if(!url.startsWith("!"))
                {           
                    // white rules
                    if(url.startsWith("@@"))
                    {
                        AdBlockRule rule( url.mid(2) );
                        _whiteList << rule;
                        continue;
                    }
                    
                    // hide (CSS) rules
                    if(url.startsWith("##"))
                    {
                        _hideList << url.mid(2);
                        continue;
                    }

                    AdBlockRule rule( url );
                    _blackList << rule;
                }
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

    // check white rules before :)
    foreach(const AdBlockRule &filter, _whiteList)
    {
        if(filter.match(urlString))
        {
            kDebug() << "****ADBLOCK: WHITE RULE (@@) Matched: ***********" << urlString;
            return 0;        
        }
    }
    
    // then check the black ones :(
    foreach(const AdBlockRule &filter, _blackList)
    {
        if(filter.match(urlString))
        {
            kDebug() << "****ADBLOCK: BLACK RULE Matched: ***********" << urlString;
            AdBlockNetworkReply *reply = new AdBlockNetworkReply(request, urlString, this);
            return reply;        
        }
    }
    
    // no match
    return 0;
}


void AdBlockManager::applyHidingRules(WebPage *page)
{
    if(!page || !page->mainFrame())
        return;
    
    if (!_isAdblockEnabled)
        return;
    
    if (!_isHideAdsEnabled)
        return;
    
    foreach(const QString &filter, _hideList)
    {
        QWebElement document = page->mainFrame()->documentElement();
        QWebElementCollection elements = document.findAll(filter);

        foreach (QWebElement element, elements) 
        {
            element.setStyleProperty(QLatin1String("visibility"), QLatin1String("hidden"));
            element.removeFromDocument();
        }
    }
}
