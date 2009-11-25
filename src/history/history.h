/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2007-2008 Trolltech ASA. All rights reserved
* Copyright (C) 2008 Benjamin C. Meyer <ben@meyerhome.net>
* Copyright (C) 2008-2009 by Andrea Diamantini <adjam7 at gmail dot com>
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


#ifndef HISTORY_H
#define HISTORY_H


// KDE Includes
#include <KUrl>

// Qt Includes
#include <QDateTime>
#include <QHash>
#include <QObject>
#include <QTimer>
#include <QSortFilterProxyModel>
#include <QWebHistoryInterface>


/**
 * Elements in this class represent an history item
 *
 */
class HistoryItem
{
public:
    HistoryItem() {}
    explicit HistoryItem(const QString &u,
                         const QDateTime &d = QDateTime(), 
                         const QString &t = QString()
                        )
            : title(t), url(u), dateTime(d) {}

    inline bool operator==(const HistoryItem &other) const
    {
        return other.title == title
               && other.url == url && other.dateTime == dateTime;
    }

    // history is sorted in reverse
    inline bool operator <(const HistoryItem &other) const
    {
        return dateTime > other.dateTime;
    }

    QString title;
    QString url;
    QDateTime dateTime;
};



// ---------------------------------------------------------------------------------------------------------------


// Forward Declarations
class AutoSaver;
class HistoryModel;
class HistoryFilterModel;
class HistoryTreeModel;

class KCompletion;


/**
 * THE History Manager:
 * It manages rekonq history
 *
 */
class HistoryManager : public QWebHistoryInterface
{
    Q_OBJECT
    Q_PROPERTY(int historyLimit READ historyLimit WRITE setHistoryLimit)

signals:
    void historyReset();
    void entryAdded(const HistoryItem &item);
    void entryRemoved(const HistoryItem &item);
    void entryUpdated(int offset);

public:
    HistoryManager(QObject *parent = 0);
    ~HistoryManager();

    bool historyContains(const QString &url) const;
    void addHistoryEntry(const QString &url);
    void updateHistoryEntry(const KUrl &url, const QString &title);
    void removeHistoryEntry(const KUrl &url, const QString &title = QString());

    int historyLimit() const;
    void setHistoryLimit(int limit);

    QList<HistoryItem> history() const;
    void setHistory(const QList<HistoryItem> &history, bool loadedAndSorted = false);

    // History manager keeps around these models for use by the completer and other classes
    HistoryModel *historyModel() const;
    HistoryFilterModel *historyFilterModel() const;
    HistoryTreeModel *historyTreeModel() const;

    /**
    * @returns the KCompletion object.
    */
    KCompletion *completionObject() const;

public slots:
    void clear();
    void loadSettings();

private slots:
    void save();
    void checkForExpired();

protected:
    void addHistoryEntry(const HistoryItem &item);
    void removeHistoryEntry(const HistoryItem &item);

private:
    void load();

    AutoSaver *m_saveTimer;
    int m_historyLimit;
    QTimer m_expiredTimer;
    QList<HistoryItem> m_history;
    QString m_lastSavedUrl;

    HistoryModel *m_historyModel;
    HistoryFilterModel *m_historyFilterModel;
    HistoryTreeModel *m_historyTreeModel;

    // the completion object we sync with
    KCompletion *m_completion;
};


#endif // HISTORY_H
