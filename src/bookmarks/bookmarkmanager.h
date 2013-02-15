/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2008-2012 by Andrea Diamantini <adjam7 at gmail dot com>
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


#ifndef BOOKMARK_MANAGER_H
#define BOOKMARK_MANAGER_H


// Rekonq Includes
#include "rekonq_defines.h"

// KDE Includes
#include <KBookmark>
#include <KActionMenu>

// Qt Includes
#include <QObject>
#include <QWeakPointer>

// Forward Declarations
class BookmarkToolBar;
class BookmarkOwner;

class KAction;
class KActionCollection;
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
class BookmarkManager : public QObject
{
    Q_OBJECT

public:
    /**
     * Entry point.
     * Access to BookmarkManager class by using
     * BookmarkManager::self()->thePublicMethodYouNeed()
     */
    static BookmarkManager *self();

    ~BookmarkManager();

    /**
    * @short set the Bookmarks Toolbar Action
    */
    void registerBookmarkBar(BookmarkToolBar *toolbar);
    void removeBookmarkBar(BookmarkToolBar *toolbar);

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

    inline KBookmarkManager* manager()
    {
        return m_manager;
    }

    inline BookmarkOwner* owner()
    {
        return m_owner;
    }

    QList<KBookmark> find(const QString &text);

    KBookmark bookmarkForUrl(const KUrl &url);

    KBookmark findByAddress(const QString &);

    void openFolderinTabs(const KBookmarkGroup &bm);

    void emitChanged();

    static inline QString bookmark_mime_type()
    {
        return QL1S("application/x-rekonq-bookmark");
    }
    
    KActionMenu* bookmarkActionMenu(QWidget *parent);

private:
    /**
    * @short Class constructor.
    * Connect BookmarksProvider with bookmarks source
    * (actually konqueror's bookmarks).
    * @param parent The WebWindow to provide bookmarks objects.
    */
    explicit BookmarkManager(QObject *parent = 0);

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

    void slotEditBookmarks();

    KBookmark bookmarkCurrentPage(const KBookmark &bookmark = KBookmark());

Q_SIGNALS:
    /**
    * @short This signal is emitted when an url has to be loaded
    */
    void openUrl(const KUrl &, const Rekonq::OpenType &);

    void bookmarksUpdated();

private:
    void find(QList<KBookmark> *list, const KBookmark &bookmark, const QString &text);
    KBookmark bookmarkForUrl(const KBookmark &bookmark, const KUrl &url);
    void copyBookmarkGroup(const KBookmarkGroup &groupToCopy, KBookmarkGroup destGroup);

    KBookmarkManager *m_manager;
    BookmarkOwner *m_owner;
    KActionCollection *m_actionCollection;
    QList<BookmarkToolBar *> m_bookmarkToolBars;

    static QWeakPointer<BookmarkManager> s_bookmarkManager;
};


#endif // BOOKMARK_MANAGER_H
