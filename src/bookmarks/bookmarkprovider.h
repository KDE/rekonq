/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2008-2011 by Andrea Diamantini <adjam7 at gmail dot com>
* Copyright (C) 2009 by Paweł Prażak <pawelprazak at gmail dot com>
* Copyright (C) 2009-2010 by Lionel Chauvin <megabigbug@yahoo.fr>
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


#ifndef BOOKMARKPROVIDER_H
#define BOOKMARKPROVIDER_H


// Rekonq Includes
#include "rekonq_defines.h"

// Qt Includes
#include <QObject>

// Forward Declarations
class BookmarksPanel;
class BookmarkToolBar;
class BookmarkOwner;
class BookmarkMenu;

class KAction;
class KActionCollection;
class KActionMenu;
class KBookmark;
class KBookmarkGroup;
class KBookmarkManager;
class KUrl;

class QAction;


/**
 * This class represent the interface to rekonq bookmarks system.
 * All rekonq needs (Bookmarks Menu, Bookmarks Toolbar) is provided
 * from this class.
 * So it implements code to have each one.
 */
class BookmarkProvider : public QObject
{
    Q_OBJECT

public:
    /**
    * @short Class constructor.
    * Connect BookmarksProvider with bookmarks source
    * (actually konqueror's bookmarks).
    * @param parent The MainWindow to provide bookmarks objects.
    */
    BookmarkProvider(QObject *parent = 0);
    ~BookmarkProvider();

    /**
     * @short Get the Bookmarks Menu Action
     * @param the parent widget
     * @return the Bookmarks Menu
     */
    KActionMenu* bookmarkActionMenu(QWidget *parent);

    /**
    * @short set the Bookmarks Toolbar Action
    */
    void registerBookmarkBar(BookmarkToolBar *toolbar);
    void removeBookmarkBar(BookmarkToolBar *toolbar);

    void registerBookmarkPanel(BookmarksPanel *panel);
    void removeBookmarkPanel(BookmarksPanel *panel);

    /**
     * @short Get action by name
     * This method returns poiner bookmark action of given name.
     * @pre m_actionCollection != NULL
     * @param name Name of action you want to get
     * @return It returns actions if one exists or empty object
     */
    QAction* actionByName(const QString &name);

    /**
     * returns Bookmark Manager root group
     *
     * @return the root bookmark group
     */
    KBookmarkGroup rootGroup();

    inline KBookmarkManager* bookmarkManager()
    {
        return m_manager;
    }

    inline BookmarkOwner* bookmarkOwner()
    {
        return m_owner;
    }

    QList<KBookmark> find(const QString &text);

    KBookmark bookmarkForUrl(const KUrl &url);

public Q_SLOTS:
    /**
     * @short Waits for signal that the group with the address has been modified by the caller.
     * Waits for signal that the group (or any of its children) with the address
     * @p groupAddress (e.g. "/4/5") has been modified by the caller @p caller.
     * @param groupAddress bookmark group address
     * @param caller caller that modified the bookmarks
     * @see  KBookmarkManager::changed
     */
    void slotBookmarksChanged();
    void fillBookmarkBar(BookmarkToolBar *toolBar);

private Q_SLOTS:
    void slotPanelChanged();

Q_SIGNALS:
    /**
    * @short This signal is emitted when an url has to be loaded
    */
    void openUrl(const KUrl &, const Rekonq::OpenType &);

private:
    void find(QList<KBookmark> *list, const KBookmark &bookmark, const QString &text);
    KBookmark bookmarkForUrl(const KBookmark &bookmark, const KUrl &url);
    void copyBookmarkGroup(const KBookmarkGroup &groupToCopy, KBookmarkGroup destGroup);

    KBookmarkManager *m_manager;
    BookmarkOwner *m_owner;
    KActionCollection *m_actionCollection;
    QList<BookmarkToolBar *> m_bookmarkToolBars;
    QList<BookmarksPanel *> m_bookmarkPanels;
};


#endif // BOOKMARKPROVIDER_H
