/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2009 by Nils Weigel <nehlsen at gmail dot com>
* Copyright (C) 2010-2013 by Andrea Diamantini <adjam7 at gmail dot com>
* Copyright (C) 2010 by Yoann Laissus <yoann dot laissus at gmail dot com>
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


#ifndef BOOKMARKS_PANEL_H
#define BOOKMARKS_PANEL_H


// Rekonq Includes
#include "rekonq_defines.h"

// Local Includes
#include "urlpanel.h"

// Forward Declarations
class BookmarksTreeModel;

class KBookmark;
class QModelIndex;


class REKONQ_TESTS_EXPORT BookmarksPanel : public UrlPanel
{
    Q_OBJECT

public:
    explicit BookmarksPanel(const QString &title, QWidget *parent = 0, Qt::WindowFlags flags = 0);
    ~BookmarksPanel();

public Q_SLOTS:
    void loadFoldedState();

Q_SIGNALS:
    void expansionChanged();

private Q_SLOTS:
    void contextMenu(const QPoint &pos);

    virtual void contextMenuItem(const QPoint &pos)
    {
        contextMenu(pos);
    }
    virtual void contextMenuGroup(const QPoint &pos)
    {
        contextMenu(pos);
    }
    virtual void contextMenuEmpty(const QPoint &pos)
    {
        contextMenu(pos);
    }

    void deleteBookmark();
    void onCollapse(const QModelIndex &index);
    void onExpand(const QModelIndex &index);

private:
    virtual void setup();

    void loadFoldedState(const QModelIndex &root);

    KBookmark bookmarkForIndex(const QModelIndex &index);

    virtual QAbstractItemModel* model();

    BookmarksTreeModel *_bkTreeModel;
    bool _loadingState;
};

#endif // BOOKMARKS_PANEL_H
