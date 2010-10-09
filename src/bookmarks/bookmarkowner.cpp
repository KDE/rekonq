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
#include "bookmarkowner.moc"

// Local Includes
#include "application.h"
#include "bookmarkprovider.h"
#include "mainwindow.h"
#include "webtab.h"
#include "mainview.h"

// KDE Includes
#include <KAction>
#include <KBookmarkDialog>
#include <KLocalizedString>
#include <KMessageBox>
#include "iconmanager.h"

// Qt Includes
#include <QtGui/QClipboard>


BookmarkOwner::BookmarkOwner(KBookmarkManager *manager, QObject *parent)
        : QObject(parent)
        , KBookmarkOwner()
        , m_manager(manager)
{
}


KAction* BookmarkOwner::action(const KBookmark &bookmark, const BookmarkAction &bmAction)
{
    switch (bmAction)
    {
    case OPEN:
        return createAction(i18n("Open"), "tab-new",
                     i18n("Open bookmark in current tab"), SLOT(openBookmark(const KBookmark &)), bookmark);
    case OPEN_IN_TAB:
        return createAction(i18n("Open in New Tab"), "tab-new",
                     i18n("Open bookmark in new tab"), SLOT(openBookmarkInNewTab(const KBookmark &)), bookmark);
    case OPEN_IN_WINDOW:
        return createAction(i18n("Open in New Window"), "window-new",
                     i18n("Open bookmark in new window"), SLOT(openBookmarkInNewWindow(const KBookmark &)), bookmark);
    case OPEN_FOLDER:
        return createAction(i18n("Open Folder in Tabs"), "tab-new",
                     i18n("Open all the bookmarks in folder in tabs"), SLOT(openBookmarkFolder(const KBookmark &)), bookmark);
    case BOOKMARK_PAGE:
        return createAction(i18n("Add Bookmark"), "bookmark-new",
                     i18n("Bookmark current page"), SLOT(bookmarkCurrentPage(const KBookmark &)), bookmark);
    case NEW_FOLDER:
        return createAction(i18n("New Folder"), "folder-new",
                     i18n("Create a new bookmark folder"), SLOT(newBookmarkFolder(const KBookmark &)), bookmark);
    case NEW_SEPARATOR:
        return createAction(i18n("New Separator"), "edit-clear",
                     i18n("Create a new bookmark separator"), SLOT(newSeparator(const KBookmark &)), bookmark);
    case COPY:
        return createAction(i18n("Copy Link"), "edit-copy",
                     i18n("Copy the bookmark's link address"), SLOT(copyLink(const KBookmark &)), bookmark);
    case EDIT:
        return createAction(i18n("Edit"), "configure",
                     i18n("Edit the bookmark"), SLOT(editBookmark(const KBookmark &)), bookmark);
    case DELETE:
       return  createAction(i18n("Delete"), "edit-delete",
                     i18n("Delete the bookmark"), SLOT(deleteBookmark(const KBookmark &)), bookmark);
    default:
       return 0;
    }
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
    if (keyboardModifiers & Qt::ControlModifier || mouseButtons & Qt::MidButton)
    {
        openBookmarkInNewTab(bookmark);
    }
    else
    {
        openBookmark(bookmark);
    }
}


void BookmarkOwner::openFolderinTabs(const KBookmarkGroup &bookmark)
{
    QList<KUrl> urlList = bookmark.groupUrlList();

    if (urlList.length() > 8)
    {
        if (KMessageBox::warningContinueCancel(
                    Application::instance()->mainWindow(),
                    i18ncp("%1=Number of tabs. Value is always >=8",
                           "You are about to open %1 tabs.\nAre you sure?",
                           "You are about to open %1 tabs.\nAre you sure?", urlList.length()))
                != KMessageBox::Continue
           )
            return;
    }

    Q_FOREACH(const KUrl &url, urlList)
    {
        emit openUrl(url, Rekonq::NewFocusedTab);
    }
}


void BookmarkOwner::openBookmarkFolder(const KBookmark &bookmark)
{
    if (bookmark.isGroup())
        openFolderinTabs(bookmark.toGroup());
}


void BookmarkOwner::openBookmark(const KBookmark &bookmark)
{
    emit openUrl(bookmark.url(), Rekonq::CurrentTab);
}


void BookmarkOwner::openBookmarkInNewTab(const KBookmark &bookmark)
{
    emit openUrl(bookmark.url(), Rekonq::NewTab);
}


void BookmarkOwner::openBookmarkInNewWindow(const KBookmark &bookmark)
{
    emit openUrl(bookmark.url(), Rekonq::NewWindow);
}


KBookmark BookmarkOwner::bookmarkCurrentPage(const KBookmark &bookmark)
{
    KBookmark newBk;
    KBookmarkGroup parent;

    if (!bookmark.isNull())
    {
        if (bookmark.isGroup())
            parent = bookmark.toGroup();
        else
            parent = bookmark.parentGroup();

        newBk = parent.addBookmark(currentTitle().replace('&', "&&"), KUrl(currentUrl()) );
        parent.moveBookmark(newBk, bookmark);
    }
    else
    {
        parent = Application::bookmarkProvider()->rootGroup();
        newBk = parent.addBookmark(currentTitle(), KUrl(currentUrl()));
    }

    m_manager->emitChanged(parent);
    return newBk;
}


KBookmarkGroup BookmarkOwner::newBookmarkFolder(const KBookmark &bookmark)
{
    KBookmarkGroup newBk;
    KBookmarkDialog *dialog = bookmarkDialog(m_manager, QApplication::activeWindow());
    QString folderName = i18n("New folder");

    if (!bookmark.isNull())
    {
        if (bookmark.isGroup())
        {
            newBk = dialog->createNewFolder(folderName, bookmark);
        }
        else
        {
            newBk = dialog->createNewFolder(folderName, bookmark.parentGroup());
            if (!newBk.isNull())
            {
                KBookmarkGroup parent = newBk.parentGroup();
                parent.moveBookmark(newBk, bookmark);
                m_manager->emitChanged(parent);
            }
        }
    }
    else
    {
        newBk = dialog->createNewFolder(folderName);
    }

    delete dialog;
    return newBk;
}


KBookmark BookmarkOwner::newSeparator(const KBookmark &bookmark)
{
    KBookmark newBk;

    if (!bookmark.isNull())
    {
        if (bookmark.isGroup())
        {
            newBk = bookmark.toGroup().createNewSeparator();
        }
        else
        {
            newBk = bookmark.parentGroup().createNewSeparator();
            newBk.parentGroup().moveBookmark(newBk, bookmark);
        }
    }
    else
    {
        newBk = Application::bookmarkProvider()->rootGroup().createNewSeparator();
    }

    newBk.setIcon(("edit-clear"));

    m_manager->emitChanged(newBk.parentGroup());
    return newBk;
}


void BookmarkOwner::copyLink(const KBookmark &bookmark)
{
    if (bookmark.isNull())
        return;

    QApplication::clipboard()->setText(bookmark.url().url());
}


void BookmarkOwner::editBookmark(KBookmark bookmark)
{
    if (bookmark.isNull())
        return;

    bookmark.setFullText(bookmark.fullText().replace("&&", "&"));
    KBookmarkDialog *dialog = bookmarkDialog(m_manager, QApplication::activeWindow());

    dialog->editBookmark(bookmark);
    bookmark.setFullText(bookmark.fullText().replace('&', "&&"));

    delete dialog;
}


bool BookmarkOwner::deleteBookmark(KBookmark bookmark)
{
    if (bookmark.isNull())
        return false;

    KBookmarkGroup bmg = bookmark.parentGroup();
    QString name = QString(bookmark.fullText()).replace("&&", "&");
    QString dialogCaption, dialogText;

    if (bookmark.isGroup())
    {
        dialogCaption = i18n("Bookmark Folder Deletion");
        dialogText = i18n("Are you sure you wish to remove the bookmark folder\n\"%1\"?", name);
    }
    else if (bookmark.isSeparator())
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

    bmg.deleteBookmark(bookmark);
    m_manager->emitChanged(bmg);
    return true;
}


KAction* BookmarkOwner::createAction(const QString &text,
                                 const QString &icon, const QString &help, const char *slot,
                                 const KBookmark & bookmark)
{
    CustomBookmarkAction *act = new CustomBookmarkAction(bookmark, KIcon(icon), text, this);
    act->setHelpText(help);
    connect(act, SIGNAL(triggered(const KBookmark &)), this, slot);
    return act;
}


// -------------------------------------------------------------------------------------------------


CustomBookmarkAction::CustomBookmarkAction(const KBookmark &bookmark, const KIcon &icon, const QString &text, QObject *parent)
    : KAction(icon, text, parent)
    , m_bookmark(bookmark)
{
    connect(this, SIGNAL(triggered()), this, SLOT(onActionTriggered()));
}

CustomBookmarkAction::~CustomBookmarkAction()
{
}

void CustomBookmarkAction::onActionTriggered()
{
    emit triggered(m_bookmark);
}
