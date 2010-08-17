/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2008-2010 by Andrea Diamantini <adjam7 at gmail dot com>
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


#ifndef BOOKMARKS_H
#define BOOKMARKS_H


// Rekonq Includes
#include "rekonq_defines.h"

// Local Includes
#include "application.h"
#include "urlresolver.h"
#include "bookmarkspanel.h"
#include "bookmarkstoolbar.h"

// Qt Includes
#include <QWidget>

// KDE Includes
#include <KBookmarkOwner>

// Forward Declarations
class BookmarkProvider;

class KAction;
class KActionCollection;
class KActionMenu;
class KUrl;
class KToolBar;
class KBookmarkManager;


/**
 * Reimplementation of KBookmarkOwner, this class allows to manage
 * bookmarks as actions
 *
 */
class REKONQ_TESTS_EXPORT BookmarkOwner : public QObject , public KBookmarkOwner
{
    Q_OBJECT

public:

    /**
     * @short The class constructor.
     *
     * @param parent the pointer parent Bookmark provider. We need it
     *               to get pointer to MainWindow
     */
    BookmarkOwner(QObject *parent = 0);
    virtual ~BookmarkOwner() {}

    enum BookmarkAction
    {
        OPEN = 0,
        OPEN_IN_TAB,
        OPEN_IN_WINDOW,
        OPEN_FOLDER,
        BOOKMARK_PAGE,
        NEW_FOLDER,
        NEW_SEPARATOR,
        COPY,
        EDIT,
        DELETE,
        NUM_ACTIONS
    };

    /**
     * @return the action or 0 if it doesn't exist.
     */
    KAction* action(const BookmarkAction &bmAction);

    /**
     * This function is called when a bookmark is selected and belongs to
     * the ancestor class.
     * This method actually emits signal to load bookmark's url.
     *
     * @param bookmark          the bookmark to open
     * @param mouseButtons      the mouse buttons clicked to select the bookmark
     * @param keyboardModifiers the keyboard modifiers pushed when the bookmark was selected
     */
    virtual void openBookmark(const KBookmark &bookmark,
                              Qt::MouseButtons mouseButtons,
                              Qt::KeyboardModifiers keyboardModifiers);


    /**
     * Bookmarks the current page.
     * @param position Where to insert the bookmark.
     */
    static void bookmarkPage(KBookmark &position);
    /**
     * Creates a new bookmark folder.
     * @param position Where to insert the folder.
     */
    static void newBookmarkFolder(KBookmark &position);
    /**
     * Creates a new separator.
     * @param position Where to insert the separator.
     */
    static void newSeparator(KBookmark &position);

    /**
     * Opens the edit dialog for a bookmark.
     * @param bookmark The bookmark to edit.
     */
    static void editBookmark(KBookmark &bookmark);
    /**
     * Promps the user to delete a bookmark.
     * @param bookmark The bookmark to delete.
     * @return true if the bookmark was deleted, false if canceled.
     */
    static bool deleteBookmark(KBookmark &bookmark);


    /**
     * this method, from KBookmarkOwner interface, allows to add the current page
     * to the bookmark list, returning the URL page as QString.
     *
     * @return the current page's URL
     */
    virtual QString currentUrl() const;

    /**
     * this method, from KBookmarkOwner interface, allows to add the current page
     * to the bookmark list, returning the title's page as QString.
     *
     * @return the current page's title
     */
    virtual QString currentTitle() const;

    /**
    * This function returns whether the owner supports tabs.
    */
    virtual bool supportsTabs() const;

    /**
    * Called if the user wants to open every bookmark in this folder in a new tab.
    * The default implementation does nothing.
    * This is only called if supportsTabs() returns true
    */
    virtual void openFolderinTabs(const KBookmarkGroup &bookmark);

    virtual QList< QPair<QString, QString> > currentBookmarkList() const;

signals:
    /**
     * This signal is emitted when an url has to be loaded
     *
     * @param url the URL to load
     *
     */
    void openUrl(const KUrl &, const Rekonq::OpenType &);

private:
    QVector<KAction*> actions;

    void setupActions();
    void createAction(const BookmarkAction &action,
                      const QString &text, const QString &icon,
                      const QString &help, const char *slot);
};


// ------------------------------------------------------------------------------


/**
 * This class represent the interface to rekonq bookmarks system.
 * All rekonq needs (Bookmarks Menu, Bookmarks Toolbar) is provided
 * from this class.
 * So it implements code to have each one
 *
 *
 */
class BookmarkProvider : public QObject
{
    Q_OBJECT

public:
    /**
    * @short Class constructor.
    * Connect BookmarksProvider with bookmarks source
    * (actually konqueror's bookmarks)
    * @param parent The MainWindow to provide bookmarks objects
    *
    */
    BookmarkProvider(QObject* parent = 0);
    ~BookmarkProvider();

    /**
     * @short Get the Bookmarks Menu Action
     * @param the parent widget
     * @return the Bookmarks Menu
     */
    KActionMenu *bookmarkActionMenu(QWidget *parent);

    KAction *bookmarkToolBarAction(KToolBar *t);

    /**
    * @short set the Bookmarks Toolbar Action
    */
    void setupBookmarkBar(BookmarkToolBar *);

    void removeToolBar(BookmarkToolBar *);

    /**
     * @short Get action by name
     * This method returns poiner bookmark action of given name.
     * @pre m_actionCollection != NULL
     * @param name Name of action you want to get
     * @return It returns actions if one exists or empty object
     */
    QAction *actionByName(const QString &name);

    /**
     * returns Bookmark Manager root group
     *
     * @return the root bookmark group
     */
    KBookmarkGroup rootGroup();

    inline KBookmarkManager *bookmarkManager() { return m_manager; }

    inline BookmarkOwner *bookmarkOwner() { return m_owner; }

    QList<KBookmark> find(QString text);

    void registerBookmarkPanel(BookmarksPanel *panel);
    void removeBookmarkPanel(BookmarksPanel *panel);

    KBookmark bookmarkForUrl(const KUrl &url);

signals:
    /**
    * @short This signal is emitted when an url has to be loaded
    *
    * @param url the URL to load
    */
    void openUrl(const KUrl &, const Rekonq::OpenType &);


public slots:
    /**
     * @short Opens the context menu on given position
     * @param point Point on which you want to open this menu
     */
    void contextMenu(const QPoint &point);

    /**
     * @short Waits for signal that the group with the address has been modified by the caller.
     * Waits for signal that the group (or any of its children) with the address
     * @p groupAddress (e.g. "/4/5") has been modified by the caller @p caller.
     * @param group bookmark group address
     * @param caller caller that modified the bookmarks
     * @see  KBookmarkManager::changed
     */
    void slotBookmarksChanged(const QString &group, const QString &caller);
    void fillBookmarkBar(BookmarkToolBar *toolBar);

private slots:
    void slotAddBookmark();
    void slotPanelChanged();

private:
    QList<KBookmark> find(QList<KBookmark> list, const KBookmark &bookmark, QString text);

    QString titleForBookmarkUrl(const KBookmark &bookmark, const QString &url);
    KBookmark bookmarkForUrl(const KBookmark &bookmark, const KUrl &url);

    KBookmarkManager *m_manager;
    BookmarkOwner *m_owner;
    KActionCollection *m_actionCollection;
    QList<BookmarkToolBar *> m_bookmarkToolBars;
    QList<BookmarksPanel*> m_bookmarkPanels;

    KActionMenu *_bookmarkActionMenu;
};


#endif
