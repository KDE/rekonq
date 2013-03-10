/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2009 by Nils Weigel <nehlsen at gmail dot com>
* Copyright (C) 2010-2013 by Andrea Diamantini <adjam7 at gmail dot com>
* Copyright (C) 2010 by Yoann Laissus <yoann dot laissus at gmail dot com>
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
#include "bookmarkspanel.h"
#include "bookmarkspanel.moc"

// Auto Includes
#include "rekonq.h"

// Local Includes
#include "bookmarkmanager.h"
#include "bookmarkstreemodel.h"
#include "bookmarkscontextmenu.h"
#include "bookmarkowner.h"

#include "paneltreeview.h"
#include "urlfilterproxymodel.h"


BookmarksPanel::BookmarksPanel(const QString &title, QWidget *parent, Qt::WindowFlags flags)
    : UrlPanel(title, parent, flags)
    , _bkTreeModel(new BookmarksTreeModel(this))
    , _loadingState(false)
{
    setObjectName("bookmarksPanel");
    setVisible(ReKonfig::showBookmarksPanel());

    panelTreeView()->setDragEnabled(true);
    panelTreeView()->setAcceptDrops(true);
    connect(_bkTreeModel, SIGNAL(bookmarksUpdated()), this, SLOT(loadFoldedState()));
}


BookmarksPanel::~BookmarksPanel()
{
    ReKonfig::setShowBookmarksPanel(!isHidden());
}


void BookmarksPanel::loadFoldedState()
{
    _loadingState = true;
    loadFoldedState(QModelIndex());
    _loadingState = false;
}


void BookmarksPanel::contextMenu(const QPoint &pos)
{
    if (_loadingState)
        return;

    BookmarksContextMenu menu(bookmarkForIndex(panelTreeView()->indexAt(pos)),
                              BookmarkManager::self()->manager(),
                              BookmarkManager::self()->owner()
                             );

    menu.exec(panelTreeView()->mapToGlobal(pos));
}


void BookmarksPanel::deleteBookmark()
{
    QModelIndex index = panelTreeView()->currentIndex();
    if (_loadingState || !index.isValid())
        return;

    BookmarkManager::self()->owner()->deleteBookmark(bookmarkForIndex(index));
}


void BookmarksPanel::onCollapse(const QModelIndex &index)
{
    if (_loadingState)
        return;

    bookmarkForIndex(index).internalElement().setAttribute("folded", "yes");
    emit expansionChanged();
}


void BookmarksPanel::onExpand(const QModelIndex &index)
{
    if (_loadingState)
        return;

    bookmarkForIndex(index).internalElement().setAttribute("folded", "no");
    emit expansionChanged();
}


void BookmarksPanel::setup()
{
    UrlPanel::setup();

    connect(panelTreeView(), SIGNAL(delKeyPressed()), this, SLOT(deleteBookmark()));
    connect(panelTreeView(), SIGNAL(collapsed(QModelIndex)), this, SLOT(onCollapse(QModelIndex)));
    connect(panelTreeView(), SIGNAL(expanded(QModelIndex)), this, SLOT(onExpand(QModelIndex)));

    loadFoldedState();
}


void BookmarksPanel::loadFoldedState(const QModelIndex &root)
{
    QAbstractItemModel *model = panelTreeView()->model();
    if (!model)
        return;

    int count = model->rowCount(root);
    QModelIndex index;

    for (int i = 0; i < count; ++i)
    {
        index = model->index(i, 0, root);
        if (index.isValid())
        {
            KBookmark bm = bookmarkForIndex(index);
            if (bm.isGroup())
            {
                panelTreeView()->setExpanded(index, bm.toGroup().isOpen());
                loadFoldedState(index);
            }
        }
    }
}


KBookmark BookmarksPanel::bookmarkForIndex(const QModelIndex &index)
{
    if (!index.isValid())
        return KBookmark();

    const UrlFilterProxyModel *proxyModel = static_cast<const UrlFilterProxyModel*>(index.model());
    QModelIndex originalIndex = proxyModel->mapToSource(index);

    BtmItem *node = static_cast<BtmItem*>(originalIndex.internalPointer());
    return node->getBkm();
}


QAbstractItemModel* BookmarksPanel::model()
{
    return _bkTreeModel;
}
