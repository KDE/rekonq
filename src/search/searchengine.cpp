/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2008-2010 by Andrea Diamantini <adjam7 at gmail dot com>
* Copyright (C) 2009-2010 by Lionel Chauvin <megabigbug@yahoo.fr>
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


//local includes
#include "searchengine.h"
#include "application.h"
#include "iconmanager.h"

// Auto Includes
#include "rekonq.h"

//KDE includes
#include <KConfigGroup>
#include <KServiceTypeTrader>


bool SearchEngine::m_loaded = false;
QString SearchEngine::m_delimiter = "";
KService::List SearchEngine::m_favorites;
KService::Ptr SearchEngine::m_defaultEngine;


void SearchEngine::reload()
{
    KConfig config("kuriikwsfilterrc"); //Shared with konqueror
    KConfigGroup cg = config.group("General");

    //load delimiter
    m_delimiter = cg.readEntry("KeywordDelimiter", ":");

    //load favorite engines
    QStringList favoriteEngines;
    favoriteEngines = cg.readEntry("FavoriteSearchEngines", favoriteEngines);
    KService::List favorites;
    KService::Ptr service;
    foreach(const QString &engine, favoriteEngines)
    {
        service = KService::serviceByDesktopPath(QString("searchproviders/%1.desktop").arg(engine));
        if (service)
        {
            QUrl url = service->property("Query").toUrl();
            kDebug() << "ENGINE URL: " << url;
            Application::iconManager()->downloadIconFromUrl(url);
            
            favorites << service;
        }
    }
    m_favorites = favorites;

    //load default engine
    QString d = cg.readEntry("DefaultSearchEngine");  
    m_defaultEngine = KService::serviceByDesktopPath(QString("searchproviders/%1.desktop").arg(d));
    if (!m_defaultEngine)
    {
        d = QL1S("google");
        m_defaultEngine = KService::serviceByDesktopPath(QString("searchproviders/%1.desktop").arg(d));
    }
    
    m_loaded = true;
}


QString SearchEngine::delimiter()
{
    if (!m_loaded)
        reload();

    return m_delimiter;
}


KService::List SearchEngine::favorites()
{
    if (!m_loaded)
        reload();

    return m_favorites;
}


KService::Ptr SearchEngine::defaultEngine()
{
    if (!m_loaded)
        reload();

    return m_defaultEngine;
}


KService::Ptr SearchEngine::fromString(QString text)
{
    KService::List providers = KServiceTypeTrader::self()->query("SearchProvider");
    int i = 0;
    bool found = false;
    KService::Ptr service;
    while (!found && i < providers.size())
    {
        QStringList list = providers.at(i)->property("Keys").toStringList();
        foreach(const QString &key, list)
        {
            const QString searchPrefix = key + delimiter();
            if (text.startsWith(searchPrefix))
            {
                service = providers.at(i);
                found = true;
            }
        }
        i++;
    }

    return service;
}


QString SearchEngine::buildQuery(KService::Ptr engine, QString text)
{
    QString query = engine->property("Query").toString();
    query = query.replace("\\{@}", KUrl::toPercentEncoding(text));
    return query;
}



