/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2007-2008 Trolltech ASA. All rights reserved
* Copyright (C) 2008 Benjamin C. Meyer <ben@meyerhome.net>
* Copyright (C) 2008-2011 by Andrea Diamantini <adjam7 at gmail dot com>
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
#include "historymanager.h"
#include "historymanager.moc"

// Auto Includes
#include "rekonq.h"

// Local Includes
#include "historymodels.h"
#include "autosaver.h"
#include "application.h"

// KDE Includes
#include <KStandardDirs>
#include <KLocale>
#include <KCompletion>

// Qt Includes
#include <QtCore/QList>
#include <QtCore/QUrl>
#include <QtCore/QDate>
#include <QtCore/QDateTime>
#include <QtCore/QString>
#include <QtCore/QFile>
#include <QtCore/QDataStream>
#include <QtCore/QBuffer>

#include <QtGui/QClipboard>

// generic algorithms
#include <QtAlgorithms>


static const unsigned int HISTORY_VERSION = 24;


HistoryManager::HistoryManager(QObject *parent)
        : QWebHistoryInterface(parent)
        , m_saveTimer(new AutoSaver(this))
        , m_historyLimit(0)
        , m_historyTreeModel(0)
{
    connect(this, SIGNAL(entryAdded(const HistoryItem &)), m_saveTimer, SLOT(changeOccurred()));
    connect(this, SIGNAL(entryRemoved(const HistoryItem &)), m_saveTimer, SLOT(changeOccurred()));
    connect(m_saveTimer, SIGNAL(saveNeeded()), this, SLOT(save()));

    load();

    HistoryModel *historyModel = new HistoryModel(this, this);
    m_historyFilterModel = new HistoryFilterModel(historyModel, this);
    m_historyTreeModel = new HistoryTreeModel(m_historyFilterModel, this);

    // QWebHistoryInterface will delete the history manager
    QWebHistoryInterface::setDefaultInterface(this);
}


HistoryManager::~HistoryManager()
{
    m_saveTimer->saveIfNeccessary();

    delete m_historyFilterModel;
    delete m_historyTreeModel;
}


bool HistoryManager::historyContains(const QString &url) const
{
    return m_historyFilterModel->historyContains(url);
}


void HistoryManager::addHistoryEntry(const QString &url)
{
    QWebSettings *globalSettings = QWebSettings::globalSettings();
    if (globalSettings->testAttribute(QWebSettings::PrivateBrowsingEnabled))
        return;

    QUrl cleanUrl(url);

    // don't store about: urls (home page related)
    if (cleanUrl.scheme() == QString("about"))
        return;

    cleanUrl.setPassword(QString());
    cleanUrl.setHost(cleanUrl.host().toLower());
    QString checkUrlString = cleanUrl.toString();

    HistoryItem item;

    // NOTE
    // check if the url has just been visited.
    // if so, remove previous entry from history, update and prepend it
    if (historyContains(checkUrlString))
    {
        int index = m_historyFilterModel->historyLocation(checkUrlString);
        item = m_history.at(index);
        m_history.removeOne(item);
        emit entryRemoved(item);

        item.dateTime = QDateTime::currentDateTime();
        item.visitCount++;
    }
    else
    {
        item = HistoryItem(checkUrlString, QDateTime::currentDateTime());
    }

    m_history.prepend(item);
    emit entryAdded(item);

    if (m_history.count() == 1)
        checkForExpired();
}


void HistoryManager::setHistory(const QList<HistoryItem> &history, bool loadedAndSorted)
{
    m_history = history;

    // verify that it is sorted by date
    if (!loadedAndSorted)
        qSort(m_history.begin(), m_history.end());

    checkForExpired();

    if (loadedAndSorted)
    {
        m_lastSavedUrl = m_history.value(0).url;
    }
    else
    {
        m_lastSavedUrl.clear();
        m_saveTimer->changeOccurred();
    }
    emit historyReset();
}


void HistoryManager::checkForExpired()
{
    if (m_historyLimit < 0 || m_history.isEmpty())
        return;

    QDateTime now = QDateTime::currentDateTime();
    int nextTimeout = 0;

    while (!m_history.isEmpty())
    {
        QDateTime checkForExpired = m_history.last().dateTime;
        checkForExpired.setDate(checkForExpired.date().addDays(m_historyLimit));
        if (now.daysTo(checkForExpired) > 7)
        {
            // check at most in a week to prevent int overflows on the timer
            nextTimeout = 7 * 86400;
        }
        else
        {
            nextTimeout = now.secsTo(checkForExpired);
        }
        if (nextTimeout > 0)
            break;
        HistoryItem item = m_history.takeLast();
        // remove from saved file also
        m_lastSavedUrl.clear();
        emit entryRemoved(item);
    }

    if (nextTimeout > 0)
        QTimer::singleShot(nextTimeout * 1000, this, SLOT(checkForExpired()));
}


void HistoryManager::updateHistoryEntry(const KUrl &url, const QString &title)
{
    QString urlString = url.url();
    urlString.remove(QL1S("www."));
    if (urlString.startsWith(QL1S("http")) && urlString.endsWith(QL1C('/')))
        urlString.remove(urlString.length() - 1, 1);

    for (int i = 0; i < m_history.count(); ++i)
    {
        QString itemUrl = m_history.at(i).url;
        itemUrl.remove(QL1S("www."));
        if (itemUrl.startsWith(QL1S("http")) && itemUrl.endsWith(QL1C('/')))
            itemUrl.remove(itemUrl.length() - 1, 1);

        if (urlString == itemUrl)
        {
            m_history[i].title = title;
            m_history[i].url = url.url();
            m_saveTimer->changeOccurred();
            if (m_lastSavedUrl.isEmpty())
                m_lastSavedUrl = m_history.at(i).url;

            emit entryUpdated(i);
            break;
        }
    }
}


void HistoryManager::removeHistoryEntry(const KUrl &url, const QString &title)
{
    HistoryItem item;
    for (int i = 0; i < m_history.count(); ++i)
    {
        if (url == m_history.at(i).url
                && (title.isEmpty() || title == m_history.at(i).title))
        {
            item = m_history.at(i);
            m_lastSavedUrl.clear();
            m_history.removeOne(item);
            emit entryRemoved(item);
            break;
        }
    }
}


QList<HistoryItem> HistoryManager::find(const QString &text)
{
    QList<HistoryItem> list;

    QStringList urlKeys = m_historyFilterModel->keys();
    Q_FOREACH(const QString &url, urlKeys)
    {
        int index = m_historyFilterModel->historyLocation(url);
        HistoryItem item = m_history.at(index);

        QStringList words = text.split(' ');
        bool matches = true;
        foreach(const QString &word, words)
        {
            if (!url.contains(word, Qt::CaseInsensitive)
                    && !item.title.contains(word, Qt::CaseInsensitive))
            {
                matches = false;
                break;
            }
        }
        if (matches)
            list << item;
    }

    return list;
}


void HistoryManager::clear()
{
    m_history.clear();
    m_lastSavedUrl.clear();
    m_saveTimer->changeOccurred();
    m_saveTimer->saveIfNeccessary();
    historyReset();
}


void HistoryManager::loadSettings()
{
    int historyExpire = ReKonfig::expireHistory();
    int days;
    switch (historyExpire)
    {
    case 0:
        days = 1;
        break;
    case 1:
        days = 7;
        break;
    case 2:
        days = 14;
        break;
    case 3:
        days = 30;
        break;
    case 4:
        days = 365;
        break;
    case 5:
        days = -1;
        break;
    default:
        days = -1;
        break;
    }
    m_historyLimit = days;
}


void HistoryManager::load()
{
    loadSettings();

    QString historyFilePath = KStandardDirs::locateLocal("appdata" , "history");
    QFile historyFile(historyFilePath);
    if (!historyFile.exists())
        return;
    if (!historyFile.open(QFile::ReadOnly))
    {
        kDebug() << "Unable to open history file" << historyFile.fileName();
        return;
    }

    QList<HistoryItem> list;
    QDataStream in(&historyFile);
    // Double check that the history file is sorted as it is read in
    bool needToSort = false;
    HistoryItem lastInsertedItem;
    QByteArray data;
    QDataStream stream;
    QBuffer buffer;
    stream.setDevice(&buffer);
    while (!historyFile.atEnd())
    {
        in >> data;
        buffer.close();
        buffer.setBuffer(&data);
        buffer.open(QIODevice::ReadOnly);
        quint32 version;
        stream >> version;

        HistoryItem item;

        switch (version)
        {
        case HISTORY_VERSION:   // default case
            stream >> item.url;
            stream >> item.dateTime;
            stream >> item.title;
            stream >> item.visitCount;
            break;

        case 23:    // this will be used to upgrade previous structure...
            stream >> item.url;
            stream >> item.dateTime;
            stream >> item.title;
            item.visitCount = 1;
            break;

        default:
            continue;
        };

        if (!item.dateTime.isValid())
            continue;

        if (item == lastInsertedItem)
        {
            if (lastInsertedItem.title.isEmpty() && !list.isEmpty())
                list[0].title = item.title;
            continue;
        }

        if (!needToSort && !list.isEmpty() && lastInsertedItem < item)
            needToSort = true;

        list.prepend(item);
        lastInsertedItem = item;
    }
    if (needToSort)
        qSort(list.begin(), list.end());

    setHistory(list, true);

    // If we had to sort re-write the whole history sorted
    if (needToSort)
    {
        m_lastSavedUrl.clear();
        m_saveTimer->changeOccurred();
    }
}


void HistoryManager::save()
{
    bool saveAll = m_lastSavedUrl.isEmpty();
    int first = m_history.count() - 1;
    if (!saveAll)
    {
        // find the first one to save
        for (int i = 0; i < m_history.count(); ++i)
        {
            if (m_history.at(i).url == m_lastSavedUrl)
            {
                first = i - 1;
                break;
            }
        }
    }
    if (first == m_history.count() - 1)
        saveAll = true;

    QString historyFilePath = KStandardDirs::locateLocal("appdata" , "history");
    QFile historyFile(historyFilePath);

    // When saving everything use a temporary file to prevent possible data loss.
    QTemporaryFile tempFile;
    tempFile.setAutoRemove(false);
    bool open = false;
    if (saveAll)
    {
        open = tempFile.open();
    }
    else
    {
        open = historyFile.open(QFile::Append);
    }

    if (!open)
    {
        kDebug() << "Unable to open history file for saving"
        << (saveAll ? tempFile.fileName() : historyFile.fileName());
        return;
    }

    QDataStream out(saveAll ? &tempFile : &historyFile);
    for (int i = first; i >= 0; --i)
    {
        QByteArray data;
        QDataStream stream(&data, QIODevice::WriteOnly);
        HistoryItem item = m_history.at(i);
        stream << HISTORY_VERSION << item.url << item.dateTime << item.title << item.visitCount;
        out << data;
    }
    tempFile.close();

    if (saveAll)
    {
        if (historyFile.exists() && !historyFile.remove())
        {
            kDebug() << "History: error removing old history." << historyFile.errorString();
        }
        if (!tempFile.rename(historyFile.fileName()))
        {
            kDebug() << "History: error moving new history over old." << tempFile.errorString() << historyFile.fileName();
        }
    }
    m_lastSavedUrl = m_history.value(0).url;
}
