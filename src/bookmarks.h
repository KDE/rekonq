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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef BOOKMARKS_H
#define BOOKMARKS_H

// KDE Includes
#include <KBookmarkOwner>
#include <KBookmarkManager>
#include <KBookmarkMenu>
#include <KUrl>
#include <KMenu>
#include <KActionCollection>
#include <KMainWindow>

class MainWindow;

/*
 *
 */
class OwnBookMarks : public QObject , public KBookmarkOwner
{
Q_OBJECT
public:
    OwnBookMarks(KMainWindow *);

    virtual void openBookmark (const KBookmark & , Qt::MouseButtons , Qt::KeyboardModifiers );

    // KBookmarkOwner interface:
    virtual QString currentUrl() const;
    virtual QString currentTitle() const;

signals:
    void openUrl(const KUrl &);

private:
    MainWindow *m_parent;
};

// ------------------------------------------------------------------------------

/*
 *
 */
class BookmarksProvider : public QObject
{
Q_OBJECT
public:
    BookmarksProvider(KMainWindow*);

    void provideBmToolbar(KToolBar*);
    KMenu *bookmarksMenu();

private:
    KMainWindow *m_parent;
    OwnBookMarks *m_owner;
    KBookmarkManager *m_manager;
    KActionCollection *m_ac;
};
#endif
