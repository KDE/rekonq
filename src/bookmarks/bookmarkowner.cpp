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
#include "bookmarkowner.h"

// Local Includes
#include "application.h"
#include "bookmarksmanager.h"
#include "mainwindow.h"
#include "webtab.h"
#include "mainview.h"

// KDE Includes
#include <KBookmarkDialog>
#include <KMessageBox>

// Qt Includes
#include <QtGui/QClipboard>


BookmarkOwner::BookmarkOwner(KBookmarkManager *manager, QObject *parent)
        : QObject(parent)
        , KBookmarkOwner()
        , m_manager(manager)
        , actions(QVector<KAction*>(NUM_ACTIONS))
{
    setupActions();
}


KAction* BookmarkOwner::action(const BookmarkAction &bmAction)
{
    return static_cast<KAction*>(actions.at(bmAction));
}


QString BookmarkOwner::currentTitle() const
{
    return Application::instance()->mainWindow()->currentTab()->view()->title();
}


QString BookmarkOwner::currentUrl() const
{
    return Application::instance()->mainWindow()->currentTab()->url().url();
}


bool BookmarkOwner::supportsTabs() const
{
    return true;
}


QList< QPair<QString, QString> > BookmarkOwner::currentBookmarkList() const
{
    QList< QPair<QString, QString> > bkList;
    MainView *view = Application::instance()->mainWindow()->mainView();
    int tabNumber = view->count();

    for (int i = 0; i < tabNumber; ++i)
    {
        QPair<QString, QString> item;
        item.first = view->webTab(i)->view()->title();
        item.second = view->webTab(i)->url().url();
        bkList << item;
    }

    return bkList;
}


void BookmarkOwner::openBookmark(const KBookmark &bookmark,
                                 Qt::MouseButtons mouseButtons,
                                 Qt::KeyboardModifiers keyboardModifiers)
{
    bookmarkSelected(bookmark);
    if (keyboardModifiers & Qt::ControlModifier || mouseButtons & Qt::MidButton)
    {
        openBookmarkInNewTab();
    }
    else
    {
        openBookmark();
    }
}


void BookmarkOwner::openFolderinTabs(const KBookmarkGroup &bookmark)
{
    bookmarkSelected(bookmark);
    openBookmarkFolder();
}


void BookmarkOwner::bookmarkSelected(const KBookmark &bookmark)
{
    selected = bookmark;
}


void BookmarkOwner::openBookmark()
{
    emit openUrl(selected.url(), Rekonq::CurrentTab);
}


void BookmarkOwner::openBookmarkInNewTab()
{
    emit openUrl(selected.url(), Rekonq::NewTab);
}


void BookmarkOwner::openBookmarkInNewWindow()
{
    emit openUrl(selected.url(), Rekonq::NewWindow);
}


void BookmarkOwner::openBookmarkFolder()
{
    if (!selected.isGroup())
        return;

    QList<KUrl> urlList = selected.toGroup().groupUrlList();

    if (urlList.length() > 8)
    {
        if (KMessageBox::warningContinueCancel(
                    Application::instance()->mainWindow(),
                    i18n("You are about to open %1 tabs.\nAre you sure?", urlList.length()))
                != KMessageBox::Continue
           )
            return;
    }

    foreach (KUrl url, urlList)
    {
        emit openUrl(url, Rekonq::NewFocusedTab);
    }
}


void BookmarkOwner::bookmarkCurrentPage()
{
    KBookmarkGroup parent;

    if (!selected.isNull())
    {
        if (selected.isGroup())
            parent = selected.toGroup();
        else
            parent = selected.parentGroup();

        KBookmark newBk = parent.addBookmark(currentTitle().replace('&', "&&"), KUrl(currentUrl()));
        parent.moveBookmark(newBk, selected);
    }
    else
    {
        parent = Application::bookmarkProvider()->rootGroup();
        parent.addBookmark(currentTitle(), KUrl(currentUrl()));
    }

    m_manager->emitChanged(parent);
}


void BookmarkOwner::newBookmarkFolder()
{
    KBookmarkDialog *dialog = bookmarkDialog(m_manager, QApplication::activeWindow());
    QString folderName = i18n("New folder");

    if (!selected.isNull())
    {
        if (selected.isGroup())
        {
            dialog->createNewFolder(folderName, selected);
        }
        else
        {
            KBookmark newBk = dialog->createNewFolder(folderName, selected.parentGroup());
            if (!newBk.isNull())
            {
                KBookmarkGroup parent = newBk.parentGroup();
                parent.moveBookmark(newBk, selected);
                m_manager->emitChanged(parent);
            }
        }
    }
    else
    {
        dialog->createNewFolder(folderName);
    }

    delete dialog;
}


void BookmarkOwner::newSeparator()
{
    KBookmark newBk;

    if (!selected.isNull())
    {
        if (selected.isGroup())
        {
            newBk = selected.toGroup().createNewSeparator();
        }
        else
        {
            newBk = selected.parentGroup().createNewSeparator();
            newBk.parentGroup().moveBookmark(newBk, selected);
        }
    }
    else
    {
        newBk = Application::bookmarkProvider()->rootGroup().createNewSeparator();
    }

    newBk.setIcon(("edit-clear"));

    m_manager->emitChanged(newBk.parentGroup());
}


void BookmarkOwner::copyLink()
{
    if (selected.isNull())
        return;

    QApplication::clipboard()->setText(selected.url().url());
}


void BookmarkOwner::editBookmark()
{
    if (selected.isNull())
        return;

    selected.setFullText(selected.fullText().replace("&&", "&"));
    KBookmarkDialog *dialog = bookmarkDialog(m_manager, QApplication::activeWindow());

    dialog->editBookmark(selected);
    selected.setFullText(selected.fullText().replace('&', "&&"));

    delete dialog;
}


bool BookmarkOwner::deleteBookmark()
{
    if (selected.isNull())
        return false;

    KBookmarkGroup bmg = selected.parentGroup();
    QString name = QString(selected.fullText()).replace("&&", "&");
    QString dialogCaption, dialogText;

    if (selected.isGroup())
    {
        dialogCaption = i18n("Bookmark Folder Deletion");
        dialogText = i18n("Are you sure you wish to remove the bookmark folder\n\"%1\"?", name);
    }
    else if (selected.isSeparator())
    {
        dialogCaption = i18n("Separator Deletion");
        dialogText = i18n("Are you sure you wish to remove this separator?");
    }
    else
    {
        dialogCaption = i18n("Bookmark Deletion");
        dialogText = i18n("Are you sure you wish to remove the bookmark\n\"%1\"?", name);
    }

    if (KMessageBox::warningContinueCancel(
                QApplication::activeWindow(),
                dialogText,
                dialogCaption,
                KStandardGuiItem::del(),
                KStandardGuiItem::cancel(),
                "bookmarkDeletition_askAgain")
            != KMessageBox::Continue
       )
        return false;

    bmg.deleteBookmark(selected);
    m_manager->emitChanged(bmg);
    return true;
}


void BookmarkOwner::setupActions()
{
    createAction(OPEN, i18n("Open"), "tab-new",
                 i18n("Open bookmark in current tab"), SLOT(openBookmark()));
    createAction(OPEN_IN_TAB, i18n("Open in New Tab"), "tab-new",
                 i18n("Open bookmark in new tab"), SLOT(openBookmarkInNewTab()));
    createAction(OPEN_IN_WINDOW, i18n("Open in New Window"), "window-new",
                 i18n("Open bookmark in new window"), SLOT(openBookmarkInNewWindow()));
    createAction(OPEN_FOLDER, i18n("Open Folder in Tabs"), "tab-new",
                 i18n("Open all the bookmarks in folder in tabs"), SLOT(openBookmarkFolder()));
    createAction(BOOKMARK_PAGE, i18n("Add Bookmark"), "bookmark-new",
                 i18n("Bookmark current page"), SLOT(bookmarkCurrentPage()));
    createAction(NEW_FOLDER, i18n("New Folder"), "folder-new",
                 i18n("Create a new bookmark folder"), SLOT(newBookmarkFolder()));
    createAction(NEW_SEPARATOR, i18n("New Separator"), "edit-clear",
                 i18n("Create a new bookmark separatork"), SLOT(newSeparator()));
    createAction(COPY, i18n("Copy Link"), "edit-copy",
                 i18n("Copy the bookmark's link address"), SLOT(copyLink()));
    createAction(EDIT, i18n("Edit"), "configure",
                 i18n("Edit the bookmark"), SLOT(editBookmark()));
    createAction(DELETE, i18n("Delete"), "edit-delete",
                 i18n("Delete the bookmark"), SLOT(deleteBookmark()));
}


void BookmarkOwner::createAction(const BookmarkAction &action, const QString &text,
                                 const QString &icon, const QString &help, const char *slot)
{
    KAction *act = new KAction(KIcon(icon), text, this);
    act->setHelpText(help);
    actions[action] = act;
    connect(act, SIGNAL(triggered()), this, slot);
}
