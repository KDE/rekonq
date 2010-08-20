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


// Self Includes
#include "bookmarkprovider.h"

// Local Includes
#include "application.h"
#include "bookmarkspanel.h"
#include "bookmarkscontextmenu.h"
#include "bookmarkstoolbar.h"
#include "bookmarkowner.h"

// KDE Includes
#include <KActionCollection>
#include <KStandardDirs>

// Qt Includes
#include <QtCore/QFile>


BookmarkProvider::BookmarkProvider(QObject *parent)
        : QObject(parent)
        , m_manager(0)
        , m_owner(0)
        , m_actionCollection(new KActionCollection(this))
{
    kDebug() << "Loading Bookmarks Manager...";

    QString bookmarkFile = KStandardDirs::locate("data", "konqueror/bookmarks.xml");  // share konqueror's bookmarks

    if (!QFile::exists(bookmarkFile))
    {
        bookmarkFile = KStandardDirs::locateLocal("appdata", "bookmarks.xml", true);
        QFile bkms( KStandardDirs::locate("appdata" , "defaultbookmarks.xbel") );
        bkms.copy(bookmarkFile);
    }

    m_manager = KBookmarkManager::managerForFile(bookmarkFile, "rekonq");

    connect(m_manager, SIGNAL(changed(const QString &, const QString &)),
            this, SLOT(slotBookmarksChanged(const QString &, const QString &)));

    // setup menu
    m_owner = new BookmarkOwner(m_manager, this);
    connect(m_owner, SIGNAL(openUrl(const KUrl&, const Rekonq::OpenType&)),
            this, SIGNAL(openUrl(const KUrl&, const Rekonq::OpenType&)));

    KAction *a = KStandardAction::addBookmark(this, SLOT(slotAddBookmark()), this);
    m_actionCollection->addAction(QL1S("rekonq_add_bookmark"), a);

    kDebug() << "Loading Bookmarks Manager... DONE!";
}


BookmarkProvider::~BookmarkProvider()
{
    delete m_manager;
}


void BookmarkProvider::registerBookmarkBar(BookmarkToolBar *toolbar)
{
    if (m_bookmarkToolBars.contains(toolbar))
        return;

    kDebug() << "new bookmark bar...";

    m_bookmarkToolBars.append(toolbar);
    toolbar->toolBar()->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(toolbar->toolBar(), SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(contextMenu(const QPoint&)));

    kDebug() << "new bookmark bar... DONE!";
}


void BookmarkProvider::removeToolBar(BookmarkToolBar *toolbar)
{
    m_bookmarkToolBars.removeOne(toolbar);
}


void BookmarkProvider::slotBookmarksChanged(const QString& /*groupAddress*/, const QString& /*caller*/)
{
    foreach(BookmarkToolBar *bookmarkToolBar, m_bookmarkToolBars)
    {
        if (bookmarkToolBar)
        {
            bookmarkToolBar->toolBar()->clear();
            fillBookmarkBar(bookmarkToolBar);
        }
    }
}


QAction* BookmarkProvider::actionByName(const QString &name)
{
    QAction *action = m_actionCollection->action(name);
    if (action)
        return action;
    return new QAction(this);
}


void BookmarkProvider::contextMenu(const QPoint &point)
{
    if (m_bookmarkToolBars.isEmpty())
        return;

    KToolBar *bookmarkToolBar = m_bookmarkToolBars.at(0)->toolBar();
    if (!bookmarkToolBar)
        return;

    KBookmarkActionInterface *action = dynamic_cast<KBookmarkActionInterface*>(bookmarkToolBar->actionAt(point));
    if (!action)
        return;

    BookmarksContextMenu menu(action->bookmark(), bookmarkManager(), bookmarkOwner());
    menu.exec(bookmarkToolBar->mapToGlobal(point));
}


KActionMenu* BookmarkProvider::bookmarkActionMenu(QWidget *parent)
{
    kDebug() << "new Bookmarks menu...";

    KMenu *menu = new KMenu(parent);
    KActionMenu *bookmarkActionMenu = new KActionMenu(menu);
    bookmarkActionMenu->setMenu(menu);
    bookmarkActionMenu->setText(i18n("&Bookmarks"));
    new BookmarkMenu(m_manager, m_owner, menu, m_actionCollection);

    kDebug() << "new Bookmarks menu... DONE";

    return bookmarkActionMenu;
}


void BookmarkProvider::fillBookmarkBar(BookmarkToolBar *toolBar)
{
    KBookmarkGroup root = m_manager->toolbar();
    if (root.isNull())
        return;

    for (KBookmark bookmark = root.first(); !bookmark.isNull(); bookmark = root.next(bookmark))
    {
        if (bookmark.isGroup())
        {
            KBookmarkActionMenu *menuAction = new KBookmarkActionMenu(bookmark.toGroup(), this);
            menuAction->setDelayed(false);
            new BookmarkMenu(bookmarkManager(), bookmarkOwner(), menuAction->menu(), bookmark.address());

            connect(menuAction->menu(), SIGNAL(aboutToShow()), toolBar, SLOT(menuDisplayed()));
            connect(menuAction->menu(), SIGNAL(aboutToHide()), toolBar, SLOT(menuHidden()));

            toolBar->toolBar()->addAction(menuAction);
        }
        else if (bookmark.isSeparator())
        {
            toolBar->toolBar()->addSeparator();
        }
        else
        {
            KBookmarkAction *action = new KBookmarkAction(bookmark, m_owner, this);
            action->setIconText(action->iconText().replace('&', "&&"));
            connect(action, SIGNAL(hovered()), toolBar, SLOT(actionHovered()));
            toolBar->toolBar()->addAction(action);
            toolBar->toolBar()->widgetForAction(action)->installEventFilter(toolBar);
        }
    }
}


KBookmarkGroup BookmarkProvider::rootGroup()
{
    return m_manager->root();
}


QList<KBookmark> BookmarkProvider::find(const QString &text)
{
    QList<KBookmark> list;

    KBookmarkGroup root = Application::bookmarkProvider()->rootGroup();
    if (!root.isNull())
        for (KBookmark bookmark = root.first(); !bookmark.isNull(); bookmark = root.next(bookmark))
            find(&list, bookmark, text);

    return list;
}


void BookmarkProvider::find(QList<KBookmark> *list, const KBookmark &bookmark, const QString &text)
{
    if (bookmark.isGroup())
    {
        KBookmarkGroup group = bookmark.toGroup();
        for (KBookmark bm = group.first(); !bm.isNull(); bm = group.next(bm))
            find(list, bm, text);
    }
    else if (bookmark.url().url().contains(text) || bookmark.fullText().contains(text))
    {
        *list << bookmark;
    }
}


void BookmarkProvider::slotAddBookmark()
{
    rootGroup().addBookmark(bookmarkOwner()->currentTitle(), bookmarkOwner()->currentUrl());
    bookmarkManager()->emitChanged();
}


void BookmarkProvider::registerBookmarkPanel(BookmarksPanel *panel)
{
    if (panel && !m_bookmarkPanels.contains(panel))
    {
        m_bookmarkPanels.append(panel);
        connect(panel, SIGNAL(expansionChanged()), this, SLOT(slotPanelChanged()));
    }
}


void BookmarkProvider::removeBookmarkPanel(BookmarksPanel *panel)
{
    if (!panel)
        return;

    m_bookmarkPanels.removeOne(panel);
    panel->disconnect(this);

    if (m_bookmarkPanels.isEmpty())
        Application::bookmarkProvider()->bookmarkManager()->emitChanged();
}


void BookmarkProvider::slotPanelChanged()
{
    foreach (BookmarksPanel *panel, m_bookmarkPanels)
    {
        if (panel && panel != sender())
            panel->startLoadFoldedState();
    }
}


KBookmark BookmarkProvider::bookmarkForUrl(const KUrl &url)
{
    KBookmarkGroup root = rootGroup();
    if (root.isNull())
        return KBookmark();

    return bookmarkForUrl(root, url);
}


KBookmark BookmarkProvider::bookmarkForUrl(const KBookmark &bookmark, const KUrl &url)
{
    KBookmark found;

    if (bookmark.isGroup())
    {
        KBookmarkGroup group = bookmark.toGroup();
        KBookmark bookmark = group.first();

        while (!bookmark.isNull() && found.isNull())
        {
            found = bookmarkForUrl(bookmark, url);
            bookmark = group.next(bookmark);
        }
    }
    else if (!bookmark.isSeparator() && bookmark.url() == url)
    {
        found = bookmark;
    }

    return found;
}
