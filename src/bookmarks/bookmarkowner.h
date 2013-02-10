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


#ifndef BOOKMARKOWNER_H
#define BOOKMARKOWNER_H


// Rekonq Includes
#include "rekonq_defines.h"

// KDE Includes
#include <KAction>
#include <KBookmarkOwner>


/**
 * This class allows to manage bookmarks as actions.
 */
class REKONQ_TESTS_EXPORT BookmarkOwner : public QObject, public KBookmarkOwner
{
    Q_OBJECT

public:
    explicit BookmarkOwner(KBookmarkManager *manager, QObject *parent = 0);

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
#ifdef HAVE_NEPOMUK
        FANCYBOOKMARK,
#endif
        DELETE,
        NUM_ACTIONS,
        SET_TOOLBAR_FOLDER,
        UNSET_TOOLBAR_FOLDER
    };

    /**
     * @return A new action for the given bookmark.
     */
    KAction* createAction(const KBookmark &bookmark, const BookmarkAction &bmAction);

    // @{
    /**
     * Functions to get current information.
     */
    virtual QString currentTitle() const;
    virtual QString currentUrl() const;
    virtual QList< QPair<QString, QString> > currentBookmarkList() const;
    // @}

    virtual bool supportsTabs() const
    {
        return true;
    }

    // @{
    /**
     * This functions emit signals that open the selected URLs
     */
    virtual void openBookmark(const KBookmark &bookmark,
                              Qt::MouseButtons mouseButtons,
                              Qt::KeyboardModifiers keyboardModifiers);
    virtual void openFolderinTabs(const KBookmarkGroup &bkGoup);
    // @}

public Q_SLOTS:
    void loadBookmark(const KBookmark &bookmark);
    void loadBookmarkInNewTab(const KBookmark &bookmark);
    void loadBookmarkInNewWindow(const KBookmark &bookmark);
    void loadBookmarkFolder(const KBookmark &bookmark);

    KBookmark bookmarkCurrentPage(const KBookmark &bookmark = KBookmark());
    KBookmarkGroup newBookmarkFolder(const KBookmark &bookmark = KBookmark(), const QString &name = QString());
    KBookmark newSeparator(const KBookmark &bookmark = KBookmark());

    void copyLink(const KBookmark &bookmark);
    void editBookmark(KBookmark bookmark);
#ifdef HAVE_NEPOMUK
    void fancyBookmark(KBookmark bookmark);
#endif
    bool deleteBookmark(const KBookmark &bookmark);
    void setToolBarFolder(KBookmark bookmark = KBookmark());
    void unsetToolBarFolder();

Q_SIGNALS:
    /**
     * This signal is emitted when an url has to be loaded
     * @param url the URL to load
     */
    void openUrl(const KUrl &, const Rekonq::OpenType &);

private:
    KAction* createAction(const QString &text, const QString &icon,
                          const QString &help, const char *slot,
                          const KBookmark &bookmark);

    KBookmarkManager *m_manager;
};


// -----------------------------------------------------------------------------------------------


class CustomBookmarkAction : public KAction
{
    Q_OBJECT

public:
    CustomBookmarkAction(const KBookmark &bookmark, const KIcon &icon, const QString &text, QObject *parent);

Q_SIGNALS:
    void triggered(const KBookmark &);

private Q_SLOTS:
    void onActionTriggered();

private:
    KBookmark m_bookmark;
};

#endif // BOOKMARKOWNER_H
