/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2007-2008 Trolltech ASA. All rights reserved
* Copyright (C) 2008 Benjamin C. Meyer <ben@meyerhome.net>
* Copyright (C) 2008-2012 by Andrea Diamantini <adjam7 at gmail dot com>
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


#ifndef HISTORY_MODELS_H
#define HISTORY_MODELS_H


// Rekonq Includes
#include "rekonq_defines.h"

// KDE Includes
#include <KUrl>

// Qt Includes
#include <QHash>
#include <QAbstractTableModel>
#include <QAbstractProxyModel>
#include <QSortFilterProxyModel>

// Forward Declarations
class HistoryManager;


class HistoryModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum Roles
    {
        DateRole = Qt::UserRole + 1,
        DateTimeRole = Qt::UserRole + 2,
        UrlRole = Qt::UserRole + 3,
        UrlStringRole = Qt::UserRole + 4,
        FirstDateTimeVisitRole = Qt::UserRole + 5
    };

    explicit HistoryModel(HistoryManager *history, QObject *parent = 0);

public Q_SLOTS:
    void historyReset();
    void entryAdded();

public:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());

private:
    HistoryManager *m_historyManager;
};


// ----------------------------------------------------------------------------------------------------


/**
 * Proxy model that will remove any duplicate entries.
 * Both m_sourceRow and m_historyHash store their offsets not from
 * the front of the list, but as offsets from the back.
 *
 */
class HistoryFilterModel : public QAbstractProxyModel
{
    Q_OBJECT

public:
    explicit HistoryFilterModel(QAbstractItemModel *sourceModel, QObject *parent = 0);

    inline bool historyContains(const QString &url) const
    {
        load();
        return m_historyHash.contains(url);
    }

    inline QList<QString> keys() const
    {
        load();
        return m_historyHash.keys();
    }

    int historyLocation(const QString &url) const;

    QModelIndex mapFromSource(const QModelIndex &sourceIndex) const;
    QModelIndex mapToSource(const QModelIndex &proxyIndex) const;
    void setSourceModel(QAbstractItemModel *sourceModel);
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QModelIndex index(int, int, const QModelIndex& = QModelIndex()) const;
    QModelIndex parent(const QModelIndex& index = QModelIndex()) const;
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

private Q_SLOTS:
    void sourceReset();
    void sourceDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
    void sourceRowsInserted(const QModelIndex &parent, int start, int end);
    void sourceRowsRemoved(const QModelIndex &, int, int);

private:
    void load() const;

    mutable QList<int> m_sourceRow;
    mutable QHash<QString, int> m_historyHash;
    mutable bool m_loaded;
};


// ----------------------------------------------------------------------------------------------------------------------


/**
 * Proxy model for the history model that converts the list
 * into a tree, one top level node per day.
 *
 * Used in the HistoryDialog.
 *
 */

class HistoryTreeModel : public QAbstractProxyModel
{
    Q_OBJECT

public:
    explicit HistoryTreeModel(QAbstractItemModel *sourceModel, QObject *parent = 0);

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    int columnCount(const QModelIndex &parent) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QModelIndex mapFromSource(const QModelIndex &sourceIndex) const;
    QModelIndex mapToSource(const QModelIndex &proxyIndex) const;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index = QModelIndex()) const;
    bool hasChildren(const QModelIndex &parent = QModelIndex()) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

    void setSourceModel(QAbstractItemModel *sourceModel);

private Q_SLOTS:
    void sourceReset();
    void sourceRowsInserted(const QModelIndex &parent, int start, int end);
    void sourceRowsRemoved(const QModelIndex &parent, int start, int end);

private:
    int sourceDateRow(int row) const;
    mutable QList<int> m_sourceRowCache;
};


// ----------------------------------------------------------------------------------------------------------------------


/**
 * QSortFilterProxyModel hides all children which parent doesn't
 * match the filter. This class is used to change this behavior.
 * If a url matches the filter it'll be shown,
 * even if it's parent doesn't match it.
 */
class SortFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    explicit SortFilterProxyModel(QObject *parent = 0);

protected:
    virtual bool filterAcceptsRow(const int source_row, const QModelIndex &source_parent) const;

    // returns true if index or any of his children match the filter
    bool recursiveMatch(const QModelIndex &index) const;
};


#endif // HISTORY_MODELS_H
