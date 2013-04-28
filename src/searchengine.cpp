/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2008-2012 by Andrea Diamantini <adjam7 at gmail dot com>
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


// local includes
#include "searchengine.h"

//KDE includes
#include <KConfigGroup>
#include <KServiceTypeTrader>
#include <KUriFilter>

#include <QStringList>


struct SearchEnginePrivate
{
    SearchEnginePrivate() : isLoaded(false) {}
    bool isLoaded;
    bool isEnabled;
    bool usePreferredOnly;
    QString delimiter;
    KService::List favorites;
    KService::Ptr defaultEngine;
};


K_GLOBAL_STATIC(SearchEnginePrivate, d)



void SearchEngine::reload()
{
    KConfig config("kuriikwsfilterrc"); //Shared with konqueror
    KConfigGroup cg = config.group("General");

    d->isEnabled = cg.readEntry("EnableWebShortcuts", true);
    d->usePreferredOnly = cg.readEntry("UsePreferredWebShortcutsOnly", false);

    //load delimiter
    d->delimiter = cg.readEntry("KeywordDelimiter", ":");

    // load favorite engines
    QStringList favoriteEngines;
    favoriteEngines = cg.readEntry("PreferredWebShortcuts", favoriteEngines);

    KService::List favorites;
    KService::Ptr service;
    Q_FOREACH(const QString & engine, favoriteEngines)
    {
        service = KService::serviceByDesktopPath(QString("searchproviders/%1.desktop").arg(engine));
        if (service)
        {
            favorites << service;
        }
    }
    d->favorites = favorites;

    // load default engine
    QString dse;
    dse = cg.readEntry("DefaultWebShortcut");

    d->defaultEngine = KService::serviceByDesktopPath(QString("searchproviders/%1.desktop").arg(dse));

    d->isLoaded = true;
}


QString SearchEngine::delimiter()
{
    if (!d->isLoaded)
        reload();

    return d->delimiter;
}


KService::List SearchEngine::favorites()
{
    if (!d->isLoaded)
        reload();

    return d->favorites;
}


KService::Ptr SearchEngine::defaultEngine()
{
    if (!d->isLoaded)
        reload();

    return d->defaultEngine;
}


KService::Ptr SearchEngine::fromString(const QString &text)
{
    KService::Ptr service;

    // first, the easy part...
    if (!d->isEnabled)
        return service;

    KService::List providers = (d->usePreferredOnly)
        ? SearchEngine::favorites()
        : KServiceTypeTrader::self()->query("SearchProvider");

    int i = 0;
    bool found = false;
    while (!found && i < providers.size())
    {
        QStringList list = providers.at(i)->property("Keys").toStringList();
        Q_FOREACH(const QString & key, list)
        {
            const QString searchPrefix = key + delimiter();
            if (text.startsWith(searchPrefix))
            {
                service = providers.at(i);
                found = true;
                break;
            }
        }
        i++;
    }

    return service;
}


QString SearchEngine::buildQuery(KService::Ptr engine, const QString &text)
{
    if (!engine)
        return QString();
    
    QString shortcut = engine->property("Keys").toStringList().at(0);
    QString query = shortcut + delimiter() + text;

    QStringList filters;
    filters << QL1S("kurisearchfilter");
    KUriFilter::self()->filterUri(query, filters);
    
    return query;
}
