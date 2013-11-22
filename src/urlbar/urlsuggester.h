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


#ifndef URL_SUGGESTER_H
#define URL_SUGGESTER_H


// Rekonq Includes
#include "rekonq_defines.h"

// KDE Includes
#include <KUrl>
#include <KService>

// Qt Includes
#include <QString>
#include <QList>


class UrlSuggestionItem
{
public:

    enum types
    {
        Undefined       = 0x00000000,
        Search          = 0x00000001,
        Browse          = 0x00000010,
        History         = 0x00000100,
        Bookmark        = 0x00001000
    };

    int type;
    QString url;
    QString title;
    QString description;
    QString bookmarkPath;

    UrlSuggestionItem(const UrlSuggestionItem &item)
        : type(item.type)
        , url(item.url)
        , title(item.title)
        , description(item.description)
    {};

    UrlSuggestionItem()
        : type(UrlSuggestionItem::Undefined)
        , url(QString())
        , title(QString())
        , description(QString())
    {};

    UrlSuggestionItem(const int &_type,
                      const QString &_url,
                      const QString &_title = QString(),
                      const QString &_description = QString()
                     )
        : type(_type)
        , url(_url)
        , title(_title)
        , description(_description)
    {};

    inline bool operator==(const UrlSuggestionItem &i) const
    {
        return i.url == url;//TODO && i.title == title;
    }
};


typedef QList <UrlSuggestionItem> UrlSuggestionList;


// ----------------------------------------------------------------------


class UrlSuggester : public QObject
{
    Q_OBJECT

public:
    explicit UrlSuggester(const QString &typedUrl);

    UrlSuggestionList computeSuggestions();

private:
    void computeWebSearches();
    void computeHistory();
    void computeQurlFromUserInput();
    void computeBookmarks();

    void removeBookmarksDuplicates();

    UrlSuggestionList orderLists();

    QString _typedString;

    UrlSuggestionList _webSearches;
    UrlSuggestionList _qurlFromUserInput;
    UrlSuggestionList _history;
    UrlSuggestionList _bookmarks;
    UrlSuggestionList _suggestions;

    bool _isKDEShortUrl;

    static QRegExp _browseRegexp;
    static QRegExp _searchEnginesRegexp;
};

// ------------------------------------------------------------------------------

#endif // URL_SUGGESTER_H
