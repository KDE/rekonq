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


// Self Includes
#include "opensearchmanager.h"
#include "opensearchmanager.moc"

// Local Includes
#include "opensearchengine.h"
#include "opensearchreader.h"
#include "opensearchwriter.h"

// KDE Includes
#include <KDebug>
#include <KGlobal>
#include <KStandardDirs>
#include <KUrl>
#include <kio/scheduler.h>

// Qt Includes
#include <QtCore/QFile>


OpenSearchManager::OpenSearchManager(QObject *parent)
    : QObject(parent)
    , m_activeEngine(0)
    , m_currentJob(0)
{
    m_state = IDLE;
}


OpenSearchManager::~OpenSearchManager() 
{
    qDeleteAll(m_enginesMap.values());
    m_enginesMap.clear();
}


void OpenSearchManager::setSearchProvider(const QString &searchProvider)
{
    m_activeEngine = 0;

    if (!m_enginesMap.contains(searchProvider)) 
    {
        const QString fileName = KGlobal::dirs()->findResource("data", "rekonq/opensearch/" + searchProvider + ".xml");
        kDebug() << searchProvider <<  " file name path: " << fileName;
        if (fileName.isEmpty()) 
        {
            kDebug() << "OpenSearch file name empty";
            return;
        }
        QFile file(fileName);

        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) 
        {
            kDebug() << "Cannot open opensearch description file: " + fileName;
            return;
        }

        OpenSearchReader reader;
        OpenSearchEngine *engine = reader.read(&file);

        if (engine) 
        {
            m_enginesMap.insert(searchProvider, engine);
        }
        else 
        {
            return;
        }
    }

    m_activeEngine = m_enginesMap.value(searchProvider);
}


bool OpenSearchManager::isSuggestionAvailable()
{
    return m_activeEngine != 0;
}


void OpenSearchManager::addOpenSearchEngine(const KUrl &url, const QString &title)
{
    Q_UNUSED(title);

    if (m_state != IDLE) 
    {
        idleJob();
    }

    m_currentJob = KIO::get(url, KIO::NoReload, KIO::HideProgressInfo);
    m_state = REQ_DESCRIPTION;
    connect(m_currentJob, SIGNAL(data(KIO::Job *, const QByteArray &)), this, SLOT(dataReceived(KIO::Job *, const QByteArray &)));
    connect(m_currentJob, SIGNAL(result(KJob *)), this, SLOT(jobFinished(KJob *)));
}


void OpenSearchManager::requestSuggestion(const QString &searchText)
{
    if (!m_activeEngine) 
        return;

    if (m_state != IDLE) 
    {
        // NOTE: 
        // changing OpenSearchManager behavior
        // using idleJob here lets opensearchmanager to start another search, while
        // if we want in any case lets it finish its previous job we can just return here.
        idleJob();
    }
    
    _typedText = searchText;

    KUrl url = m_activeEngine->suggestionsUrl(searchText);
    kDebug() << "Requesting for suggestions: " << url.url();

    m_currentJob = KIO::get(url, KIO::NoReload, KIO::HideProgressInfo);
    m_state = REQ_SUGGESTION;
    connect(m_currentJob, SIGNAL(data(KIO::Job *, const QByteArray &)), this, SLOT(dataReceived(KIO::Job *, const QByteArray &)));
    connect(m_currentJob, SIGNAL(result(KJob *)), this, SLOT(jobFinished(KJob *)));
}


void OpenSearchManager::dataReceived(KIO::Job *job, const QByteArray &data)
{
    Q_UNUSED(job);
    m_jobData.append(data);
}


void OpenSearchManager::jobFinished(KJob *job)
{
    if (job->error())
    {
        emit suggestionReceived(_typedText, ResponseList());
        m_state = IDLE;
        return; // just silently return
    }
    
    if (m_state == REQ_SUGGESTION) 
    {
        const ResponseList suggestionsList = m_activeEngine->parseSuggestion(m_jobData);
        kDebug() << "Received suggestions in "<< _typedText << " from " << m_activeEngine->name() << ": ";
        foreach(Response r, suggestionsList)
        {
            kDebug() << r.title; 
        }

        emit suggestionReceived(_typedText, suggestionsList);
        idleJob();
        return;
    }

    if (m_state == REQ_DESCRIPTION) 
    {
        OpenSearchReader reader;
        OpenSearchEngine *engine = reader.read(m_jobData);
        if (engine) 
        {
            m_enginesMap.insert(engine->name(), engine);
            QString path = KGlobal::dirs()->findResource("data", "rekonq/opensearch/");
            QString fileName = trimmedEngineName(engine->name());
            QFile file(path + fileName + ".xml");
            OpenSearchWriter writer;
            writer.write(&file, engine);

            QString searchUrl = OpenSearchEngine::parseTemplate("\\{@}", engine->searchUrlTemplate());
            m_currentJob = NULL;
            emit openSearchEngineAdded(engine->name(), searchUrl, fileName);
        }
        else 
        {
            kFatal() << "Error while adding new open search engine";
        }
        
        idleJob();
    }
}


QString OpenSearchManager::trimmedEngineName(const QString &engineName) const
{
    QString trimmed;
    QString::ConstIterator constIter = engineName.constBegin();
    while (constIter != engineName.constEnd()) 
    {
        if (constIter->isSpace()) 
        {
            trimmed.append('-');
        }
        else 
        {
            if (*constIter != '.') 
            {
                trimmed.append(constIter->toLower());
            }
        }
        constIter++;
    }

    return trimmed;
}


void OpenSearchManager::idleJob()
{
    if (m_currentJob)
    {
        disconnect(m_currentJob);
        m_currentJob->kill();
    }
    
    m_jobData.clear();
    m_state = IDLE;
}
