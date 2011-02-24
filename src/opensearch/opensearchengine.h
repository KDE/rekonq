/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2009 by Jakub Wieczorek <faw217@gmail.com>
* Copyright (C) 2009 by Christian Franke <cfchris6@ts2server.com>
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


#ifndef OPENSEARCHENGINE_H
#define OPENSEARCHENGINE_H


// Rekonq Includes
#include "rekonq_defines.h"

// Local Includes
#include "suggestionparser.h"

// KDE Includes
#include <KUrl>

// Qt Includes
#include <QtCore/QPair>
#include <QtGui/QImage>


class OpenSearchEngine : public QObject
{
    Q_OBJECT

public:
    typedef QPair<QString, QString> Parameter;

    OpenSearchEngine(QObject *parent = 0);
    ~OpenSearchEngine();

    QString name() const;
    void setName(const QString &name);

    QString description() const;
    void setDescription(const QString &description);

    QString searchUrlTemplate() const;
    void setSearchUrlTemplate(const QString &searchUrl);
    KUrl searchUrl(const QString &searchTerm) const;

    bool providesSuggestions() const;

    QString suggestionsUrlTemplate() const;
    void setSuggestionsUrlTemplate(const QString &suggestionsUrl);
    KUrl suggestionsUrl(const QString &searchTerm) const;

    QList<Parameter> searchParameters() const;
    void setSearchParameters(const QList<Parameter> &searchParameters);

    QList<Parameter> suggestionsParameters() const;
    void setSuggestionsParameters(const QList<Parameter> &suggestionsParameters);

    void setSuggestionParser(SuggestionParser *parser);

    QString imageUrl() const;
    void setImageUrl(const QString &url);

    QImage image() const;
    void setImage(const QImage &image);

    bool isValid() const;

    bool operator==(const OpenSearchEngine &other) const;
    bool operator<(const OpenSearchEngine &other) const;

    ResponseList parseSuggestion(const QString &searchTerm, const QByteArray &response);

    static QString parseTemplate(const QString &searchTerm, const QString &searchTemplate);

    QString type();

    bool hasCachedSuggestionsFor(const QString &searchTerm);

    ResponseList cachedSuggestionsFor(const QString &searchTerm);

private:
    QString m_name;
    QString m_description;

    QString m_imageUrl;
    QImage m_image;

    QString m_searchUrlTemplate;
    QString m_suggestionsUrlTemplate;
    QList<Parameter> m_searchParameters;
    QList<Parameter> m_suggestionsParameters;

    SuggestionParser *m_parser;

    QString suggestionPathFor(const QString &searchTerm);

    ResponseList parseSuggestion(const QByteArray &resp);
};

#endif // OPENSEARCHENGINE_H
