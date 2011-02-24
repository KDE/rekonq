/* ============================================================
 * 
 * This file is a part of the rekonq project
 *
 * Copyright (C) 2010-2011 by Lionel Chauvin <megabigbug@yahoo.fr>
 * Copyright (C) 2010-2011 by Andrea Diamantini <adjam7 at gmail dot com>
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


#ifndef SUGGESTIONPARSER_H
#define SUGGESTIONPARSER_H


// Rekonq Includes
#include "rekonq_defines.h"

// Qt Includes
#include <QtCore/QList>
#include <QtCore/QXmlStreamReader>

#include <QtScript/QScriptEngine>


class Response
{
public:
    QString title;
    QString description;
    QString url;
    QString image;
    int image_width;
    int image_height;

    Response(const Response &item) : title(item.title),
                                     description(item.description),
                                     url(item.url),
                                     image(item.image),
                                     image_width(item.image_width),
                                     image_height(item.image_height)

    {};

    Response() : title(QString()),
                 description(QString()),
                 url(QString()),
                 image(QString()),
                 image_width(0),
                 image_height(0)

    {};

    Response(const QString &_title = QString(),
             const QString &_description = QString(),
             const QString &_url  = QString(),
             const QString &_image = QString(),
             const int &_image_width = 0,
             const int &_image_height = 0) : title(_title),
                                             description(_description),
                                             url(_url),
                                             image(_image),
                                             image_width(_image_width),
                                             image_height(_image_height)
    {};
};


// -----------------------------------------------------------------


typedef QList <Response> ResponseList;


class SuggestionParser
{
public:
    virtual ~SuggestionParser();
    virtual ResponseList parse(const QByteArray &resp);
    virtual QString type() = 0;
};


class XMLParser : public SuggestionParser
{   
protected:
    QXmlStreamReader m_reader;

public:
    ResponseList parse(const QByteArray &resp);
    inline QString type() { return QL1S("application/x-suggestions+xml"); }
};


class JSONParser : public SuggestionParser
{
private:
    QScriptEngine m_reader;
    
public:
    ResponseList parse(const QByteArray &resp);
    inline QString type() { return QL1S("application/x-suggestions+json"); }
};

#endif //SUGGESTIONPARSER_H
