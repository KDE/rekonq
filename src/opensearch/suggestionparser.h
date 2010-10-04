/* ============================================================
 * 
 * This file is a part of the rekonq project
 *
 * Copyright (C) 2010 by Lionel Chauvin <megabigbug@yahoo.fr>
 * Copyright (C) 2010 by Andrea Diamantini <adjam7 at gmail dot com>
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

// Self Includes

// Local Includes


// Qt Includes
#include <QList>
#include <QtScript/QScriptEngine>
#include <QtCore/QXmlStreamReader>


class Response
{
public:
    QString url;
    QString title;
    QString description;
  
    Response(const Response &item) : url(item.url),
                                     title(item.title),
                                     description(item.description)
    {};

    Response() : url(QString()),
                 title(QString()),
                 description(QString())
    {};

    Response(const QString &_url,
             const QString &_title = QString(),
             const QString   &description    = QString()) : url(_url),
                                                            title(_title),
                                                            description(description)
    {};
};

typedef QList <Response> ResponseList;


class SuggestionParser
{
public:
    virtual ~SuggestionParser();
    virtual ResponseList parse(const QByteArray &resp);
};


class XMLParser : public SuggestionParser
{   
protected:
    QXmlStreamReader m_reader;

public:
    ResponseList parse(const QByteArray &resp);
};

class JSONParser : public SuggestionParser
{
private:
    QScriptEngine m_reader;
    
public:
    ResponseList parse(const QByteArray &resp);
};

#endif //SUGGESTIONPARSER_H