/* ============================================================
*
* This file is a part of the rekonq project
*
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


#ifndef OPENSEARCHMANAGER_H
#define OPENSEARCHMANAGER_H


// Rekonq Includes
#include "rekonq_defines.h"

// KDE Includes
#include <kio/jobclasses.h>

// Qt Includes
#include <QtCore/QObject>

// Forward Declarations
class SuggestionEngine;
class OpenSearchEngine;


/**
 * This class acts as a proxy between the SearchBar plugin 
 * and the individual suggestion engine.
 * This class has a map of all available engines, 
 * and route the suggestion request to the correct engine.
 */
class OpenSearchManager : public QObject
{
    Q_OBJECT

    enum STATE 
    {
        REQ_SUGGESTION,
        REQ_DESCRIPTION,
        IDLE
    };
    
public:
    /**
     * Constructor
     */
    explicit OpenSearchManager(QObject *parent = 0);

    virtual ~OpenSearchManager();

    void setSearchProvider(const QString &searchProvider);

    /**
     * Check whether a search suggestion engine is available for the given search provider
     * @param searchProvider the queried search provider
     */
    bool isSuggestionAvailable();

    void addOpenSearchEngine(const KUrl &url, const QString &title);

public slots:
    /**
     * Ask the specific suggestion engine to request for suggestion for the search text
     * 
     * @param searchText the text to be queried to the suggestion service
     */
    void requestSuggestion(const QString &searchText);

private slots:
    void dataReceived(KIO::Job *job, const QByteArray &data);
    void jobFinished(KJob *job);

signals:
    void suggestionReceived(const QString &text, const QStringList &suggestion);
    void openSearchEngineAdded(const QString &name, const QString &searchUrl, const QString &fileName);

private:
    QString trimmedEngineName(const QString &engineName) const;

    // QString substitutueSearchText(const QString &searchText, const QString &requestURL) const;
    QByteArray m_jobData;
    QMap<QString, OpenSearchEngine*> m_enginesMap;
    OpenSearchEngine *m_activeEngine;
    STATE m_state;
    
    QString _typedText;
};

#endif // OPENSEARCHMANAGER_H
