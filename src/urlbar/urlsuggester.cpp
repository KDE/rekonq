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
#include "urlsuggester.h"
#include "urlsuggester.moc"

// Local Includes
#include "historymanager.h"
#include "bookmarkmanager.h"

#include "searchengine.h"

// KDE Includes
#include <KBookmark>
#include <KService>
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


// NOTE
// Induce an order in the history items
bool isHistoryItemRelevant(const HistoryItem &a, const HistoryItem &b)
{
    return a.relevance() > b.relevance();
}


// ------------------------------------------------------------------------


QRegExp UrlSuggester::_browseRegexp;
QRegExp UrlSuggester::_searchEnginesRegexp;


UrlSuggester::UrlSuggester(const QString &typedUrl)
    : QObject()
    , _typedString(typedUrl.trimmed())
    , _isKDEShortUrl(false)
{
    if (_browseRegexp.isEmpty())
    {
        QString protocol = QString("^(%1)").arg(KProtocolInfo::protocols().join("|"));
        protocol += QL1S("|javascript");
        
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


UrlSuggestionList UrlSuggester::orderedSearchItems()
{
    if (_typedString.startsWith(QL1S("rekonq:")))
    {
        QStringList aboutUrlList;
        aboutUrlList
                << QL1S("rekonq:home")
                << QL1S("rekonq:favorites")
                << QL1S("rekonq:bookmarks")
                << QL1S("rekonq:history")
                << QL1S("rekonq:downloads")
                << QL1S("rekonq:closedtabs")
                ;

        QStringList aboutUrlResults = aboutUrlList.filter(_typedString, Qt::CaseInsensitive);

        UrlSuggestionList list;

        Q_FOREACH(const QString & urlResult, aboutUrlResults)
        {
            QString name = urlResult;
            name.remove(0, 6);
            UrlSuggestionItem item(UrlSuggestionItem::Browse, urlResult, name);
            list << item;
        }

        return list;
    }

    // NOTE: this sets _isKDEShortUrl.
    // IF it is true we can just suggest it
    computeWebSearches();

    if (_isKDEShortUrl)
    {
        return _webSearches;
    }

    //compute lists
    computeHistory();
    computeQurlFromUserInput();
    computeBookmarks();

    return orderLists();
}


UrlSuggestionList UrlSuggester::orderLists()
{
    // NOTE
    // The const int here decides the number of proper suggestions, taken from history & bookmarks
    // You have to add here the "browse & search" options, always available.
    const int availableEntries = 8;

    // Browse & Search results
    UrlSuggestionList browseSearch;
    QString lowerTypedString = _typedString.toLower();

    bool textIsUrl = (_browseRegexp.indexIn(lowerTypedString) != -1);

    if (textIsUrl)
    {
        // browse url case (typed kde.org): show resolved url before
        browseSearch << _qurlFromUserInput;
        browseSearch << _webSearches;
    }
    else
    {
        // NON url case: propose web search before
        browseSearch << _webSearches;
        browseSearch << _qurlFromUserInput;
    }


    // find relevant items (the one you are more probably searching...)
    UrlSuggestionList relevant;

    // history
    Q_FOREACH(const UrlSuggestionItem & item, _history)
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

    // just add this is relevant is Null

    if (relevant.count() == 0)
    {
        // bookmarks
        Q_FOREACH(const UrlSuggestionItem & item, _bookmarks)
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

    if (_typedString.count() > 1)
        removeBookmarksDuplicates();

    // and finally, results
    UrlSuggestionList list;
    if (textIsUrl)
        list += browseSearch + relevant + _history + _bookmarks;
    else
        list += relevant + browseSearch + _history + _bookmarks;

    return list;
}


//////////////////////////////////////////////////////////////////////////
// PRIVATE ENGINES


// QUrl from User Input (easily the best solution... )
void UrlSuggester::computeQurlFromUserInput()
{
    QString url = _typedString;
    QUrl urlFromUserInput = QUrl::fromUserInput(url);
    if (urlFromUserInput.isValid())
    {
        // ensure http(s) hosts are lower cases
        if (urlFromUserInput.scheme().startsWith(QL1S("http")))
        {
            QString hst = urlFromUserInput.host();
            urlFromUserInput.setHost(hst.toLower());
        }

        QString urlString = urlFromUserInput.toString();
        QString gTitle = i18nc("Browse a website", "Browse");
        UrlSuggestionItem gItem(UrlSuggestionItem::Browse, urlString, gTitle);
        _qurlFromUserInput << gItem;
    }
}


// webSearches
void UrlSuggester::computeWebSearches()
{
    QString query = _typedString;

    // this result is generated when an user types something like gg:kde
    KService::Ptr engine = SearchEngine::fromString(_typedString);
    if (engine)
    {
        query = query.remove(0, _typedString.indexOf(SearchEngine::delimiter()) + 1);
        _isKDEShortUrl = true;
    }
    else
    {
        engine = SearchEngine::defaultEngine();
    }

    if (engine)
    {
        UrlSuggestionItem item = UrlSuggestionItem(UrlSuggestionItem::Search, SearchEngine::buildQuery(engine, query), query, engine->name());
        UrlSuggestionList list;
        list << item;
        _webSearches = list;
    }
}


// history
void UrlSuggester::computeHistory()
{
    QList<HistoryItem> found = HistoryManager::self()->find(_typedString);

    // FIXME: profiling computeHistory, this seems too much expensive (around 1 second for)
    // doing it just from second time...
    if (_typedString.count() > 1)
        qSort(found.begin(), found.end(), isHistoryItemRelevant);

    Q_FOREACH(const HistoryItem & i, found)
    {
        if (_searchEnginesRegexp.isEmpty() || _searchEnginesRegexp.indexIn(i.url) == -1) //filter all urls that are search engine results
        {
            UrlSuggestionItem gItem(UrlSuggestionItem::History, i.url, i.title);
            _history << gItem;
        }
    }
}


// bookmarks
void UrlSuggester::computeBookmarks()
{
    QList<KBookmark> found = BookmarkManager::self()->find(_typedString);
    Q_FOREACH(const KBookmark & b, found)
    {
        UrlSuggestionItem gItem(UrlSuggestionItem::Bookmark, b.url().url(), b.fullText());
        _bookmarks << gItem;
    }
}


// opensearch suggestion
void UrlSuggester::computeSuggestions()
{
    // NOTE
    // This attempt basically cuts out open search suggestions.
    UrlSuggestionList list;
    emit suggestionsReady(list, _typedString);
    return;

//     // if a string startsWith /, it is probably a local path
//     // so, no need for suggestions...
//     if (_typedString.startsWith('/') || !rApp->opensearchManager()->isSuggestionAvailable())
//     {
//         UrlSuggestionList list;
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
//             SIGNAL(suggestionsReceived(QString,ResponseList)),
//             this,
//             SLOT(suggestionsReceived(QString,ResponseList)));
//
//     _typedQuery = query;
//     rApp->opensearchManager()->requestSuggestion(query);
}


// void UrlSuggester::suggestionsReceived(const QString &text, const ResponseList &suggestions)
// {
//     if (text != _typedString)
//         return;
//
//     UrlSuggestionList sugList;
//     QString urlString;
//     Q_FOREACH(const Response & i, suggestions)
//     {
//         if (text == i.title)
//             continue;
//
//         urlString = i.url;
//         if (urlString.isEmpty())
//         {
//             urlString = SearchEngine::buildQuery(UrlSuggester::searchEngine(), i.title);
//         }
//
//         UrlSuggestionItem gItem(UrlSuggestionItem::Suggestion, urlString, i.title, i.description, i.image, i.image_width, i.image_height);
//         sugList << gItem;
//     }
//     emit suggestionsReady(sugList, _typedString);
//     this->deleteLater();
// }


//////////////////////////////////////////////////////////////////////////


// WARNING: this seems  A LOT expensive and has to be done just
// when the two groups (history & bookmarks) have just been "restricted"..
void UrlSuggester::removeBookmarksDuplicates()
{
    Q_FOREACH(const UrlSuggestionItem & item, _history)
    {
        QString hu = item.url;
        Q_FOREACH(const UrlSuggestionItem & item, _bookmarks)
        {
            if (hu == item.url)
            {
                _bookmarks.removeOne(item);
                break;
            }
        }
    }
}
