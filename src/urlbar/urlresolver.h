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


#ifndef URL_RESOLVER_H
#define URL_RESOLVER_H


// Rekonq Includes
#include "rekonq_defines.h"

// KDE Includes
#include <KUrl>
#include <KService>

// Qt Includes
#include <QString>
#include <QList>
#include <QStringList>

class UrlSearchItem
{
public:

    enum types
    {
        Undefined       = 0x00000000,
        Search          = 0x00000001,
        Browse          = 0x00000010,
        History         = 0x00000100,
        Bookmark        = 0x00001000,
        Suggestion      = 0x00010000,
    };

    int type;
    QString url;
    QString title;
    QString description;
    QString bookmarkPath;
    
    UrlSearchItem(const UrlSearchItem &item) : type(item.type),
                                               url(item.url),
                                               title(item.title),
                                               description(item.description)
    {};

    UrlSearchItem() : type(UrlSearchItem::Undefined),
                      url(QString()),
                      title(QString()),
                      description(QString())
    {};
    
    UrlSearchItem(const int &_type,
                  const QString &_url,
                  const QString &_title = QString(),
                  const QString   &description    = QString()
                  )
                  : type(_type),
                  url(_url),
                  title(_title),
                  description(description)
    {};

    inline bool operator==(const UrlSearchItem &i) const
    {
        return i.url == url;//TODO && i.title == title;
    }    
};

typedef QList <UrlSearchItem> UrlSearchList;


// ----------------------------------------------------------------------


class UrlResolver : public QObject
{
    Q_OBJECT

public:
    UrlResolver(const QString &typedUrl);

    UrlSearchList orderedSearchItems();

    static KService::Ptr searchEngine()
    {
        return _searchEngine;
    };

    static void setSearchEngine(KService::Ptr engine)
    {
        _searchEngine = engine;
    };

    void computeSuggestions();

private Q_SLOTS:
    void suggestionsReceived(const QString &text, const QStringList &suggestions);    

Q_SIGNALS:
    void suggestionsReady(const UrlSearchList &, const QString &);

private:
    QString _typedString;

    UrlSearchList _webSearches;
    UrlSearchList _qurlFromUserInput;
    UrlSearchList _history;
    UrlSearchList _bookmarks;
    UrlSearchList _suggestions;
    
    void computeWebSearches();
    void computeHistory();
    void computeQurlFromUserInput();
    void computeBookmarks();

    UrlSearchItem privilegedItem(UrlSearchList* list);
    UrlSearchList orderLists();

    static QRegExp _browseRegexp;
    static QRegExp _searchEnginesRegexp;

    static KService::Ptr _searchEngine;    
};

// ------------------------------------------------------------------------------

#endif // URL_RESOLVER_H
