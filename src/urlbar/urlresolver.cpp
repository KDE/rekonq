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
#include "bookmarksmanager.h"

// KDE Includes
#include <KUriFilter>
#include <KCompletion>
#include <KService>
#include <KConfig>
#include <KConfigGroup>

// Qt Includes
#include <QByteArray>

// defines
#define MAX_ELEMENTS 10


// NOTE 
// default kurifilter plugin list (at least in my box):
// 1. "kshorturifilter"
// 2. "kurisearchfilter"
// 3. "localdomainurifilter"
// 4 ."kuriikwsfilter"
// 5. "fixhosturifilter"


// ------------------------------------------------------------------------


QRegExp UrlResolver::_browseRegexp;


UrlResolver::UrlResolver(const QString &typedUrl)
        : _typedString(typedUrl.trimmed())
{
    if ( _browseRegexp.isEmpty() )
    {
        kDebug() << "browse regexp empty. Setting value..";
        
        QString protocol = "^(http://|https://|file://|ftp://|man:|info:|apt:)";
        
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
    
}


UrlSearchList UrlResolver::orderedSearchItems()
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

    if( _typedString == QL1S("about:") )
    {
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
    
    if(_browseRegexp.indexIn(_typedString) != -1)
    {
        list << qurlFromUserInputResolution();
        list << webSearchesResolution();
    }
    else
    {
        list << webSearchesResolution();
        list << qurlFromUserInputResolution();
    }

    //find the history items that match the typed string
    UrlSearchList historyList = historyResolution();
    UrlSearchItem privileged = privilegedItem(&historyList);
    int historyResults = historyList.count();
    
    //find the bookmarks items that match the typed string
    UrlSearchList bookmarksList = bookmarksResolution();
    if (privileged.type == UrlSearchItem::Undefined)
    {
        privileged = privilegedItem(&bookmarksList);
    }
    else if(privileged.type == UrlSearchItem::History && bookmarksList.removeOne(privileged))
    {
        privileged.type |= UrlSearchItem::Bookmark;
    }
    int bookmarksResults = bookmarksList.count();
    
    if (privileged.type != UrlSearchItem::Undefined)
    {
        list.prepend(privileged);
    }
    
    int availableEntries = MAX_ELEMENTS - list.count();
    
    UrlSearchList commonList;
    int commonResutls = 0;

    //prefer items which are history items als well bookmarks item
    //if there are more than 1000 bookmark results, the performance impact is noticeable
    if(bookmarksResults < 1000)
    {
        //add as many items to the common list as there are available entries in the dropdown list
        UrlSearchItem urlSearchItem;
        for(int i = 0; i < historyList.count(); i++)
        {
            if (bookmarksList.removeOne(historyList.at(i)))
            {
                urlSearchItem = historyList.takeAt(i);
                urlSearchItem.type |= UrlSearchItem::Bookmark;
                commonList << urlSearchItem;
                commonResutls++;
                if(commonResutls >= availableEntries)
                {
                    break;
                }
            }
        }
        
        commonResutls = commonList.count();
        if(commonResutls >= availableEntries)
        {
            commonList = commonList.mid(0, availableEntries);
            historyList = UrlSearchList();
            bookmarksList = UrlSearchList();
            availableEntries = 0;
        }
        else        //remove all items from the history and bookmarks list up to the remaining entries in the dropdown list
        {
            availableEntries -= commonResutls;
            if(historyResults >= availableEntries)
            {
                historyList = historyList.mid(0, availableEntries);
            }
            if(bookmarksResults >= availableEntries)
            {
                bookmarksList = bookmarksList.mid(0, availableEntries);
            }
        }
    }
    else        //if there are too many bookmarks items, remove all items up to the remaining entries in the dropdown list
    {
        
        if(historyResults >= availableEntries)
        {
            historyList = historyList.mid(0, availableEntries);
        }
        if(bookmarksResults >= availableEntries)
        {
            bookmarksList = bookmarksList.mid(0, availableEntries);
        }

        UrlSearchItem urlSearchItem;
        for(int i = 0; i < historyList.count(); i++)
        {
            if (bookmarksList.removeOne(historyList.at(i)))
            {
                urlSearchItem = historyList.takeAt(i);
                urlSearchItem.type |= UrlSearchItem::Bookmark;
                commonList << urlSearchItem;
            }
        }
        
        availableEntries -= commonList.count();
    }
    
    historyResults = historyList.count();
    bookmarksResults = bookmarksList.count();
    commonResutls = commonList.count();
    
    //now fill the list to MAX_ELEMENTS
    if(availableEntries > 0)
    {
        int historyEntries = ((int) (availableEntries / 2)) + availableEntries % 2;
        int bookmarksEntries = availableEntries - historyEntries;
        
        if (historyResults >= historyEntries && bookmarksResults >= bookmarksEntries)
        {
            historyList = historyList.mid(0, historyEntries);
            bookmarksList = bookmarksList.mid(0, bookmarksEntries);
        }
        else if (historyResults < historyEntries && bookmarksResults >= bookmarksEntries)
        {
            if(historyResults + bookmarksResults > availableEntries)
            {
                bookmarksList = bookmarksList.mid(0, availableEntries - historyResults);
            }
        }
        else if (historyResults >= historyEntries && bookmarksResults < bookmarksEntries)
        {
            if(historyResults + bookmarksResults > availableEntries)
            {
                historyList = historyList.mid(0, availableEntries - bookmarksResults);
            }
        }
    }
    
    list = list + historyList + commonList + bookmarksList;
    qWarning() << "orderedSearchItems leave: " << " elapsed: " << myTime.elapsed();
    
    return list;
}


//////////////////////////////////////////////////////////////////////////
// PRIVATE ENGINES


// STEP 1 = QUrl from User Input (easily the best solution... )
UrlSearchList UrlResolver::qurlFromUserInputResolution()
{
    UrlSearchList list;
    QString url2 = _typedString;
    QUrl urlFromUserInput = QUrl::fromUserInput(url2);
    if (urlFromUserInput.isValid())
    {
        QString gTitle = i18nc("Browse a website", "Browse");
        UrlSearchItem gItem(UrlSearchItem::Browse, urlFromUserInput.toString(), gTitle);
        list << gItem;
    }

    return list;
}


// STEP 2 = Web Searches
UrlSearchList UrlResolver::webSearchesResolution()
{
    return UrlSearchList() << UrlSearchItem(UrlSearchItem::Search, QString(), QString());
}


// STEP 3 = history completion
UrlSearchList UrlResolver::historyResolution()
{
    QList<HistoryHashItem> mostVisited = Application::historyManager()->findMostVisited(_typedString);
    UrlSearchList list;
    foreach (HistoryHashItem i, mostVisited)
    {
        UrlSearchItem gItem(UrlSearchItem::History, i.url, i.title);
        list << gItem;
    }
    return list;
}


// STEP 4 = bookmarks completion
UrlSearchList UrlResolver::bookmarksResolution()
{
    UrlSearchList list;
    //AwesomeUrlCompletion *bookmarkCompletion = Application::bookmarkProvider()->completionObject();
    return list;
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
        //kDebug() << item.url.host();
        //TODO: move this to AwesomeUrlCompletion::substringCompletion and add a priviledged flag to UrlSearchItem
        if (item.url.contains(test1) || item.url.contains(test2))
        {
            list->removeAt(i);
            return item;
        }
    }
    return UrlSearchItem();
}
