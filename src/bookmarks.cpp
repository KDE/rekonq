/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2008 by Andrea Diamantini <adjam7 at gmail dot com>
*
*
* This program is free software; you can redistribute it
* and/or modify it under the terms of the GNU General
* Public License as published by the Free Software Foundation;
* either version 2, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* ============================================================ */



// Self Includes
#include "bookmarks.h"
#include "bookmarks.moc"

// Local Includes
#include "mainwindow.h"
#include "webview.h"
#include "application.h"

// KDE Includes
#include <KActionCollection>
#include <KBookmark>
#include <KBookmarkAction>
#include <KBookmarkActionMenu>
#include <KBookmarkGroup>
#include <KBookmarkMenu>
#include <KToolBar>
#include <KDebug>
#include <KMenu>
#include <KMimeType>
#include <KStandardDirs>
#include <KUrl>

// Qt Includes
#include <QActionGroup>
#include <QFile>


BookmarkOwner::BookmarkOwner(QObject *parent)
        : QObject(parent)
        , KBookmarkOwner()
{
}


void BookmarkOwner::openBookmark(const KBookmark & bookmark, 
                                 Qt::MouseButtons mouseButtons, 
                                 Qt::KeyboardModifiers keyboardModifiers)
{
    Q_UNUSED(mouseButtons) 
    Q_UNUSED(keyboardModifiers)

    // FIXME this is workaround for double call issue
    // When middle mouse button is clicked this method is called twice
    static bool isDouble = false;
    if (isDouble)
    {
        isDouble = false;
        return;
    }
    //--

    emit openUrl(bookmark.url());
}


QString BookmarkOwner::currentUrl() const
{
    return Application::instance()->mainWindow()->currentTab()->url().url();
}


QString BookmarkOwner::currentTitle() const
{
    return Application::instance()->mainWindow()->currentTab()->title();
}


// QList< QPair<QString, QString> > BookmarkOwner::currentBookmarkList() const
// {
//     QList< QPair<QString, QString> > list;
//     QList<WebView *> tabs = Application::instance()->mainWindow()->mainView()->tabs();
//     foreach(WebView *tab, tabs)
//     {
//         QString url = tab->url().url();
//         QString title = tab->title();
//         list.append(QPair<QString, QString>(url, title));
//     }
//     return list;
// }


// ------------------------------------------------------------------------------------------------------


BookmarkMenu::BookmarkMenu(KBookmarkManager *manager,
                           KBookmarkOwner *owner,
                           KMenu *menu,
                           KActionCollection* actionCollection)
        : KBookmarkMenu(manager, owner, menu, actionCollection) 
        
{
    actionCollection->addAction(KStandardAction::AddBookmark,
                                QLatin1String("add_bookmark_payload"), 
                                this, SLOT(slotAddBookmark()));

}

BookmarkMenu::~BookmarkMenu()
{
}

        
KMenu *BookmarkMenu::viewContextMenu(QAction *action)
{
    return contextMenu(action);
}


void BookmarkMenu::slotAddBookmark()
{
    KAction *action = qobject_cast<KAction *>(sender());
    if (action && !action->data().isNull())
    {
        KBookmarkGroup parentBookmark = manager()->findByAddress(parentAddress()).toGroup();
        /// TODO Add bookmark Icon
        parentBookmark.addBookmark(owner()->currentTitle(), action->data().toUrl());
        manager()->emitChanged();
        return;
    }
    
    KBookmarkMenu::slotAddBookmark();
}


// ------------------------------------------------------------------------------------------------------


BookmarkProvider::BookmarkProvider(QWidget *parent)
        : QWidget(parent)
        , m_manager(NULL)
        , m_owner(NULL)
        , m_menu(new KMenu(this))
        , m_actionCollection(new KActionCollection(this))
        , m_bookmarkMenu(NULL)
        , m_bookmarkToolBar(NULL)
{
    KUrl bookfile = KUrl("~/.kde/share/apps/konqueror/bookmarks.xml");  // share konqueror bookmarks

    if (!QFile::exists(bookfile.path()))
    {
        bookfile = KUrl("~/.kde4/share/apps/konqueror/bookmarks.xml");
        if (!QFile::exists(bookfile.path()))
        {
            QString bookmarksDefaultPath = KStandardDirs::locate("appdata" , "defaultbookmarks.xbel");
            kWarning() << bookmarksDefaultPath;
            QFile bkms(bookmarksDefaultPath);
            QString bookmarksPath = KStandardDirs::locateLocal("appdata", "bookmarks.xml", true);
            bookmarksPath.replace("rekonq", "konqueror");
            bkms.copy(bookmarksPath);

            bookfile = KUrl(bookmarksPath);
        }
    }
    m_manager = KBookmarkManager::managerForExternalFile(bookfile.path());
    connect(m_manager, SIGNAL(changed(const QString &, const QString &)), 
            this, SLOT(slotBookmarksChanged(const QString &, const QString &)));

    // setup menu
    m_owner = new BookmarkOwner(this);
    connect(m_owner, SIGNAL(openUrl(const KUrl& )), this, SIGNAL(openUrl(const KUrl& )));
    m_bookmarkMenu = new BookmarkMenu(m_manager, m_owner, m_menu, m_actionCollection);

    // setup toolbar
    setupToolBar();

}

BookmarkProvider::~BookmarkProvider()
{
    delete m_bookmarkToolBar;
    delete m_bookmarkMenu;
    delete m_actionCollection;
    delete m_menu;
    delete m_owner;
    delete m_manager;
}


void BookmarkProvider::setupToolBar()
{
    m_bookmarkToolBar = new KToolBar(this);
    m_bookmarkToolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    m_bookmarkToolBar->setIconDimensions(16);
    m_bookmarkToolBar->setAcceptDrops(true);
    m_bookmarkToolBar->setContentsMargins(0, 0, 0, 0);
    m_bookmarkToolBar->setMinimumHeight(16);
    m_bookmarkToolBar->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_bookmarkToolBar, SIGNAL(customContextMenuRequested(const QPoint &)), 
            this, SLOT(contextMenu(const QPoint &)));
             
    slotBookmarksChanged("", "");
}


void BookmarkProvider::slotBookmarksChanged(const QString &group, const QString &caller)
{
    Q_UNUSED(group)
    Q_UNUSED(caller)

    if (!m_bookmarkToolBar)
    {
        kWarning() << "There is no bookmark toolbar";
        return;
    }
    
    KActionCollection bookmarkCollection(this);
    
    KBookmarkGroup toolBarGroup = m_manager->toolbar();
    if (toolBarGroup.isNull())
        return;
    
    KBookmark bookmark = toolBarGroup.first();
    while (!bookmark.isNull()) {
        if (!bookmark.isGroup())
        {
            KAction *action = new KBookmarkAction(bookmark, m_owner, this);
            QString text = bookmark.address();
            bookmarkCollection.addAction(text, action);
        }
        bookmark = toolBarGroup.next(bookmark);
    }
    m_bookmarkToolBar->clear();
    m_bookmarkToolBar->addActions(bookmarkCollection.actions());
}


QAction *BookmarkProvider::actionByName(const QString &name)
{   
    QAction *action = m_actionCollection->action(name);
    if (action)
        return action;
    /* else */
    kWarning() << "Action named: " << name << " not found, returning empty action.";
    return new QAction(this);  // return empty object instead of NULL pointer
}


void BookmarkProvider::contextMenu(const QPoint &point)
{
    KAction* action = dynamic_cast<KAction*>(m_bookmarkToolBar->actionAt(point));
    if (!action)
        return;
    KMenu *menu = m_bookmarkMenu->viewContextMenu(action);
    menu->popup(m_bookmarkToolBar->mapToGlobal(point));
}


KActionMenu* BookmarkProvider::bookmarkActionMenu()
{
    KActionMenu *bookmarkActionMenu = new KActionMenu(this);
    bookmarkActionMenu->setMenu(m_menu);
    bookmarkActionMenu->setText(i18n("&Bookmarks"));
    return bookmarkActionMenu;
}


KAction* BookmarkProvider::bookmarkToolBarAction()
{
    KAction *bookmarkToolBarAction = new KAction(this);
    bookmarkToolBarAction->setDefaultWidget(m_bookmarkToolBar);  // The ownership is transferred to action
    bookmarkToolBarAction->setText(i18n("Bookmarks Bar"));
    bookmarkToolBarAction->setShortcutConfigurable(false);
    return bookmarkToolBarAction;
}

