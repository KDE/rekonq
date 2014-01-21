/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2008-2014 by Andrea Diamantini <adjam7 at gmail dot com>
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


// Self Includes
#include "bookmarksmenu.h"
#include "bookmarksmenu.moc"

// Local Includes
#include "bookmarkscontextmenu.h"
#include "bookmarkmanager.h"
#include "bookmarkowner.h"

#include "iconmanager.h"
#include "webwindow.h"

// Qt Includes
#include <QFrame>
#include <QActionEvent>
#include <QApplication>


BookmarkMenu::BookmarkMenu(KBookmarkManager *manager,
                           KBookmarkOwner *owner,
                           KMenu *menu,
                           KActionCollection* actionCollection)
    : KBookmarkMenu(manager, owner, menu, actionCollection)
{
}


BookmarkMenu::BookmarkMenu(KBookmarkManager  *manager,
                           KBookmarkOwner  *owner,
                           KMenu  *parentMenu,
                           const QString &parentAddress)
    : KBookmarkMenu(manager, owner, parentMenu, parentAddress)
{
}


BookmarkMenu::~BookmarkMenu()
{
    qDebug() << "Deleting BookmarkMenu.. See http://svn.reviewboard.kde.org/r/5606/ about.";
}


KMenu * BookmarkMenu::contextMenu(QAction *act)
{
    KBookmarkActionInterface* action = dynamic_cast<KBookmarkActionInterface *>(act);
    if (!action)
        return 0;
    return new BookmarksContextMenu(action->bookmark(), manager(), static_cast<BookmarkOwner*>(owner()));
}


QAction * BookmarkMenu::actionForBookmark(const KBookmark &bookmark)
{
    if (bookmark.isGroup())
    {
        KBookmarkActionMenu *actionMenu = new KBookmarkActionMenu(bookmark, this);
        BookmarkMenu *menu = new BookmarkMenu(manager(), owner(), actionMenu->menu(), bookmark.address());
        // An hack to get rid of bug 219274
        connect(actionMenu, SIGNAL(hovered()), menu, SLOT(slotAboutToShow()));
        return actionMenu;
    }
    else if (bookmark.isSeparator())
    {
        return KBookmarkMenu::actionForBookmark(bookmark);
    }
    else
    {
        KBookmarkAction *action = new KBookmarkAction(bookmark, owner(), this);
        action->setIcon(IconManager::self()->iconForUrl(KUrl(bookmark.url())));
        return action;
    }
}


void BookmarkMenu::refill()
{
    clear();
    
    if (isRoot())
    {
        addAddBookmark();
        addAddBookmarksList();
        
        if (parentMenu()->actions().count() > 0)
            parentMenu()->addSeparator();

        WebWindow *w = qobject_cast<WebWindow *>(parentMenu()->parent());
        QAction *a;
        // bk page
        a = w->actionByName(QL1S("open_bookmarks_page"));
        parentMenu()->addAction(a);
        a = w->actionByName(QL1S("show_bookmarks_toolbar"));
        parentMenu()->addAction(a);
        
        addEditBookmarks();
        
        if (parentMenu()->actions().count() > 0)
            parentMenu()->addSeparator();
    }

    fillBookmarks();
    
    if (!isRoot())
    {
        if (parentMenu()->actions().count() > 0)
            parentMenu()->addSeparator();

        addOpenFolderInTabs();
        addAddBookmarksList();
    }
}


void BookmarkMenu::addOpenFolderInTabs()
{
    KBookmarkGroup group = manager()->findByAddress(parentAddress()).toGroup();

    if (!group.first().isNull())
    {
        KBookmark bookmark = group.first();

        while (bookmark.isGroup() || bookmark.isSeparator())
        {
            bookmark = group.next(bookmark);
        }

        if (!bookmark.isNull())
        {
            parentMenu()->addAction(BookmarkManager::self()->owner()->createAction(group, BookmarkOwner::OPEN_FOLDER));
        }
    }
}
