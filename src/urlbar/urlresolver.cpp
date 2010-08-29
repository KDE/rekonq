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


// Self Includes
#include "urlresolver.h"

// Local Includes
#include "application.h"
#include "historymanager.h"
#include "bookmarkprovider.h"
#include "searchengine.h"

// KDE Includes
#include <KBookmark>
#include <KUriFilter>
#include <KCompletion>
#include <KService>
#include <KConfig>
#include <KConfigGroup>

// Qt Includes
#include <QByteArray>

// defines
#define MAX_ELEMENTS 8
#define MIN_SUGGESTIONS 3

// NOTE
// default kurifilter plugin list (at least in my box):
// 1. "kshorturifilter"
// 2. "kurisearchfilter"
// 3. "localdomainurifilter"
// 4 ."kuriikwsfilter"
// 5. "fixhosturifilter"


// ------------------------------------------------------------------------


QRegExp UrlResolver::_browseRegexp;
QRegExp UrlResolver::_searchEnginesRegexp;

UrlResolver::UrlResolver(const QString &typedUrl)
        : QObject()
        , _typedString(typedUrl.trimmed())
{
    if ( _browseRegexp.isEmpty() )
    {
        kDebug() << "browse regexp empty. Setting value..";

        QString protocol = "^(http://|https://|file://|ftp://|man:|info:|apt:|about:)";

        QString localhost = "^localhost";

        QString local = "^/";

        QString ipv4 = "^0*([1-9]?\\d|1\\d\\d|2[0-4]\\d|25[0-5])\\.0*([1-9]?\\d|1\\d\\d|2[0-4]\\d|25[0-5])"\
        "\\.0*([1-9]?\\d|1\\d\\d|2[0-4]\\d|25[0-5])\\.0*([1-9]?\\d|1\\d\\d|2[0-4]\\d|25[0-5])";

        QString ipv6 = "^([0-9a-fA-F]{4}|0)(\\:([0-9a-fA-F]{4}|0)){7}";

        QString address = "[\\d\\w-.]+\\.(a[cdefgilmnoqrstuwz]|b[abdefghijmnorstvwyz]|"\
        "c[acdfghiklmnoruvxyz]|d[ejkmnoz]|e[ceghrst]|f[ijkmnor]|g[abdefghilmnpqrstuwy]|"\
        "h[kmnrtu]|i[delmnoqrst]|j[emop]|k[eghimnprwyz]|l[abcikrstuvy]|"\
        "m[acdghklmnopqrstuvwxyz]|n[acefgilopruz]|om|p[aefghklmnrstwy]|qa|r[eouw]|"\
        "s[abcdeghijklmnortuvyz]|t[cdfghjkmnoprtvwz]|u[augkmsyz]|v[aceginu]|w[fs]|"\
        "y[etu]|z[amw]|aero|arpa|biz|com|coop|edu|info|int|gov|mil|museum|name|net|org|"\
        "pro)";

        _browseRegexp = QRegExp('(' + protocol + ")|(" + localhost + ")|(" + local + ")|(" + address + ")|(" + ipv6 + ")|(" + ipv4 +')');
    }

    if ( _searchEnginesRegexp.isEmpty() )
    {
        QString reg;
        QString engineUrl;
        Q_FOREACH(KService::Ptr s, SearchEngine::favorites())
        {
            engineUrl = QRegExp::escape(s->property("Query").toString()).replace("\\\\\\{@\\}","[\\d\\w-.]+");
            if (reg.isEmpty())
                reg = '(' + engineUrl + ')';
            else
                reg = reg + "|(" + engineUrl + ')';
        }
        _searchEnginesRegexp = QRegExp(reg);
    }
}


UrlSearchList UrlResolver::orderedSearchItems()
{
    if( _typedString == QL1S("about:") )
    {
        UrlSearchList list;
        UrlSearchItem home(UrlSearchItem::Browse, QString("about:home"),       QL1S("home") );
        list << home;
        UrlSearchItem favs(UrlSearchItem::Browse, QString("about:favorites"),  QL1S("favorites") );
        list << favs;
        UrlSearchItem clos(UrlSearchItem::Browse, QString("about:closedTabs"), QL1S("closed tabs") );
        list << clos;
        UrlSearchItem book(UrlSearchItem::Browse, QString("about:bookmarks"),  QL1S("bookmarks") );
        list << book;
        UrlSearchItem hist(UrlSearchItem::Browse, QString("about:history"),    QL1S("history") );
        list << hist;
        UrlSearchItem down(UrlSearchItem::Browse, QString("about:downloads"),  QL1S("downloads") );
        list << down;

        return list;
    }

    //compute lists
    computeSuggestions();
    computeHistory();
    computeQurlFromUserInput();
    computeWebSearches();
    computeBookmarks();

    return orderLists();
}


UrlSearchList UrlResolver::orderLists()
{
    // NOTE
    // the logic here is : "we wanna suggest (at least) 10 elements"
    // so we have (more or less) 2 from first results (1 from QUrl Resolutions, 1 from
    // search engines).
    // There are 8 remaining: if bookmarkResults + historyResults <= 8, catch all, else
    // catch first 4 results from the two resulting lists :)

    QTime myTime;
    myTime.start();

    UrlSearchList list;

    if(_browseRegexp.indexIn(_typedString) != -1)
    {
        list << _qurlFromUserInput;
        list << _webSearches;
    }
    else
    {
        list << _webSearches;
        list << _qurlFromUserInput;
    }

    //find the history items that match the typed string
    UrlSearchItem privileged = privilegedItem(&_history);
    int historyCount = _history.count();

    //find the bookmarks items that match the typed string
    if (privileged.type == UrlSearchItem::Undefined)
    {
        privileged = privilegedItem(&_bookmarks);
    }
    else if(privileged.type == UrlSearchItem::History && _bookmarks.removeOne(privileged))
    {
        privileged.type |= UrlSearchItem::Bookmark;
    }
    int bookmarksCount = _bookmarks.count();

    if (privileged.type != UrlSearchItem::Undefined)
    {
        list.prepend(privileged);
    }

    int availableEntries = MAX_ELEMENTS - list.count() - MIN_SUGGESTIONS;

    UrlSearchList common;
    int commonCount = 0;

    //prefer items which are history items als well bookmarks item
    //if there are more than 1000 bookmark results, the performance impact is noticeable
    if(bookmarksCount < 1000)
    {
        //add as many items to the common list as there are available entries in the dropdown list
        UrlSearchItem urlSearchItem;
        for(int i = 0; i < _history.count(); i++)
        {
            if (_bookmarks.removeOne(_history.at(i)))
            {
                urlSearchItem = _history.takeAt(i);
                urlSearchItem.type |= UrlSearchItem::Bookmark;
                common << urlSearchItem;
                commonCount++;
                if(commonCount >= availableEntries)
                {
                    break;
                }
            }
        }

        commonCount = common.count();
        if(commonCount >= availableEntries)
        {
            common = common.mid(0, availableEntries);
            _history = UrlSearchList();
            _bookmarks = UrlSearchList();
            availableEntries = 0;
        }
        else        //remove all items from the history and bookmarks list up to the remaining entries in the dropdown list
        {
            availableEntries -= commonCount;
            if(historyCount >= availableEntries)
            {
                _history = _history.mid(0, availableEntries);
            }
            if(bookmarksCount >= availableEntries)
            {
                _bookmarks = _bookmarks.mid(0, availableEntries);
            }
        }
    }
    else        //if there are too many bookmarks items, remove all items up to the remaining entries in the dropdown list
    {

        if(historyCount >= availableEntries)
        {
            _history = _history.mid(0, availableEntries);
        }
        if(bookmarksCount >= availableEntries)
        {
            _bookmarks = _bookmarks.mid(0, availableEntries);
        }

        UrlSearchItem urlSearchItem;
        for(int i = 0; i < _history.count(); i++)
        {
            if (_bookmarks.removeOne(_history.at(i)))
            {
                urlSearchItem = _history.takeAt(i);
                urlSearchItem.type |= UrlSearchItem::Bookmark;
                common << urlSearchItem;
            }
        }

        availableEntries -= common.count();
    }

    historyCount = _history.count();
    bookmarksCount = _bookmarks.count();
    commonCount = common.count();
    
    kDebug() << "HISTORY COUNT: " << historyCount;

    //now fill the list to MAX_ELEMENTS
    if(availableEntries > 0)
    {
        int historyEntries = ((int) (availableEntries / 2)) + availableEntries % 2;
        int bookmarksEntries = availableEntries - historyEntries;

        if (historyCount >= historyEntries && bookmarksCount >= bookmarksEntries)
        {
            _history = _history.mid(0, historyEntries);
            _bookmarks = _bookmarks.mid(0, bookmarksEntries);
        }
        else if (historyCount < historyEntries && bookmarksCount >= bookmarksEntries)
        {
            if(historyCount + bookmarksCount > availableEntries)
            {
                _bookmarks = _bookmarks.mid(0, availableEntries - historyCount);
            }
        }
        else if (historyCount >= historyEntries && bookmarksCount < bookmarksEntries)
        {
            if(historyCount + bookmarksCount > availableEntries)
            {
                _history = _history.mid(0, availableEntries - bookmarksCount);
            }
        }
    }

    availableEntries -=  _history.count();
    availableEntries -=  _bookmarks.count();

    if (_suggestions.count() > availableEntries + MIN_SUGGESTIONS)
    {
        _suggestions = _suggestions.mid(0, availableEntries + MIN_SUGGESTIONS);
    }

    list = list + _history + common + _bookmarks + _suggestions;
    qWarning() << "orderedSearchItems leave: " << " elapsed: " << myTime.elapsed();

    return list;
}


//////////////////////////////////////////////////////////////////////////
// PRIVATE ENGINES


//QUrl from User Input (easily the best solution... )
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


//webSearches
void UrlResolver::computeWebSearches()
{
    _webSearches = (UrlSearchList() << UrlSearchItem(UrlSearchItem::Search, QString(), QString()));
}


//history
void UrlResolver::computeHistory()
{
    QList<HistoryItem> found = Application::historyManager()->find(_typedString);
    qSort(found);

    Q_FOREACH(const HistoryItem &i, found)
    {
        if (_searchEnginesRegexp.indexIn(i.url) == -1) //filter all urls that are search engine results
        {
            UrlSearchItem gItem(UrlSearchItem::History, i.url, i.title);
            _history << gItem;
        }
    }
}


// bookmarks
void UrlResolver::computeBookmarks()
{
    QList<KBookmark> found = Application::bookmarkProvider()->find(_typedString);
    kDebug() << "FOUND: " << found.count();
    Q_FOREACH(const KBookmark &b, found)
    {
        UrlSearchItem gItem(UrlSearchItem::Bookmark, b.url().url(), b.fullText());
        _bookmarks << gItem;
    }
}


//opensearch suggestion
void UrlResolver::computeSuggestions()
{
    if (Application::opensearchManager()->isSuggestionAvailable())
    {
        connect(Application::opensearchManager(),
                SIGNAL(suggestionReceived(const QString &, const QStringList &)),
                this,
                SLOT(suggestionsReceived(const QString &, const QStringList &)));

        Application::opensearchManager()->requestSuggestion(_typedString);
    }
}


void UrlResolver::suggestionsReceived(const QString &text, const QStringList &suggestions)
{
    if(text != _typedString)
        return;
    
    UrlSearchList sugList;

    Q_FOREACH(const QString &s, suggestions)
    {
        UrlSearchItem gItem(UrlSearchItem::Suggestion, s, s);
        sugList << gItem;
    }
    emit suggestionsReady(sugList, _typedString);
    this->deleteLater();
}


UrlSearchItem UrlResolver::privilegedItem(UrlSearchList* list)
{
    UrlSearchItem item;
    QString dot = QString(QL1C('.'));
    QString test1 = QString(QL1C('/')) + _typedString + dot;
    QString test2 = dot + _typedString + dot;

    for(int i = 0; i<list->count(); i++)
    {
        item = list->at(i);
        //TODO: move this to AwesomeUrlCompletion::substringCompletion and add a priviledged flag to UrlSearchItem
        if (item.url.contains(test1) || item.url.contains(test2))
        {
            list->removeAt(i);
            return item;
        }
    }
    return UrlSearchItem();
}
