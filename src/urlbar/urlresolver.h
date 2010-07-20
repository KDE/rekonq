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
    };

    int type;
    QString url;
    QString title;
    QString description;
    QString bookmarkPath;
    
    UrlSearchItem(const UrlSearchItem &item) : type(item.type),
                                               url(item.url),
                                               title(item.title),
                                               description(item.description),
                                               bookmarkPath(item.bookmarkPath)
    {};

    UrlSearchItem() : type(UrlSearchItem::Undefined),
                      url(QString()),
                      title(QString()),
                      description(QString()),
                      bookmarkPath(QString())
    {};
    
    UrlSearchItem(const int &_type,
                  const QString &_url,
                  const QString &_title = QString(),
                  const QString   &description    = QString(),
                  const QString   &bookmarkPath   = QString()
                  )
                  : type(_type),
                  url(_url),
                  title(_title),
                  description(description),
                  bookmarkPath(bookmarkPath)
    {};

    inline bool operator==(const UrlSearchItem &i) const
    {
        return i.url == url;//TODO && i.title == title;
    }
};

typedef QList <UrlSearchItem> UrlSearchList;


// ----------------------------------------------------------------------


class UrlResolver
{
public:
    UrlResolver(const QString &typedUrl);

    UrlSearchList orderedSearchItems();

private:
    QString _typedString;

    UrlSearchList webSearchesResolution();
    UrlSearchList historyResolution();
    UrlSearchList qurlFromUserInputResolution();
    UrlSearchList bookmarksResolution();
    UrlSearchItem privilegedItem(UrlSearchList* list);
     
    static QRegExp _browseRegexp;
};

// ------------------------------------------------------------------------------

#endif // URL_RESOLVER_H
