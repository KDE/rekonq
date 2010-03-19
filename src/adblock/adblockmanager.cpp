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
#include "adblockmanager.h"
#include "adblockmanager.moc"

// Auto Includes
#include "rekonq.h"

// Local Includes
#include "adblocknetworkreply.h"
#include "webpage.h"

// KDE Includes
#include <KSharedConfig>
#include <KConfigGroup>
#include <KDebug>
#include <KIO/TransferJob>

// Qt Includes
#include <QUrl>
#include <QWebElement>


AdBlockManager::AdBlockManager(QObject *parent)
    : QObject(parent)
    , _isAdblockEnabled(false)
    , _isHideAdsEnabled(false)
    , _index(0)
{
}


AdBlockManager::~AdBlockManager()
{
}


void AdBlockManager::loadSettings(bool checkUpdateDate)
{
    _index = 0;
    _buffer.clear();
    
    _whiteList.clear();
    _blackList.clear();
    _hideList.clear();
    
    _isAdblockEnabled = ReKonfig::adBlockEnabled();
    kDebug() << "ADBLOCK ENABLED = " << _isAdblockEnabled;
    
    // no need to load filters if adblock is not enabled :)
    if(!_isAdblockEnabled)
        return;
    
    // just to be sure..
    _isHideAdsEnabled = ReKonfig::hideAdsEnabled();
    
    // local settings
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup rulesGroup( config, "rules" );
    QStringList rules;
    rules = rulesGroup.readEntry( "local-rules" , QStringList() );
    loadRules( rules );

    // ----------------------------------------------------------
        
    QDateTime today = QDateTime::currentDateTime();
    QDateTime lastUpdate = ReKonfig::lastUpdate();  //  the day of the implementation.. :)
    int days = ReKonfig::updateInterval();
    
    if( !checkUpdateDate || today > lastUpdate.addDays( days ) )
    {
        ReKonfig::setLastUpdate( today );
        
        updateNextSubscription();
        return;
    }

    // else
    QStringList titles = ReKonfig::subscriptionTitles();
    foreach(const QString &title, titles)
    {
        rules = rulesGroup.readEntry( title + "-rules" , QStringList() );
        loadRules(rules);
    }
}


void AdBlockManager::loadRules(const QStringList &rules)
{
    foreach(const QString &stringRule, rules)
    {
        // ! rules are comments
        if( !stringRule.startsWith('!') && !stringRule.startsWith('[') && !stringRule.isEmpty() )
        {           
            // white rules
            if( stringRule.startsWith( QLatin1String("@@") ) )
            {
                AdBlockRule rule( stringRule.mid(2) );
                _whiteList << rule;
                continue;
            }
            
            // hide (CSS) rules
            if( stringRule.startsWith( QLatin1String("##") ) )
            {
                _hideList << stringRule.mid(2);
                continue;
            }

            AdBlockRule rule( stringRule );
            _blackList << rule;
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
            kDebug() << "****ADBLOCK: WHITE RULE (@@) Matched: ***********";
            kDebug() << "Filter exp: " << filter.pattern();
            kDebug() << "UrlString:  " << urlString;
            return 0;        
        }
    }
    
    // then check the black ones :(
    foreach(const AdBlockRule &filter, _blackList)
    {
        if(filter.match(urlString))
        {
            kDebug() << "****ADBLOCK: BLACK RULE Matched: ***********";
            kDebug() << "Filter exp: " << filter.pattern();
            kDebug() << "UrlString:  " << urlString;
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
            kDebug() << "Hide element: " << element.localName();
            element.setStyleProperty(QLatin1String("visibility"), QLatin1String("hidden"));
            element.removeFromDocument();
        }
    }
}


void AdBlockManager::updateNextSubscription()
{
    QStringList locations = ReKonfig::subscriptionLocations();
    
    if( _index < locations.size() )
    {
        QString urlString = locations.at(_index);
        kDebug() << "DOWNLOADING FROM " << urlString;
        KUrl subUrl = KUrl( urlString );
        
        KIO::TransferJob* job = KIO::get( subUrl , KIO::Reload , KIO::HideProgressInfo );                                              
        connect(job, SIGNAL(data(KIO::Job*, const QByteArray&)), this, SLOT(subscriptionData(KIO::Job*, const QByteArray&)));                                                         
        connect(job, SIGNAL(result(KJob*)), this, SLOT(slotResult(KJob*)));
        
        return;
    }

    _index = 0;
    _buffer.clear();
}


void AdBlockManager::slotResult(KJob *job)
{
    kDebug() << "SLOTRESULT";
    if(job->error())
        return;

    QList<QByteArray> list = _buffer.split('\n');
    QStringList ruleList;
    foreach(const QByteArray &ba, list)
    {
        kDebug() << ba;
        ruleList << QString(ba);
    }
    loadRules(ruleList);
    saveRules(ruleList);
    
    _index++;
    
    // last..
    updateNextSubscription();
}


void AdBlockManager::subscriptionData(KIO::Job* job, const QByteArray& data)
{
    kDebug() << "subscriptionData";
    Q_UNUSED(job)
    
    if (data.isEmpty())
        return;
                                                                                          
    int oldSize = _buffer.size();
    _buffer.resize( _buffer.size() + data.size() );
    memcpy( _buffer.data() + oldSize, data.data(), data.size() );  
}


void AdBlockManager::saveRules(const QStringList &rules)
{
    QStringList cleanedRules;
    foreach(const QString &r, rules)
    {
        if( !r.startsWith('!') && !r.startsWith('[') && !r.isEmpty() )
            cleanedRules << r;
    }
    
    QStringList titles = ReKonfig::subscriptionTitles();
    QString title = titles.at(_index) + "-rules";
    
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup cg( config , "rules" );
    cg.writeEntry( title, cleanedRules );
}


void AdBlockManager::addSubscription(const QString &title, const QString &location)
{
    QStringList titles = ReKonfig::subscriptionTitles();
    if( titles.contains(title) )
        return;
    
    QStringList locations = ReKonfig::subscriptionLocations();
    if( locations.contains(location) )
        return;
    
    titles << title;
    locations << location;
    
    ReKonfig::setSubscriptionTitles(titles);
    ReKonfig::setSubscriptionLocations(locations);
}
