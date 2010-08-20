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


#ifndef BOOKMARKOWNER_H
#define BOOKMARKOWNER_H


// Rekonq Includes
#include "rekonq_defines.h"

// KDE Includes
#include <KBookmarkOwner>

// Forward Declarations
class KAction;

/**
 * Reimplementation of KBookmarkOwner, this class allows to manage
 * bookmarks as actions.
 */
class REKONQ_TESTS_EXPORT BookmarkOwner : public QObject, public KBookmarkOwner
{
    Q_OBJECT

public:
    explicit BookmarkOwner(KBookmarkManager *manager, QObject *parent = 0);
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
     * @return the current page's title.
     */
    virtual QString currentTitle() const;
    /**
     * @return the current page's URL.
     */
    virtual QString currentUrl() const;

    /**
    * @return whether the owner supports tabs.
    */
    virtual bool supportsTabs() const;

    /**
    * @return list of title, URL pairs of the open tabs.
    */
    virtual QList< QPair<QString, QString> > currentBookmarkList() const;

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
    * Called if the user wants to open every bookmark in this folder in a new tab.
    * The default implementation does nothing.
    * This is only called if supportsTabs() returns true
    */
    virtual void openFolderinTabs(const KBookmarkGroup &bookmark);

signals:
    /**
     * This signal is emitted when an url has to be loaded
     * @param url the URL to load
     */
    void openUrl(const KUrl &, const Rekonq::OpenType &);

public slots:
    void setCurrentBookmark(const KBookmark &bookmark);

    void openBookmark(const KBookmark &bookmark = KBookmark());
    void openBookmarkInNewTab(const KBookmark &bookmark = KBookmark());
    void openBookmarkInNewWindow(const KBookmark &bookmark = KBookmark());
    void openBookmarkFolder(const KBookmark &bookmark = KBookmark());
    void bookmarkCurrentPage(const KBookmark &bookmark = KBookmark());
    void newBookmarkFolder(const KBookmark &bookmark = KBookmark());
    void newSeparator(const KBookmark &bookmark = KBookmark());
    void copyLink(const KBookmark &bookmark = KBookmark());
    void editBookmark(KBookmark bookmark = KBookmark());
    bool deleteBookmark(KBookmark bookmark = KBookmark());

private:
    KBookmarkManager *m_manager;

    QVector<KAction*> m_actions;
    KBookmark m_currentBookmark;

    void setupActions();
    void createAction(const BookmarkAction &action,
                      const QString &text, const QString &icon,
                      const QString &help, const char *slot);
};

#endif // BOOKMARKOWNER_H
