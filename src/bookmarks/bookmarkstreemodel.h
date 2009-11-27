/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2009 by Nils Weigel <nehlsen at gmail dot com>
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


#ifndef BOOKMARKSTREEMODEL_H
#define BOOKMARKSTREEMODEL_H

// Qt Includes
#include <QAbstractItemModel>

// KDE includes
#include <KBookmark>

class BookmarksTreeModel : public QAbstractItemModel
{
    Q_OBJECT
    Q_DISABLE_COPY(BookmarksTreeModel)

public:
    explicit BookmarksTreeModel(QObject *parent = 0);
    ~BookmarksTreeModel();

	virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;

	virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
	virtual Qt::ItemFlags flags(const QModelIndex &index) const;

	virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    virtual QModelIndex parent(const QModelIndex &index) const;
	virtual QVariant data(const QModelIndex &index, int role) const;
//     virtual bool setData(const QModelIndex &index, const QVariant &value, int role);

private slots:
	void bookmarksChanged( const QString &groupAddress );

private:
	class BtmItem;
	BtmItem *m_root;

	void resetModel();

    void setRoot(KBookmarkGroup bmg);
	void populate( BtmItem *node, KBookmarkGroup bmg);
};

#endif // BOOKMARKSTREEMODEL_H
