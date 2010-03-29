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
#include <KDebug>
#include <KService>
#include <KConfig>
#include <KConfigGroup>

// Qt Includes
#include <QString>
#include <QByteArray>
#include <QUrl>


// NOTE default kurifilter plugin list (at least in my box)
// 1. "kshorturifilter"
// 2. "kurisearchfilter"
// 3. "localdomainurifilter"
// 4 ."kuriikwsfilter"
// 5. "fixhosturifilter"


UrlResolver::UrlResolver(const QString &typedUrl)
    : _urlString(typedUrl)
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
    
    if(historyResults + bookmarkResults > checkPoint)
    {
        historyList = historyList.mid(0,3);
        bookmarksList = bookmarksList.mid(0,3);
    }
    list << historyList;
    list << bookmarksList;
    
    return list;
}


bool UrlResolver::isHttp()
{
    QString r = "[\\d\\w-.]+\\.(a[cdefgilmnoqrstuwz]|b[abdefghijmnorstvwyz]|"\
    "c[acdfghiklmnoruvxyz]|d[ejkmnoz]|e[ceghrst]|f[ijkmnor]|g[abdefghilmnpqrstuwy]|"\
    "h[kmnrtu]|i[delmnoqrst]|j[emop]|k[eghimnprwyz]|l[abcikrstuvy]|"\
    "m[acdghklmnopqrstuvwxyz]|n[acefgilopruz]|om|p[aefghklmnrstwy]|qa|r[eouw]|"\
    "s[abcdeghijklmnortuvyz]|t[cdfghjkmnoprtvwz]|u[augkmsyz]|v[aceginu]|w[fs]|"\
    "y[etu]|z[amw]|aero|arpa|biz|com|coop|edu|info|int|gov|mil|museum|name|net|org|"\
    "pro)";

    return (QRegExp(r, Qt::CaseInsensitive).indexIn(_urlString) != -1) 
            || _urlString.startsWith("http:") 
            || _urlString.startsWith("https:");
}

//////////////////////////////////////////////////////////////////////////
// PRIVATE ENGINES


// STEP 1 = QUrl from User Input (easily the best solution... )
UrlSearchList UrlResolver::qurlFromUserInputResolution()
{
    UrlSearchList list;
    
    QString url2 = _urlString;
    QUrl urlFromUserInput = QUrl::fromUserInput(url2);
    if(urlFromUserInput.isValid())
    {
        QByteArray ba = urlFromUserInput.toEncoded();
        if(!ba.isEmpty())
        {           
            QString gUrl = QString(ba);
            QString gTitle = i18n("Browse");
            UrlSearchItem gItem(gUrl, gTitle, QString("") );
            list << gItem;

        }
    }
    
    return list;
}


// STEP 2 = Web Searches
UrlSearchList UrlResolver::webSearchesResolution()
{
    UrlSearchList list;
    
    QString url1 = _urlString;
    if(KUrl(url1).isRelative())
    {
        // KUriFilter has the worst performance possible here and let this trick unusable
        QString gUrl = QString("http://www.google.com/search?q=%1&ie=UTF-8&oe=UTF-8").arg(url1);
        QString gTitle = i18n("Search Google for ") + url1;
        UrlSearchItem gItem(gUrl, gTitle, QString("http://www.google.com") );
        list << gItem;
        
//         QString wUrl = QString("http://en.wikipedia.org/wiki/Special:Search?search=%1&go=Go").arg(url1);
//         QString wTitle = i18n("Search Wikipedia for ") + url1;
//         UrlSearchItem wItem(wUrl, wTitle, QString("http://wikipedia.org") );
//         list << wItem;
    }

    return list;
}


// STEP 3 = history completion
UrlSearchList UrlResolver::historyResolution()
{
    UrlSearchList list;
    
    KCompletion *historyCompletion = Application::historyManager()->completionObject();
    QStringList historyResults = historyCompletion->substringCompletion(_urlString);
    Q_FOREACH(const QString &s, historyResults)
    {
        UrlSearchItem it(s, s, QString("view-history"));
        list << it;
    }

    return list;
}


// STEP 4 = bookmarks completion
UrlSearchList UrlResolver::bookmarksResolution()
{
    UrlSearchList list;
    
    KCompletion *bookmarkCompletion = Application::bookmarkProvider()->completionObject();
    QStringList bookmarkResults = bookmarkCompletion->substringCompletion(_urlString);
    Q_FOREACH(const QString &s, bookmarkResults)
    {
        UrlSearchItem it( s, QString(), QString("rating") );
        list << it;
    }

    return list;
}
