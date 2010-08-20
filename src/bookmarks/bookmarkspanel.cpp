/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2009 by Nils Weigel <nehlsen at gmail dot com>
* Copyright (C) 2010 by Andrea Diamantini <adjam7 at gmail dot com>
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

// Local Includes
#include "application.h"
#include "bookmarkprovider.h"
#include "bookmarkstreemodel.h"
#include "bookmarksproxy.h"
#include "bookmarkscontextmenu.h"
#include "bookmarkowner.h"
#include "paneltreeview.h"

// Auto Includes
#include "rekonq.h"

// Qt includes
#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QHeaderView>

// KDE includes
#include <KLineEdit>

BookmarksPanel::BookmarksPanel(const QString &title, QWidget *parent, Qt::WindowFlags flags)
        : QDockWidget(title, parent, flags)
        , m_treeView(new PanelTreeView(this))
        , m_loadingState(false)
        , m_loaded(false)
{
    setObjectName("bookmarksPanel");
    setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

    connect(this, SIGNAL(visibilityChanged(bool)), this, SLOT(showing(bool)));

    setVisible(ReKonfig::showBookmarksPanel());
}


BookmarksPanel::~BookmarksPanel()
{
    ReKonfig::setShowBookmarksPanel(false);
}


void BookmarksPanel::showing(bool b)
{
    if(b && !m_loaded)
        setup();
}


void BookmarksPanel::startLoadFoldedState()
{
    m_loadingState = true;
    loadFoldedState(QModelIndex());
    m_loadingState = false;
}


void BookmarksPanel::contextMenu(const QPoint &pos)
{
    if (m_loadingState)
        return;

    BookmarksContextMenu menu(bookmarkForIndex( m_treeView->indexAt(pos) ),
                              Application::bookmarkProvider()->bookmarkManager(),
                              Application::bookmarkProvider()->bookmarkOwner(),
                              this
                             );

    menu.exec(m_treeView->mapToGlobal(pos));
}


void BookmarksPanel::deleteBookmark()
{
    QModelIndex index = m_treeView->currentIndex();
    if (m_loadingState || !index.isValid())
        return;

    Application::bookmarkProvider()->bookmarkOwner()->deleteBookmark(bookmarkForIndex(index));
}


void BookmarksPanel::onCollapse(const QModelIndex &index)
{
    if (m_loadingState)
        return;

    bookmarkForIndex(index).internalElement().setAttribute("folded", "yes");
    emit expansionChanged();
}


void BookmarksPanel::onExpand(const QModelIndex &index)
{
    if (m_loadingState)
        return;

    bookmarkForIndex(index).internalElement().setAttribute("folded", "no");
    emit expansionChanged();
}


void BookmarksPanel::loadFoldedState(const QModelIndex &root)
{
    int count = m_treeView->model()->rowCount(root);
    QModelIndex index;

    for (int i = 0; i < count; ++i)
    {
        index = m_treeView->model()->index(i, 0, root);
        if (index.isValid())
        {
            KBookmark bm = bookmarkForIndex(index);
            if (bm.isGroup())
            {
                m_treeView->setExpanded(index, bm.toGroup().isOpen());
                loadFoldedState(index);
            }
        }
    }
}


void BookmarksPanel::setup()
{
    kDebug() << "Loading bookmarks panel setup...";

    QWidget *ui = new QWidget(this);

    // setup search bar
    QHBoxLayout *searchLayout = new QHBoxLayout;
    searchLayout->setContentsMargins(5, 0, 0, 0);
    QLabel *searchLabel = new QLabel(i18n("&Search:"));
    searchLayout->addWidget(searchLabel);
    KLineEdit *search = new KLineEdit;
    search->setClearButtonShown(true);
    searchLayout->addWidget(search);
    searchLabel->setBuddy(search);

    // setup tree view
    m_treeView->setUniformRowHeights(true);
    m_treeView->header()->hide();
    m_treeView->setDragEnabled(true);
    m_treeView->setAutoExpandDelay(750);
    m_treeView->setDefaultDropAction(Qt::MoveAction);
    m_treeView->viewport()->setAcceptDrops(true);

    // put everything together
    QVBoxLayout *vBoxLayout = new QVBoxLayout;
    vBoxLayout->setContentsMargins(0, 0, 0, 0);
    vBoxLayout->addLayout(searchLayout);
    vBoxLayout->addWidget(m_treeView);

    // add it to the UI
    ui->setLayout(vBoxLayout);
    setWidget(ui);

    BookmarksTreeModel *model = new BookmarksTreeModel(this);
    BookmarksProxy *proxy = new BookmarksProxy(ui);
    proxy->setSourceModel(model);
    m_treeView->setModel(proxy);

    connect(search, SIGNAL(textChanged(const QString &)), proxy, SLOT(setFilterFixedString(const QString &)));

    connect(model, SIGNAL(bookmarksUpdated()), this, SLOT(startLoadFoldedState()));

    connect(m_treeView, SIGNAL(contextMenuItemRequested(const QPoint &)), this, SLOT(contextMenu(const QPoint &)));
    connect(m_treeView, SIGNAL(contextMenuGroupRequested(const QPoint &)), this, SLOT(contextMenu(const QPoint &)));
    connect(m_treeView, SIGNAL(contextMenuEmptyRequested(const QPoint &)), this, SLOT(contextMenu(const QPoint &)));
    connect(m_treeView, SIGNAL(delKeyPressed()), this, SLOT(deleteBookmark()));
    connect(m_treeView, SIGNAL(collapsed(const QModelIndex &)), this, SLOT(onCollapse(const QModelIndex &)));
    connect(m_treeView, SIGNAL(expanded(const QModelIndex &)), this, SLOT(onExpand(const QModelIndex &)));

    startLoadFoldedState();

    m_loaded = true;
}


KBookmark BookmarksPanel::bookmarkForIndex(const QModelIndex &index)
{
    if (!index.isValid())
        return KBookmark();

    const BookmarksProxy *proxyModel = static_cast<const BookmarksProxy*>(index.model());
    QModelIndex originalIndex = proxyModel->mapToSource(index);

    BtmItem *node = static_cast<BtmItem*>(originalIndex.internalPointer());
    return node->getBkm();
}
