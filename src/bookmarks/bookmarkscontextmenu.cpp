/* ============================================================
*
* This file is a part of the rekonq project
*
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
#include "bookmarkscontextmenu.h"

// Local Includes
#include "bookmarkowner.h"


BookmarksContextMenu::BookmarksContextMenu(const KBookmark &bookmark, KBookmarkManager *manager, BookmarkOwner *owner, QWidget *parent)
        : KBookmarkContextMenu(bookmark, manager, owner, parent)
        , m_bmOwner(owner)
{
}


BookmarksContextMenu::~BookmarksContextMenu()
{
}


void BookmarksContextMenu::addBookmarkActions()
{
    addAction(m_bmOwner->createAction(bookmark(), BookmarkOwner::OPEN_IN_TAB));
    addAction(m_bmOwner->createAction(bookmark(), BookmarkOwner::OPEN_IN_WINDOW));

    addSeparator();

    addAction(m_bmOwner->createAction(bookmark(), BookmarkOwner::BOOKMARK_PAGE));
    addAction(m_bmOwner->createAction(bookmark(), BookmarkOwner::NEW_FOLDER));
    addAction(m_bmOwner->createAction(bookmark(), BookmarkOwner::NEW_SEPARATOR));

    addSeparator();

    addAction(m_bmOwner->createAction(bookmark(), BookmarkOwner::COPY));

    addSeparator();

    addAction(m_bmOwner->createAction(bookmark(), BookmarkOwner::EDIT));
    addAction(m_bmOwner->createAction(bookmark(), BookmarkOwner::DELETE));
}


void BookmarksContextMenu::addFolderActions()
{
    KBookmarkGroup group = bookmark().toGroup();

    if (!group.first().isNull())
    {
        KBookmark child = group.first();

        while (child.isGroup() || child.isSeparator())
        {
            child = group.next(child);
        }

        if (!child.isNull())
        {
            addAction(m_bmOwner->createAction(bookmark(), BookmarkOwner::OPEN_FOLDER));
            addSeparator();
        }
    }

    addAction(m_bmOwner->createAction(bookmark(), BookmarkOwner::BOOKMARK_PAGE));
    addAction(m_bmOwner->createAction(bookmark(), BookmarkOwner::NEW_FOLDER));
    addAction(m_bmOwner->createAction(bookmark(), BookmarkOwner::NEW_SEPARATOR));

    addSeparator();

    addAction(m_bmOwner->createAction(bookmark(), BookmarkOwner::EDIT));
    addAction(m_bmOwner->createAction(bookmark(), BookmarkOwner::DELETE));
}


void BookmarksContextMenu::addSeparatorActions()
{
    addAction(m_bmOwner->createAction(bookmark(), BookmarkOwner::BOOKMARK_PAGE));
    addAction(m_bmOwner->createAction(bookmark(), BookmarkOwner::NEW_FOLDER));
    addAction(m_bmOwner->createAction(bookmark(), BookmarkOwner::NEW_SEPARATOR));

    addSeparator();

    addAction(m_bmOwner->createAction(bookmark(), BookmarkOwner::DELETE));
}


void BookmarksContextMenu::addNullActions()
{
    addAction(m_bmOwner->createAction(bookmark(), BookmarkOwner::BOOKMARK_PAGE));
    addAction(m_bmOwner->createAction(bookmark(), BookmarkOwner::NEW_FOLDER));
    addAction(m_bmOwner->createAction(bookmark(), BookmarkOwner::NEW_SEPARATOR));
}


void BookmarksContextMenu::addActions()
{
    if (bookmark().isGroup())
    {
        addFolderActions();
    }
    else if (bookmark().isSeparator())
    {
        addSeparatorActions();
    }
    else if (bookmark().isNull())
    {
        addNullActions();
    }
    else
    {
        addBookmarkActions();
    }
}
