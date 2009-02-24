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

/** 
 * Inherited from KBookmarkOwner, this class allows to manage 
 * bookmarks as actions
 *
 * @author Andrea Diamantini <adjam7@gmail.com>
 * @since 4.x
 */
class OwnBookMarks : public QObject , public KBookmarkOwner
{
Q_OBJECT

public:

    /**  
     * The class ctor.
     *
     * @param parent the pointer to the browser mainwindow. We need it
     *               to link bookmarks actions with the right window 
     *               where load url in
     */
    OwnBookMarks(KMainWindow *parent);

    /**
     * This function is called when a bookmark is selected and belongs to 
     * the ancestor class.
     * This method actually emits signal to load bookmark's url without
     * considering mousebuttons or keyboard modifiers.
     *
     * @param b  the bookmark to open
     * @param mb the mouse buttons clicked to select the bookmark
     * @param km the keyboard modifiers pushed when the bookmark was selected
     */
    virtual void openBookmark (const KBookmark &b , Qt::MouseButtons mb, Qt::KeyboardModifiers km);

    
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

signals:
    /**
     * This signal is emitted when an url has to be loaded 
     *
     * @param url the URL to load
     *
     */
    void openUrl(const KUrl &);

private:
    // the MainWindow pointer
    MainWindow *m_parent;
};


// ------------------------------------------------------------------------------


/** 
 * This class represent the interface to rekonq bookmarks system.
 * All rekonq needs (Bookmarks Menu, Bookmarks Toolbar) is provided
 * from this class.
 * So it implements code to have each one
 *
 * @author Andrea Diamantini <adjam7@gmail.com>
 * @since 4.x
 *
 */
class BookmarksProvider : public QObject
{
Q_OBJECT

public:
    /** 
     * Class constructor. Connect BookmarksProvider with bookmarks source
     * (actually konqueror's bookmarks)
     *
     * @param parent The MainWindow to provide bookmarks objects
     *
     */
    BookmarksProvider(KMainWindow* parent);

    /** 
     * Customize bookmarks toolbar
     *
     * @param toolbar the toolbar to customize
     */
    void provideBmToolbar(KToolBar* toolbar);

    /** 
     * Generate the Bookmarks Menu
     *
     * @return the Bookmarks Menu
     *
     */
    KMenu *bookmarksMenu();

private:
    KMainWindow *m_parent;
    OwnBookMarks *m_owner;
    KBookmarkManager *m_manager;
    KActionCollection *m_ac;
};
#endif
