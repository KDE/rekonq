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


#ifndef URL_RESOLVER_H
#define URL_RESOLVER_H


// Rekonq Includes
#include "rekonq_defines.h"

// Locale Includes
#include "application.h"
#include "opensearchmanager.h"
#include "suggestionparser.h"

// KDE Includes
#include <KUrl>
#include <KService>

// Qt Includes
#include <QString>
#include <QList>


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
        Suggestion      = 0x00010000
    };

    int type;
    QString url;
    QString title;
    QString description;
    QString image;
    int image_width;
    int image_height;
    QString bookmarkPath;

    UrlSearchItem(const UrlSearchItem &item)
        : type(item.type)
        , url(item.url)
        , title(item.title)
        , description(item.description)
        , image(item.image)
        , image_width(item.image_width)
        , image_height(item.image_height)
    {};

    UrlSearchItem()
        : type(UrlSearchItem::Undefined)
        , url(QString())
        , title(QString())
        , description(QString())
        , image(QString())
        , image_width(0)
        , image_height(0)
    {};

    UrlSearchItem(const int &_type,
                  const QString &_url,
                  const QString &_title = QString(),
                  const QString &_description = QString(),
                  const QString &_image = QString(),
                  const int &_image_width = 0,
                  const int &_image_height = 0
                 )
        : type(_type)
        , url(_url)
        , title(_title)
        , description(_description)
        , image(_image)
        , image_width(_image_width)
        , image_height(_image_height)
    {};

    inline bool operator==(const UrlSearchItem &i) const
    {
        return i.url == url;//TODO && i.title == title;
    }
};

typedef QList <UrlSearchItem> UrlSearchList;


// ----------------------------------------------------------------------

class HistoryItem;

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
        if (engine)
            rApp->opensearchManager()->setSearchProvider(engine->desktopEntryName());
    };

    void computeSuggestions();

    static bool isHistoryItemRelevant(const HistoryItem &a, const HistoryItem &b);

private Q_SLOTS:
    void suggestionsReceived(const QString &text, const ResponseList &suggestions);

Q_SIGNALS:
    void suggestionsReady(const UrlSearchList &, const QString &);

private:
    void computeWebSearches();
    void computeHistory();
    void computeQurlFromUserInput();
    void computeBookmarks();

    UrlSearchList orderLists();

    QString _typedString;
    QString _typedQuery;

    UrlSearchList _webSearches;
    UrlSearchList _qurlFromUserInput;
    UrlSearchList _history;
    UrlSearchList _bookmarks;
    UrlSearchList _suggestions;

    static QRegExp _browseRegexp;
    static QRegExp _searchEnginesRegexp;

    static KService::Ptr _searchEngine;

    bool _isKDEUrl;
};

// ------------------------------------------------------------------------------

#endif // URL_RESOLVER_H
