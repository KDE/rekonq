/* ============================================================
 *
 * This file is a part of the reKonq project
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef BOOKMARKS_H
#define BOOKMARKS_H

#include <KBookmarkOwner>
#include <KBookmarkManager>
#include <KBookmarkMenu>


class OwnBookMarks : public KBookMarkOwner
{
Q_OBJECT
public:
    OwnBookMarks(KMainWindow *parent);

    virtual void openBookmark (const KBookmark & , Qt::MouseButtons , Qt::KeyboardModifiers );
};


class BookmarkMenu : public KMenu
{
Q_OBJECT
public:
    BookmarkMenu(KMainWindow *parent);

private:
    void setActions();

    KBookmarkManager *m_manager;
    OwnBookMarks *m_owner;
    KActionCollection *m_ac;
    KBookmarkMenu *m_menu;
    KMainWindow *m_parent;
};

#endif

