/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2009 by Nils Weigel <nehlsen at gmail dot com>
* Copyright (C) 2010 by Andrea Diamantini <adjam7 at gmail dot com>
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

// Local Includes
#include "bookmarksmanager.h"
#include "bookmarkstreemodel.h"
#include "bookmarksproxy.h"

// Auto Includes
#include "rekonq.h"

// Qt includes
#include <QHBoxLayout>
#include <QLabel>
#include <QTreeView>
#include <QHeaderView>

// KDE includes
#include <KLineEdit>
#include <KLocalizedString>
#include <KAction>
#include <KMenu>
#include <KBookmarkDialog>
#include <KMessageBox>


BookmarksPanel::BookmarksPanel(const QString &title, QWidget *parent, Qt::WindowFlags flags)
    : QDockWidget(title, parent, flags),
    m_treeView(new UrlTreeView(this)),
    m_ac(new KActionCollection(this)),
    m_loadingState(false)
{
    setup();
    setupActions();
    setShown(ReKonfig::showBookmarksPanel());
}


BookmarksPanel::~BookmarksPanel()
{
    ReKonfig::setShowBookmarksPanel(!isHidden());
}


void BookmarksPanel::setup()
{
    setObjectName("bookmarksPanel");
    setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

    QWidget *ui = new QWidget(this);

    // setup search bar
    QHBoxLayout *searchLayout = new QHBoxLayout;
    searchLayout->setContentsMargins(5, 0, 0, 0);
    QLabel *searchLabel = new QLabel(i18n("&Search:"));
    searchLayout->addWidget(searchLabel);
    KLineEdit *search = new KLineEdit;
    search->setClearButtonShown(true);
    searchLayout->addWidget(search);
    searchLabel->setBuddy( search );

    // setup tree view
    m_treeView->setUniformRowHeights(true);
    m_treeView->setTextElideMode(Qt::ElideMiddle);
    m_treeView->setAlternatingRowColors(true);
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

    BookmarksTreeModel *model = new BookmarksTreeModel( this );
    BookmarksProxy *proxy = new BookmarksProxy(ui);
    proxy->setSourceModel( model );
    m_treeView->setModel( proxy );

    connect(m_treeView, SIGNAL(contextMenuItemRequested(const QPoint &)), this, SLOT(contextMenuBk(const QPoint &)));
    connect(m_treeView, SIGNAL(contextMenuGroupRequested(const QPoint &)), this, SLOT(contextMenuBkGroup(const QPoint &)));
    connect(m_treeView, SIGNAL(contextMenuEmptyRequested(const QPoint &)), this, SLOT(contextMenuBlank(const QPoint &)));
    connect(m_treeView, SIGNAL(delKeyPressed()), this, SLOT(deleteBookmark()));
    connect(m_treeView, SIGNAL(collapsed(const QModelIndex &)), this, SLOT(onCollapse(const QModelIndex &)));
    connect(m_treeView, SIGNAL(expanded(const QModelIndex &)), this, SLOT(onExpand(const QModelIndex &)));
    connect(search, SIGNAL(textChanged(const QString &)), proxy, SLOT(setFilterFixedString(const QString &)));
    loadFoldedState();
}


void BookmarksPanel::onCollapse(const QModelIndex &index)
{
    if(m_loadingState)
        return;

    KBookmark bookmark = bookmarkForIndex(index);
    bookmark.internalElement().setAttribute("folded", "yes");
    emit saveOnlyRequested();
}


void BookmarksPanel::onExpand(const QModelIndex &index)
{
    if(m_loadingState)
        return;

    KBookmark bookmark = bookmarkForIndex(index);
    bookmark.internalElement().setAttribute("folded", "no");
    emit saveOnlyRequested();
}


void BookmarksPanel::loadFoldedState()
{
    m_loadingState = true;
    loadFoldedState(QModelIndex());
    m_loadingState = false;
}


void BookmarksPanel::loadFoldedState(const QModelIndex &root)
{

    int count = m_treeView->model()->rowCount(root);
    QModelIndex index;

    for(int i = 0; i < count; i++)
    {
        index = m_treeView->model()->index(i, 0, root);
        if(index.isValid() && bookmarkForIndex(index).isGroup())
        {
            m_treeView->setExpanded(index, bookmarkForIndex(index).toGroup().isOpen());
            loadFoldedState(index);
        }
    }
}


void BookmarksPanel::setupActions()
{
    KAction* action;

    action = new KAction(KIcon("tab-new"), i18n("Open"), this);
    connect(action, SIGNAL(triggered()), m_treeView, SLOT(openInCurrentTab()));
    m_ac->addAction("open", action);

    action = new KAction(KIcon("tab-new"), i18n("Open in New Tab"), this);
    connect(action, SIGNAL(triggered()), m_treeView, SLOT(openInNewTab()));
    m_ac->addAction("open_tab", action);

    action = new KAction(KIcon("window-new"), i18n("Open in New Window"), this);
    connect(action, SIGNAL(triggered()), m_treeView, SLOT(openInNewWindow()));
    m_ac->addAction("open_window", action);

    action = new KAction(KIcon("rating"), i18n("Bookmark Current Page"), this);
    connect(action, SIGNAL(triggered()), this, SLOT(bookmarkCurrentPage()));
    m_ac->addAction("bookmark_page", action);

    action = new KAction(KIcon("bookmark-new"), i18n("New Bookmark"), this);
    connect(action, SIGNAL(triggered()), this, SLOT(newBookmark()));
    m_ac->addAction("bookmark_new", action);

    action = new KAction(KIcon("folder-new"), i18n("New Bookmark Folder"), this);
    connect(action, SIGNAL(triggered()), this, SLOT(newBookmarkGroup()));
    m_ac->addAction("folder_new", action);

    action = new KAction(KIcon("edit-clear"), i18n("New Separator"), this);
    connect(action, SIGNAL(triggered()), this, SLOT(newSeparator()));
    m_ac->addAction("separator_new", action);

    action = new KAction(KIcon("edit-copy"), i18n("Copy Link Adress"), this);
    connect(action, SIGNAL(triggered()), m_treeView, SLOT(copyToClipboard()));
    m_ac->addAction("copy", action);

    action = new KAction(KIcon("edit-delete"), i18n("Delete Bookmark"), this);
    connect(action, SIGNAL(triggered()), this, SLOT(deleteBookmark()));
    m_ac->addAction("delete", action);

    action = new KAction(KIcon("configure"), i18n("Properties"), this);
    connect(action, SIGNAL(triggered()), this, SLOT(editBookmark()));
    m_ac->addAction("properties", action);

    action = new KAction(KIcon("tab-new"), i18n("Open Folder in Tabs"), this);
    connect(action, SIGNAL(triggered()), this, SLOT(openFolderInTabs()));
    m_ac->addAction("open_all", action);
}


KBookmark BookmarksPanel::bookmarkForIndex(const QModelIndex &index)
{
    if(!index.isValid())
        return KBookmark();

    const QAbstractProxyModel* proxyModel = dynamic_cast< const QAbstractProxyModel* >(index.model());
    QModelIndex originalIndex = proxyModel->mapToSource(index);

    BtmItem *node = static_cast< BtmItem* >( originalIndex.internalPointer() );
    return node->getBkm();
}


void BookmarksPanel::contextMenuBk(const QPoint &pos)
{
    QPoint position = m_treeView->mapToGlobal(pos);
    QModelIndex index = m_treeView->indexAt(pos);
    if(!index.isValid() || m_loadingState)
        return;

    KBookmark selected = bookmarkForIndex(index);

    if(selected.isGroup())
    {
        contextMenuBkGroup(pos, true);
        return;
    }

    if(selected.isSeparator())
    {
        contextMenuSeparator(pos);
        return;
    }

    KMenu *menu = new KMenu(this);

    menu->addAction(m_ac->action("open"));
    menu->addAction(m_ac->action("open_tab"));
    menu->addAction(m_ac->action("open_window"));

    menu->addSeparator();

    menu->addAction(m_ac->action("bookmark_page"));
    menu->addAction(m_ac->action("bookmark_new"));
    menu->addAction(m_ac->action("folder_new"));
    menu->addAction(m_ac->action("separator_new"));

    menu->addSeparator();

    menu->addAction(m_ac->action("copy"));

    menu->addSeparator();

    menu->addAction(m_ac->action("delete"));
    menu->addAction(m_ac->action("properties"));

    menu->popup(position);
}


void BookmarksPanel::contextMenuBkGroup(const QPoint &pos, bool emptyGroup)
{
    if(m_loadingState)
        return;

    QPoint position = m_treeView->mapToGlobal(pos);
    KMenu *menu = new KMenu(this);

    if(!emptyGroup)
    {
        menu->addAction(m_ac->action("open_all"));
        menu->addSeparator();
    }

    menu->addAction(m_ac->action("bookmark_page"));
    menu->addAction(m_ac->action("bookmark_new"));
    menu->addAction(m_ac->action("folder_new"));
    menu->addAction(m_ac->action("separator_new"));

    menu->addSeparator();

    menu->addAction(m_ac->action("delete"));
    menu->addAction(m_ac->action("properties"));

    menu->popup(position);
}


void BookmarksPanel::contextMenuSeparator(const QPoint &pos)
{
    QPoint position = m_treeView->mapToGlobal(pos);
    KMenu *menu = new KMenu(this);

    menu->addAction(m_ac->action("bookmark_page"));
    menu->addAction(m_ac->action("bookmark_new"));
    menu->addAction(m_ac->action("folder_new"));
    menu->addAction(m_ac->action("separator_new"));

    menu->addSeparator();

    menu->addAction(m_ac->action("delete"));

    menu->popup(position);
}


void BookmarksPanel::contextMenuBlank(const QPoint &pos)
{
    QPoint position = m_treeView->mapToGlobal(pos);
    KMenu *menu = new KMenu(this);

    menu->addAction(m_ac->action("bookmark_page"));
    menu->addAction(m_ac->action("bookmark_new"));
    menu->addAction(m_ac->action("folder_new"));
    menu->addAction(m_ac->action("separator_new"));

    menu->popup(position);
}


void BookmarksPanel::deleteBookmark()
{
    QModelIndex index = m_treeView->currentIndex();
    if(!index.isValid())
        return;

    KBookmark bm = bookmarkForIndex(index);
    bool folder = bm.isGroup();

    if (KMessageBox::warningContinueCancel(
            QApplication::activeWindow(),
            folder ? i18n("Are you sure you wish to remove the bookmark folder\n\"%1\"?", bm.text())
                   : i18n("Are you sure you wish to remove the bookmark\n\"%1\"?", bm.text()),
            folder ? i18n("Bookmark Folder Deletion")
                   : i18n("Bookmark Deletion"),
            KStandardGuiItem::del())
          != KMessageBox::Continue
        )
        return;


    bm.parentGroup().deleteBookmark(bm);
    Application::instance()->bookmarkProvider()->bookmarkManager()->emitChanged();
}


void BookmarksPanel::editBookmark()
{
    QModelIndex index = m_treeView->currentIndex();
    if(!index.isValid())
        return;

    KBookmark selected = bookmarkForIndex(index);

    KBookmarkDialog *dialog = Application::bookmarkProvider()->bookmarkOwner()->bookmarkDialog(Application::bookmarkProvider()->bookmarkManager(), QApplication::activeWindow());
    dialog->editBookmark(selected);
    delete dialog;
}


void BookmarksPanel::openFolderInTabs()
{
    QModelIndex index = m_treeView->currentIndex();
    if(!index.isValid() || !bookmarkForIndex(index).isGroup())
        return;

    QList<KUrl> allChild = bookmarkForIndex(index).toGroup().groupUrlList();

    if(allChild.length() > 8) // 8, a good choice ?
    {
        if(!(KMessageBox::warningContinueCancel(this, i18n("You are about to open a lot of tabs : %1\nAre you sure  ?", QString::number(allChild.length()))) == KMessageBox::Continue))
            return;
    }

    for(int i = 0; i < allChild.length(); i++)
        emit openUrl(allChild.at(i).url(), Rekonq::SettingOpenTab);
}


void BookmarksPanel::newBookmark()
{
    QModelIndex index = m_treeView->currentIndex();

    KBookmark selected;
    KBookmark newBk;

    KBookmarkDialog *dialog = Application::bookmarkProvider()->bookmarkOwner()->bookmarkDialog(Application::bookmarkProvider()->bookmarkManager(), QApplication::activeWindow());

    if(index.isValid())
    {
        selected = bookmarkForIndex(index);

         if(selected.isGroup())
            newBk = dialog->addBookmark("New bookmark", KUrl(), selected);
        else
            newBk = dialog->addBookmark("New bookmark", KUrl(), selected.parentGroup());
    }

    else
    {
        newBk = dialog->addBookmark("New bookmark", KUrl());
    }

    delete dialog;

    // a click on cancel
    if(newBk.isNull())
        return;

    // addBookmark already added the bookmark, but without the default favicon
    KBookmarkGroup parent = newBk.parentGroup();
    parent.deleteBookmark(newBk);
    newBk.setIcon(("text-html"));
    parent.addBookmark(newBk);

    if(index.isValid())
        parent.moveBookmark(newBk, selected);

    Application::bookmarkProvider()->bookmarkManager()->emitChanged();
}


void BookmarksPanel::newBookmarkGroup()
{
    QModelIndex index = m_treeView->currentIndex();
    KBookmark newBk;

    KBookmarkDialog *dialog = Application::bookmarkProvider()->bookmarkOwner()->bookmarkDialog(Application::bookmarkProvider()->bookmarkManager(), QApplication::activeWindow());

    if(index.isValid())
    {
        KBookmark selected = bookmarkForIndex(index);

        if(selected.isGroup())
        {
            newBk = dialog->createNewFolder("New folder", selected);
        }

        else
        {
            newBk = dialog->createNewFolder("New folder", selected.parentGroup());
            selected.parentGroup().moveBookmark(newBk, selected);
            Application::bookmarkProvider()->bookmarkManager()->emitChanged();
        } 
    }

    else
    {
        dialog->createNewFolder("New folder");
    }

    delete dialog;
}


void BookmarksPanel::newSeparator()
{
    QModelIndex index = m_treeView->currentIndex();

    KBookmark selected;
    KBookmark newBk;

    if(index.isValid())
    {
        selected = bookmarkForIndex(index);

        if(selected.isGroup())
            newBk = selected.toGroup().createNewSeparator();
        else
            newBk = selected.parentGroup().createNewSeparator();
    }

    else
    {
        newBk = Application::bookmarkProvider()->rootGroup().createNewSeparator();
    }

    KBookmarkGroup parent = newBk.parentGroup();
    newBk.setIcon(("edit-clear"));
    parent.addBookmark(newBk);

    if(index.isValid())
        parent.moveBookmark(newBk, selected);

    Application::bookmarkProvider()->bookmarkManager()->emitChanged();
}


void BookmarksPanel::bookmarkCurrentPage()
{
    QModelIndex index = m_treeView->currentIndex();
    KBookmarkGroup parent = Application::bookmarkProvider()->rootGroup();

    if(index.isValid())
    {
        KBookmark selected = bookmarkForIndex(index);
        parent = selected.parentGroup();

        if(selected.isGroup())
            parent = selected.toGroup();

        KBookmark newBk = parent.addBookmark(Application::bookmarkProvider()->bookmarkOwner()->currentTitle(), KUrl(Application::bookmarkProvider()->bookmarkOwner()->currentUrl()), "text-html");
        parent.moveBookmark(newBk, selected);
    }

    else
    {
       parent.addBookmark(Application::bookmarkProvider()->bookmarkOwner()->currentTitle(), KUrl(Application::bookmarkProvider()->bookmarkOwner()->currentUrl()), "text-html");
    }

    Application::bookmarkProvider()->bookmarkManager()->emitChanged();
}
