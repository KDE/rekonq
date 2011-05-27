/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2008-2011 by Andrea Diamantini <adjam7 at gmail dot com>
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

#ifndef BOOKMARKSTOOLBAR_H
#define BOOKMARKSTOOLBAR_H


// Rekonq Includes
#include "rekonq_defines.h"

// KDE Includes
#include <KBookmarkMenu>

/**
 * This class represent the rekonq bookmarks menu.
 * It's just a simple class inherited from KBookmarkMenu
 *
 */
class BookmarkMenu : public KBookmarkMenu
{
    Q_OBJECT

public:
    BookmarkMenu(KBookmarkManager* manager,
                 KBookmarkOwner* owner,
                 KMenu* menu,
                 KActionCollection* actionCollection);
    BookmarkMenu(KBookmarkManager  *manager,
                 KBookmarkOwner  *owner,
                 KMenu  *parentMenu,
                 const QString &parentAddress);
    ~BookmarkMenu();

protected:
    virtual KMenu * contextMenu(QAction * act);
    virtual void refill();
    virtual QAction* actionForBookmark(const KBookmark &bookmark);

private slots:
    void actionHovered();

private:
    void addOpenFolderInTabs();

};


// ------------------------------------------------------------------------------


// KDE Includes
#include <KToolBar>


/**
 * This class manage the bookmark toolbar.
 * Some events from the toolbar are handled to allow the drag and drop
 */

class BookmarkToolBar : public QObject
{
    Q_OBJECT

public:
    BookmarkToolBar(KToolBar *toolBar, QObject *parent);

    KToolBar* toolBar();

protected:
    bool eventFilter(QObject *watched, QEvent *event);

private slots:
    void contextMenu(const QPoint &);
    void actionHovered();
    void menuDisplayed();
    void menuHidden();
    void hideMenu();
    void dragDestroyed();

private:
    void startDrag();

    KToolBar *m_toolBar;
    KMenu *m_currentMenu;
    QPoint m_startDragPos;
    QAction *m_dragAction;
    QAction *m_dropAction;
    QAction *m_checkedAction;
    bool m_filled;
};

#endif // BOOKMARKSTOOLBAR_H
