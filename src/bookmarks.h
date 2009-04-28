/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2008 by Andrea Diamantini <adjam7 at gmail dot com>
* Copyright (C) 2009 by Paweł Prażak <pawelprazak at gmail dot com>
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



#ifndef BOOKMARKS_H
#define BOOKMARKS_H

// Local includes
#include "application.h"

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
class BookmarkOwner : public QObject , public KBookmarkOwner
{
    Q_OBJECT

public:

    /**
     * @short The class constructor.
     *
     * @param parent the pointer parent Bookmark provider. We need it
     *               to get pointer to MainWindow
     */
    BookmarkOwner(QObject *parent=0);
    virtual ~BookmarkOwner() {}

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
    virtual bool supportsTabs() const { return true; }

    /**
    * Called if the user wants to open every bookmark in this folder in a new tab.
    * The default implementation does nothing.
    * This is only called if supportsTabs() returns true
    */
    virtual void openFolderinTabs(const KBookmarkGroup &bm);

signals:
    /**
     * This signal is emitted when an url has to be loaded
     *
     * @param url the URL to load
     * @param type type of load
     * @see Application::OpenType
     *
     */
    void openUrl(const KUrl &url, Rekonq::OpenType type);

private:

};

// ------------------------------------------------------------------------------


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
    ~BookmarkMenu();

    virtual KMenu *viewContextMenu(QAction* action);

protected slots:
    void slotAddBookmark();

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
class BookmarkProvider : public QWidget
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
    BookmarkProvider(QWidget* parent=0);
    ~BookmarkProvider();

    /**
     * @short Get the Bookmarks Menu Action
     * @return the Bookmarks Menu
     */
    KActionMenu *bookmarkActionMenu();


    /**
    * @short Get the Bookmarks Toolbar Action
    * @return the Bookmarks Toolbar Action
    */
    KAction *bookmarkToolBarAction();


    /**
     * @short Get action by name
     * This method returns poiner bookmark action of given name.
     * @pre m_actionCollection != NULL
     * @param name Name of action you want to get
     * @return It returns actions if one exists or empty object
     */
    QAction *actionByName(const QString &name);

signals:
    /**
     * This signal is emitted when an url has to be loaded
     *
     * @param url the URL to load
     * @param type type of load
     * @see Application::OpenType
     *
     */
    void openUrl(const KUrl &url, Rekonq::OpenType type);


public slots:
    /**
     * @short Opens the context menu on given position
     * @param point Point on whitch you want to open this menu
     */
    void contextMenu(const QPoint &point);

    /**
     * @short Waits for signal that the group with the address has been modified by the caller.
     * Waits for signal that the group (or any of its children) with the address
     * @p groupAddress (e.g. "/4/5") has been modified by the caller @p caller.
     *
     * @param group bookmark group adress
     * @param caller caller that modified the bookmarks
     * @see  KBookmarkManager::changed
     */
    void slotBookmarksChanged(const QString &group, const QString &caller);

private:
    void setupToolBar();

    KBookmarkManager *m_manager;
    BookmarkOwner *m_owner;
    KMenu *m_menu;
    KActionCollection *m_actionCollection;
    BookmarkMenu *m_bookmarkMenu;
    KToolBar *m_bookmarkToolBar;
};

#endif
