/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2009 by Fredy Yanardi <fyanardi@gmail.com>
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
#include "opensearchmanager.h"
#include "opensearchmanager.moc"

// Local Includes
#include "opensearchengine.h"
#include "opensearchreader.h"
#include "opensearchwriter.h"
#include "application.h"

// KDE Includes
#include <KDebug>
#include <KGlobal>
#include <KStandardDirs>
#include <KUrl>
#include <kio/scheduler.h>
#include <KService>
#include <KDE/KMessageBox>
#include <KUriFilterData>
#include <KConfigGroup>

// Qt Includes
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QDBusMessage>
#include <QDBusConnection>

OpenSearchManager::OpenSearchManager(QObject *parent)
        : QObject(parent)
        , m_activeEngine(0)
        , m_currentJob(0)
{
    m_state = IDLE;
    loadEngines();
}


OpenSearchManager::~OpenSearchManager()
{
    qDeleteAll(m_engineCache);
    m_engineCache.clear();
    m_engines.clear();
}


void OpenSearchManager::setSearchProvider(const QString &searchProvider)
{
    m_activeEngine = 0;

    if (!m_engineCache.contains(searchProvider))
    {
        const QString fileName = KGlobal::dirs()->findResource("data", "rekonq/opensearch/" + trimmedEngineName(searchProvider) + ".xml");
        kDebug() << searchProvider << " trimmed name: "  << trimmedEngineName(searchProvider) << " file name path: " << fileName;
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
            m_engineCache.insert(searchProvider, engine);
        }
        else
        {
            return;
        }
    }

    m_activeEngine = m_engineCache.value(searchProvider);
}


bool OpenSearchManager::isSuggestionAvailable()
{
    return m_activeEngine != 0;
}


void OpenSearchManager::addOpenSearchEngine(const KUrl &url, const QString &title, const QString &shortcut)
{
    Q_UNUSED(title);

    m_shortcut = shortcut;

    if (m_state != IDLE)
    {
        idleJob();
    }

    m_currentJob = KIO::get(url, KIO::NoReload, KIO::HideProgressInfo);
    m_jobUrl = url;
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

    if (m_activeEngine->hasCachedSuggestionsFor(searchText))
    {
        emit suggestionsReceived(searchText, m_activeEngine->cachedSuggestionsFor(searchText));
    }
    else
    {
        KUrl url = m_activeEngine->suggestionsUrl(searchText);
        kDebug() << "Requesting for suggestions: " << url.url();
        _typedText = searchText;
        m_currentJob = KIO::get(url, KIO::NoReload, KIO::HideProgressInfo);
        m_state = REQ_SUGGESTION;
        connect(m_currentJob, SIGNAL(data(KIO::Job *, const QByteArray &)), this, SLOT(dataReceived(KIO::Job *, const QByteArray &)));
        connect(m_currentJob, SIGNAL(result(KJob *)), this, SLOT(jobFinished(KJob *)));
    }
}


void OpenSearchManager::dataReceived(KIO::Job *job, const QByteArray &data)
{
    Q_UNUSED(job);
    m_jobData.append(data);
}


void OpenSearchManager::jobFinished(KJob *job)
{
    // Do NOT parse if job had same errors or the typed string is empty
    if (job->error() || _typedText.isEmpty())
    {
        emit suggestionsReceived(_typedText, ResponseList());
        m_state = IDLE;
        return; // just silently return
    }

    if (m_state == REQ_SUGGESTION)
    {
        const ResponseList suggestionsList = m_activeEngine->parseSuggestion(_typedText, m_jobData);
        kDebug() << "Received suggestions in " << _typedText << " from " << m_activeEngine->name() << ": ";
        Q_FOREACH(const Response &r, suggestionsList)
        {
            kDebug() << r.title;
        }
        emit suggestionsReceived(_typedText, suggestionsList);
        idleJob();
        return;
    }

    if (m_state == REQ_DESCRIPTION)
    {
        OpenSearchReader reader;
        OpenSearchEngine *engine = reader.read(m_jobData);
        if (engine)
        {
            m_engineCache.insert(engine->name(), engine);
            m_engines.insert(m_jobUrl, trimmedEngineName(engine->name()));
            saveEngines();

            QString path;
            if (engine->providesSuggestions()) //save opensearch description only if it provides suggestions
            {
                OpenSearchWriter writer;
                path = KGlobal::dirs()->findResource("data", "rekonq/opensearch/");
                QFile file(path + trimmedEngineName(engine->name()) + ".xml");
                writer.write(&file, engine);
            }

            QString searchUrl = OpenSearchEngine::parseTemplate("\\{@}", engine->searchUrlTemplate());
            m_currentJob = NULL;

            path = KGlobal::mainComponent().dirs()->saveLocation("services", "searchproviders/");
            KConfig _service(path +  trimmedEngineName(engine->name()) + ".desktop", KConfig::SimpleConfig);
            KConfigGroup service(&_service, "Desktop Entry");
            service.writeEntry("Type", "Service");
            service.writeEntry("ServiceTypes", "SearchProvider");
            service.writeEntry("Name", engine->name());
            service.writeEntry("Query", searchUrl);
            service.writeEntry("Keys", m_shortcut);
            // TODO charset
            service.writeEntry("Charset", "" /* provider->charset() */);
            // we might be overwriting a hidden entry
            service.writeEntry("Hidden", false);
            service.sync();

            // Update filters in running applications...
            QDBusMessage msg = QDBusMessage::createSignal("/", "org.kde.KUriFilterPlugin", "configure");
            QDBusConnection::sessionBus().send(msg);

            emit openSearchEngineAdded(engine->name(), searchUrl, m_shortcut);
        }
        else
        {
            kFatal() << "Error while adding new open search engine";
        }

        idleJob();
    }
}


void OpenSearchManager::loadEngines()
{
    QFile file(KStandardDirs::locate("appdata", "opensearch/db_opensearch.json"));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        kDebug() << "opensearch db cannot be read";
        return;
    }

    QString fileContent = QString::fromUtf8(file.readAll());
    QScriptEngine reader;
    if (!reader.canEvaluate(fileContent))
    {
        kDebug() << "opensearch db cannot be read";
        return;
    }

    QScriptValue responseParts = reader.evaluate(fileContent);
    QVariantList list;
    qScriptValueToSequence(responseParts, list);
    QStringList l;
    Q_FOREACH(const QVariant &e, list)
    {
        l = e.toStringList();
        m_engines.insert(KUrl(l.first()), l.last());
    }
    file.close();
}


void OpenSearchManager::saveEngines()
{
    QFile file(KStandardDirs::locateLocal("appdata", "opensearch/db_opensearch.json"));
    if (!file.open(QIODevice::WriteOnly))
    {
        kDebug() << "opensearch db cannot be writen";
        return;
    }
    QTextStream out(&file);
    out << "[";
    int i = 0;
    QList<KUrl> urls = m_engines.keys();
    Q_FOREACH(const KUrl &url, urls)
    {
        out << "[\"" << url.url() << "\",\"" << m_engines.value(url) << "\"]";
        i++;
        if (i != urls.size())
        {
            out << ",\n";
        }
    }
    out << "]\n";
    file.close();
}


void  OpenSearchManager::removeDeletedEngines()
{
    KService::Ptr service;
    Q_FOREACH(const KUrl &url, m_engines.keys())
    {
        service = KService::serviceByDesktopPath(QString("searchproviders/%1.desktop").arg(m_engines.value(url)));
        if (!service)
        {
            QString path = KStandardDirs::locateLocal("appdata", "opensearch/" + trimmedEngineName(m_engines.value(url)) + ".xml");
            QFile::remove(path + trimmedEngineName(m_engines.value(url)) + ".xml");
            m_engines.remove(url);
        }
    }
    saveEngines();
}


bool OpenSearchManager::engineExists(const KUrl &url)
{
    return m_engines.contains(url);
}


QString OpenSearchManager::trimmedEngineName(const QString &engineName) const
{
    QString trimmed;
    QString::ConstIterator constIter = engineName.constBegin();
    while (constIter != engineName.constEnd())
    {
        if (constIter->isSpace())
        {
            trimmed.append('_');
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
