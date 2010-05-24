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
#include "bookmarkcontextmenu.h"
#include "bookmarkcontextmenu.moc"

// Local Includes
#include "application.h"
#include "bookmarksmanager.h"

// KDE Includes
#include <KMessageBox>
#include <KActionCollection>
#include <KBookmarkDialog>

// Qt Includes
#include <QClipboard>


BookmarkContextMenu::BookmarkContextMenu(const KBookmark & bookmark, KBookmarkManager *manager, KBookmarkOwner *owner, QWidget *parent)
        : KBookmarkContextMenu(bookmark, manager, owner, parent)
        , m_ac(new KActionCollection(this))
{
    setupActions();
}


BookmarkContextMenu::~BookmarkContextMenu()
{
    delete m_ac;
}


void BookmarkContextMenu::setupActions()
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


void BookmarkContextMenu::addBookmarkActions()
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


void BookmarkContextMenu::addFolderActions()
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


void BookmarkContextMenu::addSeparatorActions()
{
    addAction(m_ac->action("bookmark_page"));
    addAction(m_ac->action("folder_new"));
    addAction(m_ac->action("separator_new"));

    addSeparator();

    addAction(m_ac->action("delete"));
}


void BookmarkContextMenu::addActions()
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


void BookmarkContextMenu::openInCurrentTab()
{
    Application::instance()->loadUrl(bookmark().url());
}


void BookmarkContextMenu::openInNewTab()
{
    Application::instance()->loadUrl(bookmark().url(), Rekonq::SettingOpenTab);
}


void BookmarkContextMenu::openInNewWindow()
{
    Application::instance()->loadUrl(bookmark().url(), Rekonq::NewWindow);
}


void BookmarkContextMenu::copyToClipboard()
{
    if (bookmark().isNull())
        return;

    QClipboard *cb = QApplication::clipboard();
    cb->setText(bookmark().url().url());
}


void BookmarkContextMenu::deleteBookmark()
{
    KBookmark bm = bookmark();
    bool folder = bm.isGroup();
    QString name = QString(bm.text()).replace("&&", "&");

    if (KMessageBox::warningContinueCancel(
                QApplication::activeWindow(),
                folder ? i18n("Are you sure you wish to remove the bookmark folder\n\"%1\"?", name)
                : i18n("Are you sure you wish to remove the bookmark\n\"%1\"?", name),
                folder ? i18n("Bookmark Folder Deletion")
                : i18n("Bookmark Deletion"),
                KStandardGuiItem::del())
            != KMessageBox::Continue
       )
        return;

    bm.parentGroup().deleteBookmark(bm);
    manager()->emitChanged();
}


void BookmarkContextMenu::editBookmark()
{
    KBookmark selected = bookmark();
    selected.setFullText(selected.text().replace("&&", "&"));
    KBookmarkDialog *dialog = owner()->bookmarkDialog(manager(), QApplication::activeWindow());
    dialog->editBookmark(selected);
    selected.setFullText(selected.text().replace("&", "&&"));
    delete dialog;
}


void BookmarkContextMenu::openFolderInTabs()
{
    if (bookmark().isGroup())
        owner()->openFolderinTabs(bookmark().toGroup());
}


void BookmarkContextMenu::newBookmarkGroup()
{
    KBookmark selected = bookmark();
    KBookmarkDialog *dialog = owner()->bookmarkDialog(manager(), QApplication::activeWindow());

    if (!selected.isNull())
    {
        if (selected.isGroup())
        {
            dialog->createNewFolder("New folder", selected);
        }

        else
        {
            KBookmark newBk;
            newBk = dialog->createNewFolder("New folder", selected.parentGroup());
            selected.parentGroup().moveBookmark(newBk, selected);
            manager()->emitChanged();
        }
    }
    else
    {
        dialog->createNewFolder("New folder");
    }

    delete dialog;
}


void BookmarkContextMenu::newSeparator()
{
    KBookmark selected = bookmark();
    KBookmark newBk;

    if (!selected.isNull())
    {
        if (selected.isGroup())
            newBk = selected.toGroup().createNewSeparator();
        else
            newBk = selected.parentGroup().createNewSeparator();
    }

    else
    {
        newBk = Application::bookmarkProvider()->rootGroup().createNewSeparator();
    }

    KBookmarkGroup parent = newBk.parentGroup();
    newBk.setIcon(("edit-clear"));
    parent.addBookmark(newBk);

    if (!selected.isNull())
        parent.moveBookmark(newBk, selected);

    manager()->emitChanged();
}


void BookmarkContextMenu::bookmarkCurrentPage()
{
    KBookmarkGroup parent = Application::bookmarkProvider()->rootGroup();
    KBookmark selected = bookmark();

    if (!selected.isNull())
    {
        parent = selected.parentGroup();

        if (selected.isGroup())
            parent = selected.toGroup();

        KBookmark newBk = parent.addBookmark(owner()->currentTitle().replace("&", "&&"), KUrl(owner()->currentUrl()), "text-html");
        parent.moveBookmark(newBk, selected.parentGroup().previous(selected));
    }

    else
    {
        parent.addBookmark(owner()->currentTitle(), KUrl(owner()->currentUrl()), "text-html");
    }

    manager()->emitChanged();
}

