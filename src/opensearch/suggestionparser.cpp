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


// Self Includes
#include "suggestionparser.h"

// Qt Includes
#include<QByteArray>
#include<QStringList>


ResponseList SuggestionParser::parse(const QByteArray &)
{
    return ResponseList();
}


SuggestionParser::~SuggestionParser()
{
}


ResponseList XMLParser::parse(const QByteArray &resp)
{
    ResponseList rlist;

    m_reader.clear();
    m_reader.addData(resp);

    while (!m_reader.atEnd() && !m_reader.hasError())
    {
        m_reader.readNext();

        if (m_reader.isStartDocument()) 
            continue;

        if (m_reader.isStartElement() && m_reader.name() == QL1S("Item"))
        {
            QString title;
            QString description;
            QString url;
            QString image;
            int image_width=0;
            int image_height=0;

            m_reader.readNext();

            while( !(m_reader.isEndElement() && m_reader.name() == QL1S("Item")) )
            {
                if(m_reader.isStartElement())
                {

                    if (m_reader.name() == QL1S("Text")) 
                        title = m_reader.readElementText();
                    if (m_reader.name() == QL1S("Url")) 
                        url = m_reader.readElementText();

                    if (m_reader.name() == QL1S("Image"))
                    {
                        image = m_reader.attributes().value("source").toString();
                        image_width = m_reader.attributes().value("width").toString().toInt();
                        image_height = m_reader.attributes().value("height").toString().toInt();
                    }

                    if (m_reader.name() == QL1S("Description")) 
                        description = m_reader.readElementText();
                }

                m_reader.readNext();
            }
            rlist << Response(title, description, url, image, image_width, image_height);
        }

        m_reader.readNext();
    }

    return rlist;
}


ResponseList JSONParser::parse(const QByteArray &resp)
{
    QString response = QString::fromLocal8Bit(resp);
    response = response.trimmed();

    if (response.isEmpty())
    {
        kDebug() << "RESPONSE IS EMPTY";
        return ResponseList();
    }

    if (!response.startsWith(QL1C('['))
        || !response.endsWith(QL1C(']'))
    )
    {
        kDebug() << "RESPONSE is NOT well FORMED";
        return ResponseList();
    }

    // Evaluate the JSON response using QtScript.
    if (!m_reader.canEvaluate(response))
    {
        kDebug() << "m_reader cannot evaluate the response";
        return ResponseList();
    }

    QScriptValue responseParts = m_reader.evaluate(response);

    if (!responseParts.property(1).isArray())
    {
        kDebug() << "RESPONSE is not an array";
        return ResponseList();
    }

    ResponseList rlist;
    QStringList responsePartsList;
    qScriptValueToSequence(responseParts.property(1), responsePartsList);

    Q_FOREACH(const QString &s, responsePartsList)
    {
        rlist << Response(s);
    }

    return rlist;
}
