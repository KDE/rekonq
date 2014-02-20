/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2008-2014 by Andrea Diamantini <adjam7 at gmail dot com>
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
#include "bookmarkmanager.h"

#include "application.h"
#include "rekonqwindow.h"
#include "tabwidget.h"
#include "webwindow.h"

// KDE Includes
#include <KBookmarkDialog>
#include <KBookmarkManager>
#include <KLocalizedString>
#include <KMessageBox>

// Qt Includes
#include <QClipboard>

// Nepomuk Includes
#ifdef HAVE_NEPOMUK
// Local Nepomuk Includes
#include "resourcelinkdialog.h"

// Nepomuk Includes
#include <Nepomuk2/Resource>
#include <Nepomuk2/Vocabulary/NFO>
#endif


BookmarkOwner::BookmarkOwner(KBookmarkManager *manager, QObject *parent)
    : QObject(parent)
    , KBookmarkOwner()
    , m_manager(manager)
{
}


QAction* BookmarkOwner::createAction(const KBookmark &bookmark, const BookmarkAction &bmAction)
{
    switch (bmAction)
    {
    case OPEN:
        return createAction(i18n("Open"), QL1S("tab-new"),
                            i18n("Open bookmark in current tab"), SLOT(loadBookmark(KBookmark)), bookmark);
    case OPEN_IN_TAB:
        return createAction(i18n("Open in New Tab"), QL1S("tab-new"),
                            i18n("Open bookmark in new tab"), SLOT(loadBookmarkInNewTab(KBookmark)), bookmark);
    case OPEN_IN_WINDOW:
        return createAction(i18n("Open in New Window"), QL1S("window-new"),
                            i18n("Open bookmark in new window"), SLOT(loadBookmarkInNewWindow(KBookmark)), bookmark);
    case OPEN_FOLDER:
        return createAction(i18n("Open Folder in Tabs"), QL1S("tab-new"),
                            i18n("Open all the bookmarks in folder in tabs"), SLOT(loadBookmarkFolder(KBookmark)), bookmark);
    case BOOKMARK_PAGE:
        return createAction(i18n("Add Bookmark"), QL1S("bookmark-new"),
                            i18n("Bookmark current page"), SLOT(bookmarkCurrentPage(KBookmark)), bookmark);
    case NEW_FOLDER:
        return createAction(i18n("New Folder"), QL1S("folder-new"),
                            i18n("Create a new bookmark folder"), SLOT(newBookmarkFolder(KBookmark)), bookmark);
    case NEW_SEPARATOR:
        return createAction(i18n("New Separator"), QL1S("edit-clear"),
                            i18n("Create a new bookmark separator"), SLOT(newSeparator(KBookmark)), bookmark);
    case COPY:
        return createAction(i18n("Copy Link"), QL1S("edit-copy"),
                            i18n("Copy the bookmark's link address"), SLOT(copyLink(KBookmark)), bookmark);
    case EDIT:
        return createAction(i18n("Edit"), QL1S("configure"),
                            i18n("Edit the bookmark"), SLOT(editBookmark(KBookmark)), bookmark);
#ifdef HAVE_NEPOMUK
    case FANCYBOOKMARK:
        return createAction(i18n("Fancy Bookmark"), QL1S("nepomuk"),
                            i18n("Link Nepomuk resources"), SLOT(fancyBookmark(KBookmark)), bookmark);
#endif
    case DELETE:
        return  createAction(i18n("Delete"), QL1S("edit-delete"),
                             i18n("Delete the bookmark"), SLOT(deleteBookmark(KBookmark)), bookmark);
    case SET_TOOLBAR_FOLDER:
        return  createAction(i18n("Set as toolbar folder"), QL1S("bookmark-toolbar"),
                             QL1S(""), SLOT(setToolBarFolder(KBookmark)), bookmark);
    case UNSET_TOOLBAR_FOLDER:
        return  createAction(i18n("Unset this folder as the toolbar folder"), QL1S("bookmark-toolbar"),
                             QL1S(""), SLOT(unsetToolBarFolder()), bookmark);
    default:
        ASSERT_NOT_REACHED(unknown BookmarkAction);
        return 0;
    }
}


QString BookmarkOwner::currentTitle() const
{
    return rApp->rekonqWindow()->currentWebWindow()->title();
}


QUrl BookmarkOwner::currentUrl() const
{
    return rApp->rekonqWindow()->currentWebWindow()->url();
}


QList<KBookmarkOwner::FutureBookmark> BookmarkOwner::currentBookmarkList() const
{
    QList<KBookmarkOwner::FutureBookmark> bkList;
    TabWidget *view = rApp->rekonqWindow()->tabWidget();
    int tabNumber = view->count();

    // FIXME
//     for (int i = 0; i < tabNumber; ++i)
//     {
//         QPair<QString, QString> item;
//         item.first = view->webWindow(i)->title();
//         item.second = view->webWindow(i)->url().url();
//         bkList << item;
//     }

    return bkList;
}


void BookmarkOwner::openBookmark(const KBookmark &bookmark,
                                 Qt::MouseButtons mouseButtons,
                                 Qt::KeyboardModifiers keyboardModifiers)
{
    if (keyboardModifiers & Qt::ControlModifier || mouseButtons & Qt::MidButton)
        loadBookmarkInNewTab(bookmark);
    else
        loadBookmark(bookmark);
}


void BookmarkOwner::openFolderinTabs(const KBookmarkGroup &bkGoup)
{
    QList<QUrl> urlList = bkGoup.groupUrlList();

    if (urlList.length() > 8)
    {
        if (KMessageBox::warningContinueCancel(
                    rApp->rekonqWindow(),
                    i18ncp("%1=Number of tabs. Value is always >=8",
                           "You are about to open %1 tabs.\nAre you sure?",
                           "You are about to open %1 tabs.\nAre you sure?", urlList.length()))
                != KMessageBox::Continue
           )
            return;
    }

    Q_FOREACH(const QUrl & url, urlList)
    {
        emit openUrl(url, Rekonq::NewFocusedTab);
    }
}


void BookmarkOwner::loadBookmark(const KBookmark &bookmark)
{
    emit openUrl(bookmark.url(), Rekonq::CurrentTab);
}


void BookmarkOwner::loadBookmarkInNewTab(const KBookmark &bookmark)
{
    emit openUrl(bookmark.url(), Rekonq::NewTab);
}


void BookmarkOwner::loadBookmarkInNewWindow(const KBookmark &bookmark)
{
    emit openUrl(bookmark.url(), Rekonq::NewWindow);
}


void BookmarkOwner::loadBookmarkFolder(const KBookmark &bookmark)
{
    Q_ASSERT(bookmark.isGroup());
    openFolderinTabs(bookmark.toGroup());
}


KBookmark BookmarkOwner::bookmarkCurrentPage(const KBookmark &bookmark)
{
    KBookmarkGroup parent;

    if (!bookmark.isNull())
    {
        if (bookmark.isGroup())
            parent = bookmark.toGroup();
        else
            parent = bookmark.parentGroup();
    }
    else
    {
        parent = BookmarkManager::self()->rootGroup();
#ifdef HAVE_NEPOMUK
        Nepomuk2::Resource nfoResource;
        nfoResource = ((QUrl)currentUrl());
        nfoResource.addType(Nepomuk2::Vocabulary::NFO::Website());
        nfoResource.setLabel(currentTitle());
#endif
    }

    KBookmark newBk = parent.addBookmark(currentTitle(), QUrl(currentUrl()), QString());
    if (!bookmark.isNull())
        parent.moveBookmark(newBk, bookmark);

    m_manager->emitChanged(parent);
    return newBk;
}


KBookmarkGroup BookmarkOwner::newBookmarkFolder(const KBookmark &bookmark, const QString &name)
{
    KBookmarkGroup newBk;
    KBookmarkDialog *dialog = bookmarkDialog(m_manager, 0);

    QString folderName;
    if (name.isEmpty())
        folderName = i18n("New folder");
    else
        folderName = name;

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
        newBk = BookmarkManager::self()->rootGroup().createNewSeparator();
    }

    newBk.setIcon( QL1S("edit-clear") );

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

    KBookmarkDialog *dialog = bookmarkDialog(m_manager, 0);
    dialog->editBookmark(bookmark);

    delete dialog;
}


#ifdef HAVE_NEPOMUK
void BookmarkOwner::fancyBookmark(KBookmark bookmark)
{
    Nepomuk2::Resource nfoResource = (QUrl)bookmark.url();

    QPointer<Nepomuk2::ResourceLinkDialog> r = new Nepomuk2::ResourceLinkDialog(nfoResource);
    r->exec();

    r->deleteLater();
}
#endif

bool BookmarkOwner::deleteBookmark(const KBookmark &bookmark)
{
    if (bookmark.isNull())
        return false;

    KBookmarkGroup bmg = bookmark.parentGroup();
    QString dialogCaption, dialogText;

    if (bookmark.isGroup())
    {
        dialogCaption = i18n("Bookmark Folder Deletion");
        dialogText = i18n("Are you sure you wish to remove the bookmark folder\n\"%1\"?", bookmark.fullText());
    }
    else if (bookmark.isSeparator())
    {
        dialogCaption = i18n("Separator Deletion");
        dialogText = i18n("Are you sure you wish to remove this separator?");
    }
    else
    {
        dialogCaption = i18n("Bookmark Deletion");
        dialogText = i18n("Are you sure you wish to remove the bookmark\n\"%1\"?", bookmark.fullText());
    }

    if (KMessageBox::warningContinueCancel(
                0,
                dialogText,
                dialogCaption,
                KStandardGuiItem::del(),
                KStandardGuiItem::cancel(),
                QL1S("bookmarkDeletition_askAgain"))
            != KMessageBox::Continue
       )
        return false;

    bmg.deleteBookmark(bookmark);
#ifdef HAVE_NEPOMUK
    Nepomuk2::Resource nfoResource(bookmark.url());
    nfoResource.remove();
#endif
    m_manager->emitChanged(bmg);
    return true;
}


void BookmarkOwner::setToolBarFolder(KBookmark bookmark)
{
    if (!bookmark.isGroup())
        return;

    unsetToolBarFolder();
    bookmark.internalElement().setAttribute( QL1S("toolbar"), QL1S("yes") );
    bookmark.setIcon( QL1S("bookmark-toolbar") );

    m_manager->emitChanged();
}


void BookmarkOwner::unsetToolBarFolder()
{
    KBookmarkGroup toolbar = m_manager->toolbar();
    if (!toolbar.isNull())
    {
        toolbar.internalElement().setAttribute( QL1S("toolbar"), QL1S("no") );
        toolbar.setIcon( QL1S("") );
    }
    m_manager->emitChanged();
}


QAction* BookmarkOwner::createAction(const QString &text, const QString &icon,
                                     const QString &help, const char *slot,
                                     const KBookmark &bookmark)
{
    CustomBookmarkAction *act = new CustomBookmarkAction(bookmark, QIcon::fromTheme(icon), text, this);
// FIXME    act->setHelpText(help);
    connect(act, SIGNAL(triggered(KBookmark)), this, slot);
    return act;
}


// -------------------------------------------------------------------------------------------------


CustomBookmarkAction::CustomBookmarkAction(const KBookmark &bookmark, const QIcon &icon, const QString &text, QObject *parent)
    : QAction(icon, text, parent)
    , m_bookmark(bookmark)
{
    connect(this, SIGNAL(triggered()), this, SLOT(onActionTriggered()));
}

void CustomBookmarkAction::onActionTriggered()
{
    emit triggered(m_bookmark);
}
