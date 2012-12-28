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


#ifndef BOOKMARKS_CONTEXT_MENU_H
#define BOOKMARKS_CONTEXT_MENU_H


// KDE Includes
#include <KBookmarkMenu>

// Forward Declarations
class BookmarkOwner;


class BookmarksContextMenu : public KBookmarkContextMenu
{
public:
    BookmarksContextMenu(const KBookmark &bookmark,
                         KBookmarkManager *manager,
                         BookmarkOwner *owner,
                         bool nullForced = false,
                         QWidget *parent = 0);
    virtual void addActions();

private:
    void addFolderActions();
    void addBookmarkActions();
    void addSeparatorActions();
    void addNullActions();

    BookmarkOwner *m_bmOwner;
    bool m_nullForced;
};

#endif // BOOKMARKS_CONTEXT_MENU_H
