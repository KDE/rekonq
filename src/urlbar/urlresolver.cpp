/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2009-2012 by Andrea Diamantini <adjam7 at gmail dot com>
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
#include "urlresolver.moc"

// Local Includes
#include "historymanager.h"
#include "bookmarkmanager.h"
#include "searchengine.h"

// KDE Includes
#include <KBookmark>
#include <KUriFilter>
#include <KCompletion>
#include <KService>
#include <KConfig>
#include <KConfigGroup>
#include <KProtocolInfo>

// Qt Includes
#include <QByteArray>


// NOTE
// default kurifilter plugin list (at least in my box):
// 1. "kshorturifilter"
// 2. "kurisearchfilter"
// 3. "localdomainurifilter"
// 4 ."kuriikwsfilter"
// 5. "fixhosturifilter"


// ------------------------------------------------------------------------

KService::Ptr UrlResolver::_searchEngine;

QRegExp UrlResolver::_browseRegexp;
QRegExp UrlResolver::_searchEnginesRegexp;


UrlResolver::UrlResolver(const QString &typedUrl)
    : QObject()
    , _typedString(typedUrl.trimmed())
    , _typedQuery()
{
    if (!_searchEngine)
        setSearchEngine(SearchEngine::defaultEngine());

    if (_browseRegexp.isEmpty())
    {
        QString protocol = QString("^(%1)").arg(KProtocolInfo::protocols().join("|"));

        QString localhost = QL1S("^localhost");

        QString local = QL1S("^/");

        QString ipv4 = QL1S("^0*([1-9]?\\d|1\\d\\d|2[0-4]\\d|25[0-5])\\.0*([1-9]?\\d|1\\d\\d|2[0-4]\\d|25[0-5])"\
                            "\\.0*([1-9]?\\d|1\\d\\d|2[0-4]\\d|25[0-5])\\.0*([1-9]?\\d|1\\d\\d|2[0-4]\\d|25[0-5])");

        QString ipv6 = QL1S("^([0-9a-fA-F]{4}|0)(\\:([0-9a-fA-F]{4}|0)){7}");

        QString address = QL1S("[\\d\\w-.]+\\.(a[cdefgilmnoqrstuwz]|b[abdefghijmnorstvwyz]|"\
                               "c[acdfghiklmnoruvxyz]|d[ejkmnoz]|e[ceghrstu]|f[ijkmnor]|g[abdefghilmnpqrstuwy]|"\
                               "h[kmnrtu]|i[delmnoqrst]|j[emop]|k[eghimnprwyz]|l[abcikrstuvy]|"\
                               "m[acdghklmnopqrstuvwxyz]|n[acefgilopruz]|om|p[aefghklmnrstwy]|qa|r[eouw]|"\
                               "s[abcdeghijklmnortuvyz]|t[cdfghjkmnoprtvwz]|u[augkmsyz]|v[aceginu]|w[fs]|"\
                               "y[etu]|z[amw]|aero|arpa|biz|com|coop|edu|info|int|gov|local|mil|museum|name|net|org|"\
                               "pro)");

        QString joiner = QL1S(")|(");
        _browseRegexp = QRegExp(QL1C('(') +
                                protocol + joiner +
                                localhost + joiner +
                                local + joiner +
                                address + joiner +
                                ipv6 + joiner +
                                ipv4 + QL1C(')')
                               );
    }

    if (_searchEnginesRegexp.isEmpty())
    {
        QString reg;
        QString engineUrl;
        Q_FOREACH(KService::Ptr s, SearchEngine::favorites())
        {
            engineUrl = QRegExp::escape(s->property("Query").toString()).replace(QL1S("\\\\\\{@\\}"), QL1S("[\\d\\w-.]+"));
            if (reg.isEmpty())
                reg = QL1C('(') + engineUrl + QL1C(')');
            else
                reg = reg + QL1S("|(") + engineUrl + QL1C(')');
        }
        _searchEnginesRegexp = QRegExp(reg);
    }
}


UrlSearchList UrlResolver::orderedSearchItems()
{
    if (_typedString.startsWith(QL1S("about:")))
    {
        UrlSearchList list;
        UrlSearchItem home(UrlSearchItem::Browse, QL1S("about:home"),       QL1S("home"));
        list << home;
        UrlSearchItem favs(UrlSearchItem::Browse, QL1S("about:favorites"),  QL1S("favorites"));
        list << favs;
        UrlSearchItem clos(UrlSearchItem::Browse, QL1S("about:closedTabs"), QL1S("closed tabs"));
        list << clos;
        UrlSearchItem book(UrlSearchItem::Browse, QL1S("about:bookmarks"),  QL1S("bookmarks"));
        list << book;
        UrlSearchItem hist(UrlSearchItem::Browse, QL1S("about:history"),    QL1S("history"));
        list << hist;
        UrlSearchItem down(UrlSearchItem::Browse, QL1S("about:downloads"),  QL1S("downloads"));
        list << down;

        return list;
    }

    //compute lists
    computeHistory();
    computeQurlFromUserInput();
    computeWebSearches();
    computeBookmarks();

    return orderLists();
}


UrlSearchList UrlResolver::orderLists()
{
    // NOTE
    // The const int here decides the number of proper suggestions, taken from history & bookmarks
    // You have to add here the "browse & search" options, always available.
    const int availableEntries = 8;

    bool webSearchFirst = false;
    // Browse & Search results
    UrlSearchList browseSearch;
    if (_browseRegexp.indexIn(_typedString) != -1)
    {
        webSearchFirst = true;
        browseSearch << _webSearches;
    }
    else
    {
        browseSearch << _webSearches;
        browseSearch << _qurlFromUserInput;
    }


    // find relevant items (the one you are more probably searching...)
    UrlSearchList relevant;

    // history
    Q_FOREACH(const UrlSearchItem & item, _history)
    {
        QString hst = KUrl(item.url).host();
        if (item.url.startsWith(_typedString)
                || hst.startsWith(_typedString)
                || hst.remove("www.").startsWith(_typedString))
        {
            relevant << item;
            _history.removeOne(item);
            break;
        }
    }

    // bookmarks
    Q_FOREACH(const UrlSearchItem & item, _bookmarks)
    {
        QString hst = KUrl(item.url).host();
        if (item.url.startsWith(_typedString)
                || hst.startsWith(_typedString)
                || hst.remove("www.").startsWith(_typedString))
        {
            relevant << item;
            _bookmarks.removeOne(item);
            break;
        }
    }

    // decide history & bookmarks number
    int historyCount = _history.count();
    int bookmarksCount = _bookmarks.count();
    int relevantCount = relevant.count();

    const int historyEntries = (availableEntries - relevantCount) / 2;
    const int bookmarksEntries = availableEntries - relevantCount - historyEntries;

    if (historyCount >= historyEntries && bookmarksCount >= bookmarksEntries)
    {
        _history = _history.mid(0, historyEntries);
        _bookmarks = _bookmarks.mid(0, bookmarksEntries);
    }
    else if (historyCount < historyEntries && bookmarksCount >= bookmarksEntries)
    {
        if (historyCount + bookmarksCount > availableEntries)
        {
            _bookmarks = _bookmarks.mid(0, availableEntries - historyCount);
        }
    }
    else if (historyCount >= historyEntries && bookmarksCount < bookmarksEntries)
    {
        if (historyCount + bookmarksCount > availableEntries)
        {
            _history = _history.mid(0, availableEntries - bookmarksCount);
        }
    }

    // and finally, results
    UrlSearchList list;

    if (webSearchFirst)
        list << _qurlFromUserInput;
    list += relevant + browseSearch + _history + _bookmarks;
    return list;
}


//////////////////////////////////////////////////////////////////////////
// PRIVATE ENGINES


// QUrl from User Input (easily the best solution... )
void UrlResolver::computeQurlFromUserInput()
{
    QString url = _typedString;
    QUrl urlFromUserInput = QUrl::fromUserInput(url);
    if (urlFromUserInput.isValid())
    {
        QString gTitle = i18nc("Browse a website", "Browse");
        UrlSearchItem gItem(UrlSearchItem::Browse, urlFromUserInput.toString(), gTitle);
        _qurlFromUserInput << gItem;
    }
}


// webSearches
void UrlResolver::computeWebSearches()
{
    QString query = _typedString;
    KService::Ptr engine = SearchEngine::fromString(_typedString);
    if (engine)
    {
        query = query.remove(0, _typedString.indexOf(SearchEngine::delimiter()) + 1);
        setSearchEngine(engine);
    }

    if (_searchEngine)
    {
        UrlSearchItem item = UrlSearchItem(UrlSearchItem::Search, SearchEngine::buildQuery(_searchEngine, query), query);
        UrlSearchList list;
        list << item;
        _webSearches = list;
    }
}


// history
void UrlResolver::computeHistory()
{
    QList<HistoryItem> found = rApp->historyManager()->find(_typedString);
    qSort(found.begin(), found.end(), isHistoryItemRelevant);

    Q_FOREACH(const HistoryItem & i, found)
    {
        if (_searchEnginesRegexp.isEmpty() || _searchEnginesRegexp.indexIn(i.url) == -1) //filter all urls that are search engine results
        {
            UrlSearchItem gItem(UrlSearchItem::History, i.url, i.title);
            _history << gItem;
        }
    }
}


// bookmarks
void UrlResolver::computeBookmarks()
{
    QList<KBookmark> found = rApp->bookmarkManager()->find(_typedString);
    Q_FOREACH(const KBookmark & b, found)
    {
        UrlSearchItem gItem(UrlSearchItem::Bookmark, b.url().url(), b.fullText());
        _bookmarks << gItem;
    }
}


// opensearch suggestion
void UrlResolver::computeSuggestions()
{
    // NOTE
    // This attempt basically cuts out open search suggestions.
    UrlSearchList list;
    emit suggestionsReady(list, _typedString);
    return;

//     // if a string startsWith /, it is probably a local path
//     // so, no need for suggestions...
//     if (_typedString.startsWith('/') || !rApp->opensearchManager()->isSuggestionAvailable())
//     {
//         UrlSearchList list;
//         emit suggestionsReady(list, _typedString);
//         return;
//     }
//
//     QString query = _typedString;
//     KService::Ptr engine = SearchEngine::fromString(_typedString);
//     if (engine)
//     {
//         query = query.remove(0, _typedString.indexOf(SearchEngine::delimiter()) + 1);
//         setSearchEngine(engine);
//     }
//
//     connect(rApp->opensearchManager(),
//             SIGNAL(suggestionsReceived(const QString &, const ResponseList &)),
//             this,
//             SLOT(suggestionsReceived(const QString &, const ResponseList &)));
//
//     _typedQuery = query;
//     rApp->opensearchManager()->requestSuggestion(query);
}


bool UrlResolver::isHistoryItemRelevant(const HistoryItem &a, const HistoryItem &b)
{
    return a.relevance() > b.relevance();
}


void UrlResolver::suggestionsReceived(const QString &text, const ResponseList &suggestions)
{
    if (text != _typedQuery)
        return;

    UrlSearchList sugList;
    QString urlString;
    Q_FOREACH(const Response & i, suggestions)
    {
        if (text == i.title)
            continue;

        urlString = i.url;
        if (urlString.isEmpty())
        {
            urlString = SearchEngine::buildQuery(UrlResolver::searchEngine(), i.title);
        }

        UrlSearchItem gItem(UrlSearchItem::Suggestion, urlString, i.title, i.description, i.image, i.image_width, i.image_height);
        sugList << gItem;
    }
    emit suggestionsReady(sugList, _typedString);
    this->deleteLater();
}
