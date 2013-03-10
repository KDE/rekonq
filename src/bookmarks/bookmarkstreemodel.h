/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2009 by Nils Weigel <nehlsen at gmail dot com>
* Copyright (C) 2010-2013 by Andrea Diamantini <adjam7 at gmail dot com>
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


#ifndef BOOKMARKS_TREE_MODEL_H
#define BOOKMARKS_TREE_MODEL_H


// Rekonq Includes
#include "rekonq_defines.h"

// KDE includes
#include <KBookmark>

// Qt Includes
#include <QtCore/QAbstractItemModel>


class BtmItem
{
public:
    BtmItem(const KBookmark &bm);
    ~BtmItem();

    QVariant data(int role = Qt::DisplayRole) const;
    int row() const;
    int childCount() const;
    BtmItem* child(int n);
    BtmItem* parent() const;
    void appendChild(BtmItem *child);
    void clear();
    KBookmark getBkm() const;

private:
    BtmItem *m_parent;
    QList< BtmItem* > m_children;
    KBookmark m_kbm;
};


// -------------------------------------------------------------------------------------------------


class REKONQ_TESTS_EXPORT BookmarksTreeModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit BookmarksTreeModel(QObject *parent = 0);
    virtual ~BookmarksTreeModel();

    /**
     * @return number of rows under the given parent.
     */
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    /**
     * @return number of columns (always 1).
     */
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;

    virtual Qt::ItemFlags flags(const QModelIndex &index) const;

    /**
     * @return index in the model specified by the given row, column and parent.
     */
    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    /**
     * @return parent of the given index.
     */
    virtual QModelIndex parent(const QModelIndex &index) const;
    virtual QVariant data(const QModelIndex &index, int role) const;

    virtual QStringList mimeTypes() const;
    virtual bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent);
    virtual Qt::DropActions supportedDropActions() const;
    virtual QMimeData *mimeData(const QModelIndexList &indexes) const;

private Q_SLOTS:
    void bookmarksChanged(const QString &groupAddress);

Q_SIGNALS:
    void bookmarksUpdated();

private:
    void resetModel();
    void setRoot(KBookmarkGroup bmg);
    void populate(BtmItem *node, KBookmarkGroup bmg);
    KBookmark bookmarkForIndex(const QModelIndex &index) const;

    BtmItem *m_root;
};

#endif // BOOKMARKS_TREE_MODEL_H
