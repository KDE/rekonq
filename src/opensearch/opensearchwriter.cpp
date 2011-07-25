/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2009 by Jakub Wieczorek <faw217@gmail.com>
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


// Self Includes
#include "opensearchwriter.h"

// Local Includes
#include "opensearchengine.h"

// Qt Includes
#include <QtCore/QIODevice>


OpenSearchWriter::OpenSearchWriter()
    : QXmlStreamWriter()
{
    setAutoFormatting(true);
}


bool OpenSearchWriter::write(QIODevice *device, OpenSearchEngine *engine)
{
    if (!engine)
        return false;

    if (!device->isOpen())
        device->open(QIODevice::WriteOnly);

    setDevice(device);
    write(engine);
    return true;
}


void OpenSearchWriter::write(OpenSearchEngine *engine)
{
    writeStartDocument();
    writeStartElement(QL1S("OpenSearchDescription"));
    writeDefaultNamespace(QL1S("http://a9.com/-/spec/opensearch/1.1/"));

    if (!engine->name().isEmpty())
    {
        writeTextElement(QL1S("ShortName"), engine->name());
    }

    if (!engine->description().isEmpty())
    {
        writeTextElement(QL1S("Description"), engine->description());
    }

    if (!engine->searchUrlTemplate().isEmpty())
    {
        writeStartElement(QL1S("Url"));
        writeAttribute(QL1S("method"), QL1S("get"));
        writeAttribute(QL1S("template"), engine->searchUrlTemplate());

        if (!engine->searchParameters().empty())
        {
            writeNamespace(QL1S("http://a9.com/-/spec/opensearch/extensions/parameters/1.0/"), QL1S("p"));

            QList<OpenSearchEngine::Parameter>::const_iterator end = engine->searchParameters().constEnd();
            QList<OpenSearchEngine::Parameter>::const_iterator i = engine->searchParameters().constBegin();
            for (; i != end; ++i)
            {
                writeStartElement(QL1S("p:Parameter"));
                writeAttribute(QL1S("name"), i->first);
                writeAttribute(QL1S("value"), i->second);
                writeEndElement();
            }
        }

        writeEndElement();
    }

    if (!engine->suggestionsUrlTemplate().isEmpty())
    {
        writeStartElement(QL1S("Url"));
        writeAttribute(QL1S("method"), QL1S("get"));
        writeAttribute(QL1S("type"), engine->type());
        writeAttribute(QL1S("template"), engine->suggestionsUrlTemplate());

        if (!engine->suggestionsParameters().empty())
        {
            writeNamespace(QL1S("http://a9.com/-/spec/opensearch/extensions/parameters/1.0/"), QL1S("p"));

            QList<OpenSearchEngine::Parameter>::const_iterator end = engine->suggestionsParameters().constEnd();
            QList<OpenSearchEngine::Parameter>::const_iterator i = engine->suggestionsParameters().constBegin();
            for (; i != end; ++i)
            {
                writeStartElement(QL1S("p:Parameter"));
                writeAttribute(QL1S("name"), i->first);
                writeAttribute(QL1S("value"), i->second);
                writeEndElement();
            }
        }

        writeEndElement();
    }

    if (!engine->imageUrl().isEmpty())
        writeTextElement(QL1S("Image"), engine->imageUrl());

    writeEndElement();
    writeEndDocument();
}
