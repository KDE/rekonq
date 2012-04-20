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


// Self Includes
#include "historymodels.h"
#include "historymodels.moc"

// Auto Includes
#include "rekonq.h"

// Local Includes
#include "application.h"
#include "historymanager.h"
#include "iconmanager.h"

// KDE Includes
#include <KStandardDirs>
#include <KLocale>
#include <KIcon>

// Qt Includes
#include <QList>
#include <QUrl>
#include <QDate>
#include <QDateTime>
#include <QString>
#include <QFile>
#include <QDataStream>
#include <QBuffer>

#include <QClipboard>
#include <QFileInfo>

// generic algorithms
#include <QtAlgorithms>


HistoryModel::HistoryModel(HistoryManager *history, QObject *parent)
    : QAbstractTableModel(parent)
    , m_historyManager(history)
{
    Q_ASSERT(m_historyManager);
    connect(m_historyManager, SIGNAL(historyReset()), this, SLOT(historyReset()));
    connect(m_historyManager, SIGNAL(entryRemoved(HistoryItem)), this, SLOT(historyReset()));
    connect(m_historyManager, SIGNAL(entryAdded(HistoryItem)), this, SLOT(entryAdded()));
}


void HistoryModel::historyReset()
{
    reset();
}


void HistoryModel::entryAdded()
{
    beginInsertRows(QModelIndex(), 0, 0);
    endInsertRows();
}


QVariant HistoryModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal
            && role == Qt::DisplayRole)
    {
        switch (section)
        {
        case 0: return i18n("Title");
        case 1: return i18n("Address");
        }
    }
    return QAbstractTableModel::headerData(section, orientation, role);
}


QVariant HistoryModel::data(const QModelIndex &index, int role) const
{
    QList<HistoryItem> lst = m_historyManager->history();
    if (index.row() < 0 || index.row() >= lst.size())
        return QVariant();

    const HistoryItem &item = lst.at(index.row());
    switch (role)
    {
    case DateTimeRole:
        return item.lastDateTimeVisit;
    case DateRole:
        return item.lastDateTimeVisit.date();
    case FirstDateTimeVisitRole:
        return item.firstDateTimeVisit;
    case UrlRole:
        return QUrl(item.url);
    case Qt::UserRole:
        return KUrl(item.url);
    case UrlStringRole:
        return item.url;
    case Qt::DisplayRole:
    case Qt::EditRole:
    {
        switch (index.column())
        {
        case 0:
            // when there is no title try to generate one from the url
            if (item.title.isEmpty())
            {
                QString page = QFileInfo(QUrl(item.url).path()).fileName();
                if (!page.isEmpty())
                    return page;
                return item.url;
            }
            return item.title;
        case 1:
            return item.url;
        }
    }
    case Qt::DecorationRole:
        if (index.column() == 0)
        {
            return rApp->iconManager()->iconForUrl(item.url);
        }
    case Qt::ToolTipRole:
        QString tooltip = "";
        if (!item.title.isEmpty())
            tooltip = item.title + "<br/>";

        QString lastVisit = item.firstDateTimeVisit.toString(Qt::SystemLocaleDate);
        QString firstVisit = item.lastDateTimeVisit.toString(Qt::SystemLocaleDate);
        int visitCount = item.visitCount;
        tooltip += "<center> <b>" + item.url + "</b> </center>";
        tooltip += "<hr/>";
        tooltip += i18n("First Visit: ") + firstVisit + "<br/>";
        tooltip += i18n("Last Visit: ") + lastVisit + "<br/>";
        tooltip += i18n("Number of Visits: ") + QString::number(visitCount);

        return tooltip;
    }
    return QVariant();
}


int HistoryModel::columnCount(const QModelIndex &parent) const
{
    return (parent.isValid()) ? 0 : 2;
}


int HistoryModel::rowCount(const QModelIndex &parent) const
{
    return (parent.isValid()) ? 0 : m_historyManager->history().count();
}


bool HistoryModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if (parent.isValid())
        return false;
    int lastRow = row + count - 1;
    beginRemoveRows(parent, row, lastRow);
    QList<HistoryItem> lst = m_historyManager->history();
    for (int i = lastRow; i >= row; --i)
        lst.removeAt(i);
    disconnect(m_historyManager, SIGNAL(historyReset()), this, SLOT(historyReset()));
    m_historyManager->setHistory(lst);
    connect(m_historyManager, SIGNAL(historyReset()), this, SLOT(historyReset()));
    endRemoveRows();
    return true;
}


// ------------------------------------------------------------------------------------------------------------------


HistoryFilterModel::HistoryFilterModel(QAbstractItemModel *sourceModel, QObject *parent)
    : QAbstractProxyModel(parent)
    , m_loaded(false)
{
    setSourceModel(sourceModel);
}


int HistoryFilterModel::historyLocation(const QString &url) const
{
    load();
    if (!m_historyHash.contains(url))
        return 0;
    return sourceModel()->rowCount() - m_historyHash.value(url);
}


QVariant HistoryFilterModel::data(const QModelIndex &index, int role) const
{
    return QAbstractProxyModel::data(index, role);
}


void HistoryFilterModel::setSourceModel(QAbstractItemModel *newSourceModel)
{
    if (sourceModel())
    {
        disconnect(sourceModel(), SIGNAL(modelReset()), this, SLOT(sourceReset()));
        disconnect(sourceModel(), SIGNAL(dataChanged(QModelIndex,QModelIndex)),
                   this, SLOT(dataChanged(QModelIndex,QModelIndex)));
        disconnect(sourceModel(), SIGNAL(rowsInserted(QModelIndex,int,int)),
                   this, SLOT(sourceRowsInserted(QModelIndex,int,int)));
        disconnect(sourceModel(), SIGNAL(rowsRemoved(QModelIndex,int,int)),
                   this, SLOT(sourceRowsRemoved(QModelIndex,int,int)));
    }

    QAbstractProxyModel::setSourceModel(newSourceModel);

    if (sourceModel())
    {
        m_loaded = false;
        connect(sourceModel(), SIGNAL(modelReset()), this, SLOT(sourceReset()));
        connect(sourceModel(), SIGNAL(dataChanged(QModelIndex,QModelIndex)),
                this, SLOT(sourceDataChanged(QModelIndex,QModelIndex)));
        connect(sourceModel(), SIGNAL(rowsInserted(QModelIndex,int,int)),
                this, SLOT(sourceRowsInserted(QModelIndex,int,int)));
        connect(sourceModel(), SIGNAL(rowsRemoved(QModelIndex,int,int)),
                this, SLOT(sourceRowsRemoved(QModelIndex,int,int)));
    }
}


void HistoryFilterModel::sourceDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    emit dataChanged(mapFromSource(topLeft), mapFromSource(bottomRight));
}


QVariant HistoryFilterModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    return sourceModel()->headerData(section, orientation, role);
}


void HistoryFilterModel::sourceReset()
{
    m_loaded = false;
    reset();
}


int HistoryFilterModel::rowCount(const QModelIndex &parent) const
{
    load();
    if (parent.isValid())
        return 0;
    return m_historyHash.count();
}


int HistoryFilterModel::columnCount(const QModelIndex &parent) const
{
    return (parent.isValid()) ? 0 : 2;
}


QModelIndex HistoryFilterModel::mapToSource(const QModelIndex &proxyIndex) const
{
    load();
    int sourceRow = sourceModel()->rowCount() - proxyIndex.internalId();
    return sourceModel()->index(sourceRow, proxyIndex.column());
}


QModelIndex HistoryFilterModel::mapFromSource(const QModelIndex &sourceIndex) const
{
    load();
    QString url = sourceIndex.data(HistoryModel::UrlStringRole).toString();
    if (!m_historyHash.contains(url))
        return QModelIndex();

    // This can be done in a binary search, but we can't use qBinary find
    // because it can't take: qBinaryFind(m_sourceRow.end(), m_sourceRow.begin(), v);
    // so if this is a performance bottlneck then convert to binary search, until then
    // the cleaner/easier to read code wins the day.
    int realRow = -1;
    int sourceModelRow = sourceModel()->rowCount() - sourceIndex.row();

    for (int i = 0; i < m_sourceRow.count(); ++i)
    {
        if (m_sourceRow.at(i) == sourceModelRow)
        {
            realRow = i;
            break;
        }
    }
    if (realRow == -1)
        return QModelIndex();

    return createIndex(realRow, sourceIndex.column(), sourceModel()->rowCount() - sourceIndex.row());
}


QModelIndex HistoryFilterModel::index(int row, int column, const QModelIndex &parent) const
{
    load();
    if (row < 0 || row >= rowCount(parent)
            || column < 0 || column >= columnCount(parent))
        return QModelIndex();

    return createIndex(row, column, m_sourceRow[row]);
}


QModelIndex HistoryFilterModel::parent(const QModelIndex &) const
{
    return QModelIndex();
}


void HistoryFilterModel::load() const
{
    if (m_loaded)
        return;
    m_sourceRow.clear();
    m_historyHash.clear();
    m_historyHash.reserve(sourceModel()->rowCount());
    for (int i = 0; i < sourceModel()->rowCount(); ++i)
    {
        QModelIndex idx = sourceModel()->index(i, 0);
        QString url = idx.data(HistoryModel::UrlStringRole).toString();
        if (!m_historyHash.contains(url))
        {
            m_sourceRow.append(sourceModel()->rowCount() - i);
            m_historyHash[url] = sourceModel()->rowCount() - i;
        }
    }
    m_loaded = true;
}


void HistoryFilterModel::sourceRowsInserted(const QModelIndex &parent, int start, int end)
{
    Q_UNUSED(end);

    if (start != 0)
    {
        kDebug() << "STARTING from a NON zero position...";
        return;
    }

    if (!m_loaded)
        return;

    QModelIndex idx = sourceModel()->index(start, 0, parent);
    QString url = idx.data(HistoryModel::UrlStringRole).toString();
    if (m_historyHash.contains(url))
    {
        int sourceRow = sourceModel()->rowCount() - m_historyHash[url];
        int realRow = mapFromSource(sourceModel()->index(sourceRow, 0)).row();
        beginRemoveRows(QModelIndex(), realRow, realRow);
        m_sourceRow.removeAt(realRow);
        m_historyHash.remove(url);
        endRemoveRows();
    }
    beginInsertRows(QModelIndex(), 0, 0);
    m_historyHash.insert(url, sourceModel()->rowCount() - start);
    m_sourceRow.insert(0, sourceModel()->rowCount());
    endInsertRows();
}


void HistoryFilterModel::sourceRowsRemoved(const QModelIndex &, int start, int end)
{
    Q_UNUSED(start);
    Q_UNUSED(end);
    sourceReset();
}


/*
    Removing a continuous block of rows will remove filtered rows too as this is
    the users intention.
*/
bool HistoryFilterModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if (row < 0 || count <= 0 || row + count > rowCount(parent) || parent.isValid())
        return false;
    int lastRow = row + count - 1;
    disconnect(sourceModel(), SIGNAL(rowsRemoved(QModelIndex,int,int)),
               this, SLOT(sourceRowsRemoved(QModelIndex,int,int)));
    beginRemoveRows(parent, row, lastRow);
    int oldCount = rowCount();
    int start = sourceModel()->rowCount() - m_sourceRow.value(row);
    int end = sourceModel()->rowCount() - m_sourceRow.value(lastRow);
    sourceModel()->removeRows(start, end - start + 1);
    endRemoveRows();
    connect(sourceModel(), SIGNAL(rowsRemoved(QModelIndex,int,int)),
            this, SLOT(sourceRowsRemoved(QModelIndex,int,int)));
    m_loaded = false;
    if (oldCount - count != rowCount())
        reset();
    return true;
}


// ----------------------------------------------------------------------------------------------------------


HistoryTreeModel::HistoryTreeModel(QAbstractItemModel *sourceModel, QObject *parent)
    : QAbstractProxyModel(parent)
{
    setSourceModel(sourceModel);
}


QVariant HistoryTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    return sourceModel()->headerData(section, orientation, role);
}


QVariant HistoryTreeModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::EditRole || role == Qt::DisplayRole)
    {
        int start = index.internalId();
        if (start == 0)
        {
            int offset = sourceDateRow(index.row());
            if (index.column() == 0)
            {
                QModelIndex idx = sourceModel()->index(offset, 0);
                QDate date = idx.data(HistoryModel::DateRole).toDate();
                if (date == QDate::currentDate())
                    return i18n("Earlier Today");
                return date.toString(QL1S("dddd, MMMM d, yyyy"));
            }
            if (index.column() == 1)
            {
                return i18np("1 item", "%1 items", rowCount(index.sibling(index.row(), 0)));
            }
        }
    }

    if (role == Qt::DecorationRole && index.column() == 0 && !index.parent().isValid())
        return KIcon("view-history");

    if (role == HistoryModel::DateRole && index.column() == 0 && index.internalId() == 0)
    {
        int offset = sourceDateRow(index.row());
        QModelIndex idx = sourceModel()->index(offset, 0);
        return idx.data(HistoryModel::DateRole);
    }

    if (role == HistoryModel::FirstDateTimeVisitRole && index.column() == 0 && index.internalId() == 0)
    {
        int offset = sourceDateRow(index.row());
        QModelIndex idx = sourceModel()->index(offset, 0);
        return idx.data(HistoryModel::FirstDateTimeVisitRole);
    }

    return QAbstractProxyModel::data(index, role);
}


int HistoryTreeModel::columnCount(const QModelIndex &parent) const
{
    return sourceModel()->columnCount(mapToSource(parent));
}


int HistoryTreeModel::rowCount(const QModelIndex &parent) const
{
    if (parent.internalId() != 0
            || parent.column() > 0
            || !sourceModel())
        return 0;

    // row count OF dates
    if (!parent.isValid())
    {
        if (!m_sourceRowCache.isEmpty())
            return m_sourceRowCache.count();
        QDate currentDate;
        int rows = 0;
        int totalRows = sourceModel()->rowCount();

        for (int i = 0; i < totalRows; ++i)
        {
            QDate rowDate = sourceModel()->index(i, 0).data(HistoryModel::DateRole).toDate();
            if (rowDate != currentDate)
            {
                m_sourceRowCache.append(i);
                currentDate = rowDate;
                ++rows;
            }
        }
        Q_ASSERT(m_sourceRowCache.count() == rows);
        return rows;
    }

    // row count FOR a date
    int start = sourceDateRow(parent.row());
    int end = sourceDateRow(parent.row() + 1);
    return (end - start);
}


// Translate the top level date row into the offset where that date starts
int HistoryTreeModel::sourceDateRow(int row) const
{
    if (row <= 0)
        return 0;

    if (m_sourceRowCache.isEmpty())
        rowCount(QModelIndex());

    if (row >= m_sourceRowCache.count())
    {
        if (!sourceModel())
            return 0;
        return sourceModel()->rowCount();
    }
    return m_sourceRowCache.at(row);
}


QModelIndex HistoryTreeModel::mapToSource(const QModelIndex &proxyIndex) const
{
    int offset = proxyIndex.internalId();
    if (offset == 0)
        return QModelIndex();
    int startDateRow = sourceDateRow(offset - 1);
    return sourceModel()->index(startDateRow + proxyIndex.row(), proxyIndex.column());
}


QModelIndex HistoryTreeModel::index(int row, int column, const QModelIndex &parent) const
{
    if (row < 0
            || column < 0 || column >= columnCount(parent)
            || parent.column() > 0)
        return QModelIndex();

    if (!parent.isValid())
        return createIndex(row, column, 0);
    return createIndex(row, column, parent.row() + 1);
}


QModelIndex HistoryTreeModel::parent(const QModelIndex &index) const
{
    int offset = index.internalId();
    if (offset == 0 || !index.isValid())
        return QModelIndex();
    return createIndex(offset - 1, 0, 0);
}


bool HistoryTreeModel::hasChildren(const QModelIndex &parent) const
{
    QModelIndex grandparent = parent.parent();
    if (!grandparent.isValid())
        return true;
    return false;
}


Qt::ItemFlags HistoryTreeModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled;
}


bool HistoryTreeModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if (row < 0 || count <= 0 || row + count > rowCount(parent))
        return false;

    if (parent.isValid())
    {
        // removing pages
        int offset = sourceDateRow(parent.row());
        return sourceModel()->removeRows(offset + row, count);
    }
    else
    {
        // removing whole dates
        for (int i = row + count - 1; i >= row; --i)
        {
            QModelIndex dateParent = index(i, 0);
            int offset = sourceDateRow(dateParent.row());
            if (!sourceModel()->removeRows(offset, rowCount(dateParent)))
                return false;
        }
    }
    return true;
}


void HistoryTreeModel::setSourceModel(QAbstractItemModel *newSourceModel)
{
    if (sourceModel())
    {
        disconnect(sourceModel(), SIGNAL(modelReset()), this, SLOT(sourceReset()));
        disconnect(sourceModel(), SIGNAL(layoutChanged()), this, SLOT(sourceReset()));
        disconnect(sourceModel(), SIGNAL(rowsInserted(QModelIndex,int,int)),
                   this, SLOT(sourceRowsInserted(QModelIndex,int,int)));
        disconnect(sourceModel(), SIGNAL(rowsRemoved(QModelIndex,int,int)),
                   this, SLOT(sourceRowsRemoved(QModelIndex,int,int)));
    }

    QAbstractProxyModel::setSourceModel(newSourceModel);

    if (newSourceModel)
    {
        connect(sourceModel(), SIGNAL(modelReset()), this, SLOT(sourceReset()));
        connect(sourceModel(), SIGNAL(layoutChanged()), this, SLOT(sourceReset()));
        connect(sourceModel(), SIGNAL(rowsInserted(QModelIndex,int,int)),
                this, SLOT(sourceRowsInserted(QModelIndex,int,int)));
        connect(sourceModel(), SIGNAL(rowsRemoved(QModelIndex,int,int)),
                this, SLOT(sourceRowsRemoved(QModelIndex,int,int)));
    }

    reset();
}


void HistoryTreeModel::sourceReset()
{
    m_sourceRowCache.clear();
    reset();
}


void HistoryTreeModel::sourceRowsInserted(const QModelIndex &parent, int start, int end)
{
    Q_UNUSED(parent); // Avoid warnings when compiling release
    Q_ASSERT(!parent.isValid());
    if (start != 0 || start != end)
    {
        m_sourceRowCache.clear();
        reset();
        return;
    }

    m_sourceRowCache.clear();
    QModelIndex treeIndex = mapFromSource(sourceModel()->index(start, 0));
    QModelIndex treeParent = treeIndex.parent();
    if (rowCount(treeParent) == 1)
    {
        beginInsertRows(QModelIndex(), 0, 0);
        endInsertRows();
    }
    else
    {
        beginInsertRows(treeParent, treeIndex.row(), treeIndex.row());
        endInsertRows();
    }
}


QModelIndex HistoryTreeModel::mapFromSource(const QModelIndex &sourceIndex) const
{
    if (!sourceIndex.isValid())
        return QModelIndex();

    if (m_sourceRowCache.isEmpty())
        rowCount(QModelIndex());

    QList<int>::iterator it;
    it = qLowerBound(m_sourceRowCache.begin(), m_sourceRowCache.end(), sourceIndex.row());
    if (*it != sourceIndex.row())
        --it;

    int dateRow = qMax(0, it - m_sourceRowCache.begin());
    int row = sourceIndex.row() - m_sourceRowCache.at(dateRow);
    return createIndex(row, sourceIndex.column(), dateRow + 1);
}


void HistoryTreeModel::sourceRowsRemoved(const QModelIndex &parent, int start, int end)
{
    Q_UNUSED(parent); // Avoid warnings when compiling release
    Q_ASSERT(!parent.isValid());
    if (m_sourceRowCache.isEmpty())
        return;
    for (int i = end; i >= start;)
    {
        QList<int>::iterator it;
        it = qLowerBound(m_sourceRowCache.begin(), m_sourceRowCache.end(), i);
        // playing it safe
        if (it == m_sourceRowCache.end())
        {
            m_sourceRowCache.clear();
            reset();
            return;
        }

        if (*it != i)
            --it;
        int row = qMax(0, it - m_sourceRowCache.begin());
        int offset = m_sourceRowCache[row];
        QModelIndex dateParent = index(row, 0);
        // If we can remove all the rows in the date do that and skip over them
        int rc = rowCount(dateParent);
        if (i - rc + 1 == offset && start <= i - rc + 1)
        {
            beginRemoveRows(QModelIndex(), row, row);
            m_sourceRowCache.removeAt(row);
            i -= rc + 1;
        }
        else
        {
            beginRemoveRows(dateParent, i - offset, i - offset);
            ++row;
            --i;
        }
        for (int j = row; j < m_sourceRowCache.count(); ++j)
            --m_sourceRowCache[j];
        endRemoveRows();
    }
}
