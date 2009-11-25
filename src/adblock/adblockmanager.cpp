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


#include "adblockmanager.h"
#include "adblockmanager.moc"


// Local Includes
#include "adblockrule.h"

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

        _adBlackList.clear();
        _adWhiteList.clear();

        QMap<QString,QString> entryMap = cg.entryMap();
        QMap<QString,QString>::ConstIterator it;
        for( it = entryMap.constBegin(); it != entryMap.constEnd(); ++it )
        {
            QString name = it.key();
            QString url = it.value();

            if (name.startsWith(QLatin1String("Filter")))
            {
                if (url.startsWith(QLatin1String("@@")))
                    _adWhiteList.addFilter(url);
                else
                    _adBlackList.addFilter(url);
            }
        }
    }
}


bool AdBlockManager::isUrlAllowed(const QUrl &url)
{
    if (!_isAdblockEnabled)
        return true;

    QString urlString = QString::fromUtf8(url.toEncoded());
    
    // Check the blacklist, and only if that matches, the whitelist
    return _adBlackList.isUrlMatched(urlString) && !_adWhiteList.isUrlMatched(urlString);
}
