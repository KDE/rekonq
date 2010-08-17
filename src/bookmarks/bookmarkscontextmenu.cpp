/* ============================================================
*
* This file is a part of the rekonq project
*
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
#include "bookmarkscontextmenu.h"

// Local Includes
#include "bookmarksmanager.h"

// KDE Includes
#include <KActionCollection>

// Qt Includes
#include <QClipboard>


BookmarksContextMenu::BookmarksContextMenu(const KBookmark & bookmark, KBookmarkManager *manager, KBookmarkOwner *owner, QWidget *parent)
        : KBookmarkContextMenu(bookmark, manager, owner, parent)
        , m_ac(new KActionCollection(this))
{
    setupActions();
}


BookmarksContextMenu::~BookmarksContextMenu()
{
    delete m_ac;
}


void BookmarksContextMenu::setupActions()
{
    KAction* action;

    action = new KAction(KIcon("tab-new"), i18n("Open"), this);
    connect(action, SIGNAL(triggered()), this, SLOT(openInCurrentTab()));
    m_ac->addAction("open", action);

    action = new KAction(KIcon("tab-new"), i18n("Open in New Tab"), this);
    connect(action, SIGNAL(triggered()), this, SLOT(openInNewTab()));
    m_ac->addAction("open_tab", action);

    action = new KAction(KIcon("window-new"), i18n("Open in New Window"), this);
    connect(action, SIGNAL(triggered()), this, SLOT(openInNewWindow()));
    m_ac->addAction("open_window", action);

    action = new KAction(KIcon("bookmark-new"), i18n("Add Bookmark Here"), this);
    connect(action, SIGNAL(triggered()), this, SLOT(bookmarkCurrentPage()));
    m_ac->addAction("bookmark_page", action);

    action = new KAction(KIcon("folder-new"), i18n("New Bookmark Folder"), this);
    connect(action, SIGNAL(triggered()), this, SLOT(newBookmarkGroup()));
    m_ac->addAction("folder_new", action);

    action = new KAction(KIcon("edit-clear"), i18n("New Separator"), this);
    connect(action, SIGNAL(triggered()), this, SLOT(newSeparator()));
    m_ac->addAction("separator_new", action);

    action = new KAction(KIcon("edit-copy"), i18n("Copy Link Address"), this);
    connect(action, SIGNAL(triggered()), this, SLOT(copyToClipboard()));
    m_ac->addAction("copy", action);

    action = new KAction(KIcon("edit-delete"), i18n("Delete Bookmark"), this);
    connect(action, SIGNAL(triggered()), this, SLOT(deleteBookmark()));
    m_ac->addAction("delete", action);

    action = new KAction(KIcon("configure"), i18n("Properties"), this);
    connect(action, SIGNAL(triggered()), this, SLOT(editBookmark()));
    m_ac->addAction("properties", action);

    action = new KAction(KIcon("tab-new"), i18n("Open Folder in Tabs"), this);
    connect(action, SIGNAL(triggered()), this, SLOT(openFolderInTabs()));
    m_ac->addAction("open_all", action);
}


void BookmarksContextMenu::addBookmarkActions()
{
    addAction(m_ac->action("open"));
    addAction(m_ac->action("open_tab"));
    addAction(m_ac->action("open_window"));

    addSeparator();

    addAction(m_ac->action("bookmark_page"));
    addAction(m_ac->action("folder_new"));
    addAction(m_ac->action("separator_new"));

    addSeparator();

    addAction(m_ac->action("copy"));

    addSeparator();

    addAction(m_ac->action("delete"));
    addAction(m_ac->action("properties"));
}


void BookmarksContextMenu::addFolderActions()
{
    if (!bookmark().toGroup().first().isNull())
    {
        addAction(m_ac->action("open_all"));
        addSeparator();
    }

    addAction(m_ac->action("bookmark_page"));
    addAction(m_ac->action("folder_new"));
    addAction(m_ac->action("separator_new"));

    addSeparator();

    addAction(m_ac->action("delete"));
    addAction(m_ac->action("properties"));
}


void BookmarksContextMenu::addSeparatorActions()
{
    addAction(m_ac->action("bookmark_page"));
    addAction(m_ac->action("folder_new"));
    addAction(m_ac->action("separator_new"));

    addSeparator();

    addAction(m_ac->action("delete"));
}


void BookmarksContextMenu::addActions()
{
    if (bookmark().isGroup())
    {
        addFolderActions();
    }

    else if (bookmark().isSeparator())
    {
        addSeparatorActions();
    }

    else if (bookmark().isNull())
    {
        addAction(m_ac->action("bookmark_page"));
        addAction(m_ac->action("folder_new"));
        addAction(m_ac->action("separator_new"));
    }

    else
    {
        addBookmarkActions();
    }
}


void BookmarksContextMenu::openInCurrentTab()
{
    Application::instance()->loadUrl(bookmark().url());
}


void BookmarksContextMenu::openInNewTab()
{
    Application::instance()->loadUrl(bookmark().url(), Rekonq::NewTab);
}


void BookmarksContextMenu::openInNewWindow()
{
    Application::instance()->loadUrl(bookmark().url(), Rekonq::NewWindow);
}


void BookmarksContextMenu::copyToClipboard()
{
    if (bookmark().isNull())
        return;

    QClipboard *cb = QApplication::clipboard();
    cb->setText(bookmark().url().url());
}


void BookmarksContextMenu::deleteBookmark()
{
    KBookmark bm = bookmark();
    Application::bookmarkProvider()->bookmarkOwner()->deleteBookmark(bm);
}


void BookmarksContextMenu::editBookmark()
{
    KBookmark bm = bookmark();
    Application::bookmarkProvider()->bookmarkOwner()->editBookmark(bm);
}


void BookmarksContextMenu::openFolderInTabs()
{
    if (bookmark().isGroup())
        owner()->openFolderinTabs(bookmark().toGroup());
}


void BookmarksContextMenu::newBookmarkGroup()
{
    KBookmark bm = bookmark();
    Application::bookmarkProvider()->bookmarkOwner()->newBookmarkFolder(bm);
}


void BookmarksContextMenu::newSeparator()
{
    KBookmark bm = bookmark();
    Application::bookmarkProvider()->bookmarkOwner()->newSeparator(bm);
}


void BookmarksContextMenu::bookmarkCurrentPage()
{
    KBookmark bm = bookmark();
    Application::bookmarkProvider()->bookmarkOwner()->bookmarkPage(bm);
}

