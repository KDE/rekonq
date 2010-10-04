/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2009 by Jakub Wieczorek <faw217@gmail.com>
* Copyright (C) 2009 by Christian Franke <cfchris6@ts2server.com>
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
#include "opensearchengine.h"

// Qt Includes
#include <QtCore/QRegExp>
#include <QtCore/QStringList>
#include <QtScript/QScriptEngine>
#include <QtScript/QScriptValue>


OpenSearchEngine::OpenSearchEngine(QObject *)
    : m_scriptEngine(0)
{
}


OpenSearchEngine::~OpenSearchEngine()
{
    if (m_scriptEngine) 
    {
        delete m_scriptEngine;
    }
}


QString OpenSearchEngine::parseTemplate(const QString &searchTerm, const QString &searchTemplate)
{
    QString language = QLocale().name();
    // Simple conversion to RFC 3066.
    language = language.replace(QL1C('_'), QL1C('-'));

    QString result = searchTemplate;
    result.replace(QL1S("{count}"), QL1S("20"));
    result.replace(QL1S("{startIndex}"), QL1S("0"));
    result.replace(QL1S("{startPage}"), QL1S("0"));
    result.replace(QL1S("{language}"), language);
    result.replace(QL1S("{inputEncoding}"), QL1S("UTF-8"));
    result.replace(QL1S("{outputEncoding}"), QL1S("UTF-8"));
    result.replace(QL1S("{searchTerms}"), searchTerm);

    return result;
}


QString OpenSearchEngine::name() const
{
    return m_name;
}


void OpenSearchEngine::setName(const QString &name)
{
    m_name = name;
}


QString OpenSearchEngine::description() const
{
    return m_description;
}


void OpenSearchEngine::setDescription(const QString &description)
{
    m_description = description;
}


QString OpenSearchEngine::searchUrlTemplate() const
{
    return m_searchUrlTemplate;
}


void OpenSearchEngine::setSearchUrlTemplate(const QString &searchUrlTemplate)
{
    m_searchUrlTemplate = searchUrlTemplate;
}


KUrl OpenSearchEngine::searchUrl(const QString &searchTerm) const
{
    if (m_searchUrlTemplate.isEmpty()) 
    {
        return KUrl();
    }

    KUrl retVal = KUrl::fromEncoded(parseTemplate(searchTerm, m_searchUrlTemplate).toUtf8());

    QList<Parameter>::const_iterator i;
    for ( i = m_searchParameters.constBegin(); i != m_searchParameters.constEnd(); ++i) 
    {
        retVal.addQueryItem(i->first, parseTemplate(searchTerm, i->second));
    }

    return retVal;
}


bool OpenSearchEngine::providesSuggestions() const
{
    return !m_suggestionsUrlTemplate.isEmpty();
}


QString OpenSearchEngine::suggestionsUrlTemplate() const
{
    return m_suggestionsUrlTemplate;
}


void OpenSearchEngine::setSuggestionsUrlTemplate(const QString &suggestionsUrlTemplate)
{
    m_suggestionsUrlTemplate = suggestionsUrlTemplate;
}


KUrl OpenSearchEngine::suggestionsUrl(const QString &searchTerm) const
{
    if (m_suggestionsUrlTemplate.isEmpty()) 
    {
        return KUrl();
    }

    KUrl retVal = KUrl::fromEncoded(parseTemplate(searchTerm, m_suggestionsUrlTemplate).toUtf8());

    QList<Parameter>::const_iterator i;
    for( i = m_suggestionsParameters.constBegin(); i != m_suggestionsParameters.constEnd(); ++i)
    {
        retVal.addQueryItem(i->first, parseTemplate(searchTerm, i->second));
    }
    return retVal;
}


QList<OpenSearchEngine::Parameter> OpenSearchEngine::searchParameters() const
{
    return m_searchParameters;
}


void OpenSearchEngine::setSearchParameters(const QList<Parameter> &searchParameters)
{
    m_searchParameters = searchParameters;
}


QList<OpenSearchEngine::Parameter> OpenSearchEngine::suggestionsParameters() const
{
    return m_suggestionsParameters;
}


void OpenSearchEngine::setSuggestionsParameters(const QList<Parameter> &suggestionsParameters)
{
    m_suggestionsParameters = suggestionsParameters;
}


QString OpenSearchEngine::imageUrl() const
{
    return m_imageUrl;
}


void OpenSearchEngine::setImageUrl(const QString &imageUrl)
{
    m_imageUrl = imageUrl;
}


QImage OpenSearchEngine::image() const
{
    return m_image;
}


void OpenSearchEngine::setImage(const QImage &image)
{
    m_image = image;
}


bool OpenSearchEngine::isValid() const
{
    return (!m_name.isEmpty() && !m_searchUrlTemplate.isEmpty());
}


bool OpenSearchEngine::operator==(const OpenSearchEngine &other) const
{
    return (m_name == other.m_name
            && m_description == other.m_description
            && m_imageUrl == other.m_imageUrl
            && m_searchUrlTemplate == other.m_searchUrlTemplate
            && m_suggestionsUrlTemplate == other.m_suggestionsUrlTemplate
            && m_searchParameters == other.m_searchParameters
            && m_suggestionsParameters == other.m_suggestionsParameters);
}


bool OpenSearchEngine::operator<(const OpenSearchEngine &other) const
{
    return (m_name < other.m_name);
}


QStringList OpenSearchEngine::parseSuggestion(const QByteArray &resp)
{
    QString response = QString::fromLocal8Bit(resp);
    response = response.trimmed();

    if (response.isEmpty()) 
    {
        return QStringList();
    }

    if (    !response.startsWith(QL1C('[')) 
         || !response.endsWith(QL1C(']'))
       ) 
    {
        return QStringList();
    }

    if (!m_scriptEngine) 
    {
        m_scriptEngine = new QScriptEngine();
    }

    // Evaluate the JSON response using QtScript.
    if (!m_scriptEngine->canEvaluate(response)) 
    {
        return QStringList();
    }

    QScriptValue responseParts = m_scriptEngine->evaluate(response);

    if (!responseParts.property(1).isArray()) 
    {
        return QStringList();
    }

    QStringList suggestionsList;
    qScriptValueToSequence(responseParts.property(1), suggestionsList);

    return suggestionsList;
}
