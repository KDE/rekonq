/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2009 by Jakub Wieczorek <faw217@gmail.com>
* Copyright (C) 2009 by Fredy Yanardi <fyanardi@gmail.com>
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
#include "opensearchreader.h"

// Local Includes
#include "opensearchengine.h"
#include "suggestionparser.h"

// KDE Includes
#include <KLocalizedString>

// Qt Includes
#include <QtCore/QIODevice>


OpenSearchReader::OpenSearchReader()
    : QXmlStreamReader()
{
}


OpenSearchEngine *OpenSearchReader::read(const QByteArray &data)
{
    clear();
    addData(data);

    return read();
}


OpenSearchEngine *OpenSearchReader::read(QIODevice *device)
{
    clear();

    if (!device->isOpen()) 
    {
        device->open(QIODevice::ReadOnly);
    }

    setDevice(device);
    return read();
}


OpenSearchEngine *OpenSearchReader::read()
{
    OpenSearchEngine *engine = new OpenSearchEngine();

    while (!isStartElement() && !atEnd()) 
    {
        readNext();
    }

    if (    name() != QL1S("OpenSearchDescription")
         || namespaceUri() != QL1S("http://a9.com/-/spec/opensearch/1.1/")
       ) 
    {
        raiseError(i18n("The file is not an OpenSearch 1.1 file."));
        return engine;
    }

    while (!(isEndElement() && name() == QL1S("OpenSearchDescription")) && !atEnd()) 
    {
        readNext();

        if (!isStartElement())
            continue;

        // ShortName
        if (name() == QL1S("ShortName")) 
        {
            engine->setName(readElementText());
            continue;
        }
        
        // Description
        if (name() == QL1S("Description")) 
        {
            engine->setDescription(readElementText());
            continue;
        }
        
        // Url
        if (name() == QL1S("Url")) 
        {
            QString type = attributes().value(QL1S("type")).toString();
            QString url = attributes().value(QL1S("template")).toString();

            if (url.isEmpty())
                continue;

            QList<OpenSearchEngine::Parameter> parameters;

            readNext();

            while (!(isEndElement() && name() == QL1S("Url"))) 
            {
                if (!isStartElement() 
                    || (name() != QL1S("Param") 
                    && name() != QL1S("Parameter"))) 
                {
                    readNext();
                    continue;
                }

                QString key = attributes().value(QL1S("name")).toString();
                QString value = attributes().value(QL1S("value")).toString();

                if (!key.isEmpty() && !value.isEmpty()) 
                {
                    parameters.append(OpenSearchEngine::Parameter(key, value));
                }

                while (!isEndElement()) 
                {
                    readNext();
                }
            }

            if (type == QL1S("text/html"))
            {
                engine->setSearchUrlTemplate(url);
                engine->setSearchParameters(parameters);
            }
            else
            {
                if (engine->suggestionsUrlTemplate().isEmpty() 
                    && type == QL1S("application/x-suggestions+json")) //note: xml is prefered
                {
                    engine->setSuggestionsUrlTemplate(url);
                    engine->setSuggestionsParameters(parameters);
                    engine->setSuggestionParser(new JSONParser());
                }
                else if (type == QL1S("application/x-suggestions+xml"))
                {
                    engine->setSuggestionsUrlTemplate(url);
                    engine->setSuggestionsParameters(parameters);
                    engine->setSuggestionParser(new XMLParser());
                }
            }
            
            continue;
        }
        
        // Image
        if (name() == QL1S("Image")) 
        {
             engine->setImageUrl(readElementText());
             continue;
        }

        // Engine check
        if (    !engine->name().isEmpty()
             && !engine->description().isEmpty()
             && !engine->suggestionsUrlTemplate().isEmpty()
             && !engine->searchUrlTemplate().isEmpty()
             && !engine->imageUrl().isEmpty()
           ) 
        {
            break;
        }
    }

    return engine;
}
