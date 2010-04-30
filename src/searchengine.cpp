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

//KDE includes
#include <KConfigGroup>
#include <KServiceTypeTrader>


QString SearchEngine::m_delimiter = "";


QString SearchEngine::delimiter()
{
    if (m_delimiter == "") loadDelimiter();
    return m_delimiter;
}


void SearchEngine::loadDelimiter()
{
    KConfig config("kuriikwsfilterrc"); //Share with konqueror
    KConfigGroup cg = config.group("General");
    m_delimiter = cg.readEntry("KeywordDelimiter", ":");
}


KService::Ptr SearchEngine::m_defaultWS;


KService::Ptr SearchEngine::defaultWS()
{
    if (!m_defaultWS) loadDefaultWS();
    return m_defaultWS;
}


void SearchEngine::loadDefaultWS()
{
    KConfig config("kuriikwsfilterrc"); //Share with konqueror
    KConfigGroup cg = config.group("General");
    QString d = cg.readEntry("DefaultSearchEngine", "google");
    m_defaultWS = KService::serviceByDesktopPath(QString("searchproviders/%1.desktop").arg(d));
}


KService::Ptr SearchEngine::fromString(QString text)
{
    KService::List providers = KServiceTypeTrader::self()->query("SearchProvider");
    int i = 0;
    bool found = false;
    KService::Ptr service;
    while (!found && i < providers.size())
    {
        foreach(QString key, providers.at(i)->property("Keys").toStringList())
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


KService::List SearchEngine::m_favorites;


KService::List SearchEngine::favorites()
{
    if (m_favorites.isEmpty()) loadFavorites();
    return m_favorites;
}


void SearchEngine::loadFavorites()
{
    KConfig config("kuriikwsfilterrc"); //Share with konqueror
    KConfigGroup cg = config.group("General");
    QStringList f;
    f << "wikipedia" << "google"; //defaults
    f = cg.readEntry("FavoriteSearchEngines", f);

    KService::List favorites;
    KService::Ptr service;
    foreach(QString e, f)
    {
        service = KService::serviceByDesktopPath(QString("searchproviders/%1.desktop").arg(e));
        if (service) favorites << service;
    }

    m_favorites = favorites;
}


KService::Ptr SearchEngine::defaultEngine()
{
    int n = ReKonfig::searchEngine();
    QString engine;
    switch (n)
    {
    case 0:
        engine = QL1S("google");
        break;
    case 1:
        engine = QL1S("altavista");
        break;
    case 2:
        engine = QL1S("lycos");
        break;
    case 3:
        engine = QL1S("wikipedia");
        break;
    case 4:
        engine = QL1S("wolfram");
        break;
    default:
        engine = QL1S("google");
        break;
    }

    return KService::serviceByDesktopPath(QString("searchproviders/%1.desktop").arg(engine));
}

