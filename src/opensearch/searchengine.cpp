/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2008-2011 by Andrea Diamantini <adjam7 at gmail dot com>
* Copyright (C) 2009-2011 by Lionel Chauvin <megabigbug@yahoo.fr>
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

struct SearchEnginePrivate
{
    SearchEnginePrivate() : isLoaded(false) {}
    bool isLoaded;
    QString delimiter;
    KService::List favorites;
    KService::Ptr defaultEngine;
};

K_GLOBAL_STATIC(SearchEnginePrivate, d)

void SearchEngine::reload()
{
    KConfig config("kuriikwsfilterrc"); //Shared with konqueror
    KConfigGroup cg = config.group("General");

    //load delimiter
    d->delimiter = cg.readEntry("KeywordDelimiter", ":");

    //load favorite engines
    QStringList favoriteEngines;
    favoriteEngines = cg.readEntry("FavoriteSearchEngines", favoriteEngines);
    KService::List favorites;
    KService::Ptr service;
    Q_FOREACH(const QString & engine, favoriteEngines)
    {
        service = KService::serviceByDesktopPath(QString("searchproviders/%1.desktop").arg(engine));
        if(service)
        {
            QUrl url = service->property("Query").toUrl();
            rApp->iconManager()->downloadIconFromUrl(url);

            favorites << service;
        }
    }
    d->favorites = favorites;

    //load default engine
    QString dse = cg.readEntry("DefaultSearchEngine");
    d->defaultEngine = KService::serviceByDesktopPath(QString("searchproviders/%1.desktop").arg(dse));

    d->isLoaded = true;
}


QString SearchEngine::delimiter()
{
    if(!d->isLoaded)
        reload();

    return d->delimiter;
}


KService::List SearchEngine::favorites()
{
    if(!d->isLoaded)
        reload();

    return d->favorites;
}


KService::Ptr SearchEngine::defaultEngine()
{
    if(!d->isLoaded)
        reload();

    return d->defaultEngine;
}


KService::Ptr SearchEngine::fromString(const QString &text)
{
    KService::List providers = KServiceTypeTrader::self()->query("SearchProvider");
    int i = 0;
    bool found = false;
    KService::Ptr service;
    while(!found && i < providers.size())
    {
        QStringList list = providers.at(i)->property("Keys").toStringList();
        Q_FOREACH(const QString & key, list)
        {
            const QString searchPrefix = key + delimiter();
            if(text.startsWith(searchPrefix))
            {
                service = providers.at(i);
                found = true;
            }
        }
        i++;
    }

    return service;
}


QString SearchEngine::buildQuery(KService::Ptr engine, const QString &text)
{
    if(!engine)
        return QString();
    QString query = engine->property("Query").toString();
    query = query.replace("\\{@}", KUrl::toPercentEncoding(text));
    return query;
}
