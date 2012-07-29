/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2012 by Andrea Diamantini <adjam7 at gmail dot com>
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
#include "urlresolver.h"

// Local Includes
#include "searchengine.h"

// KDE Includes
#include <KService>
#include <KProtocolInfo>


// NOTE
// default kurifilter plugin list (at least in my box):
// 1. "kshorturifilter"
// 2. "kurisearchfilter"
// 3. "localdomainurifilter"
// 4 ."kuriikwsfilter"
// 5. "fixhosturifilter"


KUrl UrlResolver::urlFromTextTyped(const QString &typedText)
{
    QString typedString = typedText.trimmed();

    // Url from KService
    KService::Ptr engine = SearchEngine::fromString(typedString);
    if (engine)
    {
        QString query = typedString;
        query = query.remove(0, typedString.indexOf(SearchEngine::delimiter()) + 1);

        QString url = SearchEngine::buildQuery(engine, query);

        kDebug() << "Url from service: " << url;
        return KUrl(url);
    }

    // Url from User Input
    QUrl urlFromUserInput = QUrl::fromUserInput(typedString);
    if (urlFromUserInput.isValid())
    {
        // ensure http(s) hosts are lower cases
        if (urlFromUserInput.scheme().startsWith(QL1S("http")))
        {
            QString hst = urlFromUserInput.host();
            urlFromUserInput.setHost(hst.toLower());
        }

        kDebug() << "(Q)Url from user input: " << urlFromUserInput;
        return urlFromUserInput;
    }

    // failed...
    kDebug() << "KUrl fallback: " << typedText;
    return KUrl(typedText);
}


bool UrlResolver::isKDEUrl(const QString &urlString)
{
    KService::Ptr engine = SearchEngine::fromString(urlString);
    if (engine)
        return true;

    return false;
}
