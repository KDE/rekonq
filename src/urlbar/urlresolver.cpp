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
#define MAX_ELEMENTS 9


// NOTE default kurifilter plugin list (at least in my box)
// 1. "kshorturifilter"
// 2. "kurisearchfilter"
// 3. "localdomainurifilter"
// 4 ."kuriikwsfilter"
// 5. "fixhosturifilter"


bool UrlSearchItem::operator==(const UrlSearchItem &i) const
{
    return url == i.url;
}


UrlResolver::UrlResolver(const QString &typedUrl)
        : _typedString(typedUrl.trimmed())
{
}


UrlSearchList UrlResolver::orderedSearchItems()
{
    // NOTE: the logic here is : "we wanna suggest (at least) 9 elements"
    // so we have (more or less) 3 from first results (1 from QUrl Resolutions, 2 from
    // default search engines).
    // There are 6 remaining: if bookmarkResults + historyResults <= 6, catch all, else
    // catch first 3 results from the two resulting lists :)

    UrlSearchList list;
    
    if(isHttp())
    {
        list << qurlFromUserInputResolution();
        list << webSearchesResolution();
    }
    else
    {
        list << webSearchesResolution();
        list << qurlFromUserInputResolution();
    }

    
    int firstResults = list.count();
    int checkPoint = 9 - firstResults;

    UrlSearchList historyList = historyResolution();
    int historyResults = historyList.count();

    UrlSearchList bookmarksList = bookmarksResolution();
    int bookmarkResults = bookmarksList.count();

    if (historyResults + bookmarkResults > checkPoint)
    {
        historyList = historyList.mid(0, 3);
        bookmarksList = bookmarksList.mid(0, 3);
    }

    QList<UrlSearchItem> common;

    foreach(UrlSearchItem i, historyList)
    {
        if (!bookmarksList.contains(i))
        {
            list << i;
        }
        else
        {
            i.type |= UrlSearchItem::Bookmark;
            common << i;
        }
    }

    foreach(const UrlSearchItem &i, common)
    {
        list << i;
    }

    foreach(const UrlSearchItem &i, bookmarksList)
    {
        if (!common.contains(i))
            list << i;
    }

    list = placeTypedDomaineNameOnTop(list);

    return list;
}


bool UrlResolver::isHttp()
{
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

    return _typedString.startsWith( QL1S("http://") )
           || _typedString.startsWith( QL1S("https://") )
           || (QRegExp(address, Qt::CaseInsensitive).indexIn(_typedString) != -1)
           || (QRegExp(ipv4, Qt::CaseInsensitive).indexIn(_typedString) != -1)
           || (QRegExp(ipv6, Qt::CaseInsensitive).indexIn(_typedString) != -1);
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
        UrlSearchItem gItem(UrlSearchItem::Browse, urlFromUserInput, gTitle);
        list << gItem;
    }

    return list;
}


// STEP 2 = Web Searches
UrlSearchList UrlResolver::webSearchesResolution()
{
    return UrlSearchList() << UrlSearchItem(UrlSearchItem::Search, KUrl(), QString());
}


// STEP 3 = history completion
UrlSearchList UrlResolver::historyResolution()
{
    UrlSearchList list;

    KCompletion *historyCompletion = Application::historyManager()->completionObject();
    QStringList historyResults = historyCompletion->substringCompletion(_typedString);
    Q_FOREACH(const QString &s, historyResults)
    {
        UrlSearchItem it(UrlSearchItem::History, KUrl(s), Application::historyManager()->titleForHistoryUrl(s));
        list << it;
    }

    return list;
}


// STEP 4 = bookmarks completion
UrlSearchList UrlResolver::bookmarksResolution()
{
    UrlSearchList list;

    KCompletion *bookmarkCompletion = Application::bookmarkProvider()->completionObject();
    QStringList bookmarkResults = bookmarkCompletion->substringCompletion(_typedString);
    Q_FOREACH(const QString &s, bookmarkResults)
    {
        UrlSearchItem it(UrlSearchItem::Bookmark, KUrl(s), Application::bookmarkProvider()->titleForBookmarkUrl(s));
        list << it;
    }

    return list;
}


UrlSearchList UrlResolver::placeTypedDomaineNameOnTop(UrlSearchList list)
{
    int i=0;
    bool found = false;

    while(i<list.count() && !found)
    {
        UrlSearchItem item = list.at(i);
        if (item.url.url().contains("."+_typedString+".") || item.url.url().contains("/"+_typedString+"."))
        {
            list.removeAt(i);
            list.insert(0,item);
            found = true;
        }
        i++;
    }
    
    return list;
}

