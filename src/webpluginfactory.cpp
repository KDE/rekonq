/* ============================================================
*
* This file is a part of the rekonq project
*
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
#include "webpluginfactory.h"
#include "webpluginfactory.moc"

// Local Includes
#include "rekonq.h"
#include "application.h"
#include "mainwindow.h"
#include "clicktoflash.h"

// KDE Includes
#include <KDebug>


WebPluginFactory::WebPluginFactory(QObject *parent)
    : KWebPluginFactory(parent)
    , _loadClickToFlash(false)
{
    connect(this, SIGNAL(signalLoadClickToFlash(bool)), SLOT(setLoadClickToFlash(bool)));
}


void WebPluginFactory::setLoadClickToFlash(bool load)
{
    _loadClickToFlash = load;
}


QObject *WebPluginFactory::create(const QString &mimeType,
                                  const QUrl &url,
                                  const QStringList &argumentNames,
                                  const QStringList &argumentValues) const
{
    kDebug() << "loading mimeType: " << mimeType;
    
    switch( ReKonfig::pluginsEnabled() )
    {
    case 0:
        kDebug() << "No plugins found for" << mimeType << ". Falling back to KDEWebKit ones...";
        return KWebPluginFactory::create(mimeType, url, argumentNames, argumentValues);
        
    case 1:
        if( mimeType != QString("application/x-shockwave-flash") )
            break;
        
        if( _loadClickToFlash )
        {
            emit signalLoadClickToFlash(false);
            return 0; //KWebPluginFactory::create(mimeType, url, argumentNames, argumentValues);
        }
        else
        {
            ClickToFlash* ctf = new ClickToFlash(url);
            connect(ctf, SIGNAL(signalLoadClickToFlash(bool)), this, SLOT(setLoadClickToFlash(bool)));
            return ctf;
        }
        break;
        
    case 2:
        return 0;
        
    default:
        kDebug() << "oh oh.. this should NEVER happen..";
        break;
    }
    
    return KWebPluginFactory::create(mimeType, url, argumentNames, argumentValues);
}
