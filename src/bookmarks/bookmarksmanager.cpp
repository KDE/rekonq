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
#include "bookmarksmanager.h"
#include "bookmarksmanager.moc"

// Local Includes
#include "mainwindow.h"
#include "webtab.h"
#include "webview.h"
#include "mainview.h"
#include "bookmarkscontextmenu.h"

// KDE Includes
#include <KActionCollection>
#include <KBookmarkAction>
#include <KBookmarkDialog>
#include <KBookmarkGroup>
#include <KToolBar>
#include <KMenu>
#include <KStandardDirs>
#include <KUrl>
#include <KMessageBox>

// Qt Includes
#include <QtCore/QFile>
#include <QtGui/QActionGroup>


BookmarkOwner::BookmarkOwner(QObject *parent)
        : QObject(parent)
        , KBookmarkOwner()
{
}


void BookmarkOwner::openBookmark(const KBookmark & bookmark,
                                 Qt::MouseButtons mouseButtons,
                                 Qt::KeyboardModifiers keyboardModifiers)
{
    if (keyboardModifiers & Qt::ControlModifier || mouseButtons == Qt::MidButton)
    {
        emit openUrl(bookmark.url(), Rekonq::NewTab);
    }
    else
    {
        emit openUrl(bookmark.url(), Rekonq::CurrentTab);
    }
}


void BookmarkOwner::bookmarkPage(KBookmark &position)
{
    BookmarkOwner *owner = Application::bookmarkProvider()->bookmarkOwner();
    KBookmarkGroup parent = Application::bookmarkProvider()->rootGroup();

    if (!position.isNull())
    {
        parent = position.parentGroup();

        if (position.isGroup())
            parent = position.toGroup();

        KBookmark newBk = parent.addBookmark(owner->currentTitle().replace('&', "&&"), KUrl(owner->currentUrl()));
        parent.moveBookmark(newBk, position.parentGroup().previous(position));
    }

    else
    {
        parent.addBookmark(owner->currentTitle(), KUrl(owner->currentUrl()));
    }

    Application::bookmarkProvider()->bookmarkManager()->emitChanged(parent);
}


void BookmarkOwner::newBookmarkFolder(KBookmark &position)
{
    BookmarkOwner *owner = Application::bookmarkProvider()->bookmarkOwner();
    KBookmarkManager *manager = Application::bookmarkProvider()->bookmarkManager();

    KBookmarkDialog *dialog = owner->bookmarkDialog(manager, QApplication::activeWindow());

    if (!position.isNull())
    {
        if (position.isGroup())
        {
            dialog->createNewFolder("New folder", position);
        }

        else
        {
            KBookmark newBk;
            newBk = dialog->createNewFolder("New folder", position.parentGroup());
            if (!newBk.isNull())
            {
                position.parentGroup().moveBookmark(newBk, position);
                manager->emitChanged(newBk.parentGroup());
            }
        }
    }
    else
    {
        dialog->createNewFolder("New folder");
    }

    delete dialog;
}


void BookmarkOwner::newSeparator(KBookmark &position)
{
    KBookmark newBk;

    if (!position.isNull())
    {
        if (position.isGroup())
            newBk = position.toGroup().createNewSeparator();
        else
            newBk = position.parentGroup().createNewSeparator();
    }

    else
    {
        newBk = Application::bookmarkProvider()->rootGroup().createNewSeparator();
    }

    KBookmarkGroup parent = newBk.parentGroup();
    newBk.setIcon(("edit-clear"));
    parent.addBookmark(newBk);

    if (!position.isNull())
        parent.moveBookmark(newBk, position);

    Application::bookmarkProvider()->bookmarkManager()->emitChanged(newBk.parentGroup());
}


void BookmarkOwner::editBookmark(KBookmark &bookmark)
{
    BookmarkOwner *owner = Application::bookmarkProvider()->bookmarkOwner();
    KBookmarkManager *manager = Application::bookmarkProvider()->bookmarkManager();
    bookmark.setFullText(bookmark.fullText().replace("&&", "&"));
    KBookmarkDialog *dialog = owner->bookmarkDialog(manager, QApplication::activeWindow());
    dialog->editBookmark(bookmark);
    bookmark.setFullText(bookmark.fullText().replace('&', "&&"));
    delete dialog;
}


bool BookmarkOwner::deleteBookmark(KBookmark &bookmark)
{
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
        dialogText = i18n("Are you sure you wish to remove this separator?", name);
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

    KBookmarkGroup bmg = bookmark.parentGroup();
    bmg.deleteBookmark(bookmark);
    Application::bookmarkProvider()->bookmarkManager()->emitChanged(bmg);
    return true;
}


bool BookmarkOwner::supportsTabs() const
{
    return true;
}


QString BookmarkOwner::currentUrl() const
{
    return Application::instance()->mainWindow()->currentTab()->url().url();
}


QString BookmarkOwner::currentTitle() const
{
    return Application::instance()->mainWindow()->currentTab()->view()->title();
}


void BookmarkOwner::openFolderinTabs(const KBookmarkGroup &bookmark)
{
    QList<KUrl> urlList = bookmark.groupUrlList();

    if (urlList.length() > 8)
    {
        if ( !(KMessageBox::warningContinueCancel(  Application::instance()->mainWindow(),
                                                    i18ncp("%1=Number of tabs. Value is always >=8",
                                                           "You are about to open %1 tabs.\nAre you sure?",
                                                           "You are about to open %1 tabs.\nAre you sure?",
                                                           urlList.length()),
                                                    "",
                                                    KStandardGuiItem::cont(),
                                                    KStandardGuiItem::cancel(),
                                                    "openFolderInTabs_askAgain"
                                                 ) == KMessageBox::Continue)
           )
            return;
    }

    QList<KUrl>::iterator url;
    for (url = urlList.begin(); url != urlList.end(); ++url)
    {
        emit openUrl(*url, Rekonq::NewFocusedTab);
    }
}


QList< QPair<QString, QString> > BookmarkOwner::currentBookmarkList() const
{
    QList< QPair<QString, QString> > bkList;
    int tabNumber = Application::instance()->mainWindow()->mainView()->count();

    for (int i = 0; i < tabNumber; i++)
    {
        QPair<QString, QString> item;
        item.first = Application::instance()->mainWindow()->mainView()->webTab(i)->view()->title();
        item.second = Application::instance()->mainWindow()->mainView()->webTab(i)->url().url();
        bkList += item;
    }
    return bkList;
}


// ------------------------------------------------------------------------------------------------------


BookmarkProvider::BookmarkProvider(QObject *parent)
        : QObject(parent)
        , m_manager(0)
        , m_owner(0)
        , m_actionCollection(new KActionCollection(this))
        , _bookmarkActionMenu(0)
{
    kDebug() << "Loading Bookmarks Manager...";

    KUrl bookfile = KUrl("~/.kde/share/apps/konqueror/bookmarks.xml");  // share konqueror bookmarks

    if (!QFile::exists(bookfile.path()))
    {
        bookfile = KUrl("~/.kde4/share/apps/konqueror/bookmarks.xml");
        if (!QFile::exists(bookfile.path()))
        {
            QString bookmarksDefaultPath = KStandardDirs::locate("appdata" , "defaultbookmarks.xbel");
            QFile bkms(bookmarksDefaultPath);
            QString bookmarksPath = KStandardDirs::locateLocal("appdata", "bookmarks.xml", true);
            bookmarksPath.replace("rekonq", "konqueror");
            bkms.copy(bookmarksPath);

            bookfile = KUrl(bookmarksPath);
        }
    }

    m_manager = KBookmarkManager::managerForFile(bookfile.path(), "rekonq");

    connect(m_manager, SIGNAL(changed(const QString &, const QString &)),
            this, SLOT(slotBookmarksChanged(const QString &, const QString &)));

    // setup menu
    m_owner = new BookmarkOwner(this);
    connect(m_owner, SIGNAL(openUrl(const KUrl&, const Rekonq::OpenType &)), this, SIGNAL(openUrl(const KUrl&, const Rekonq::OpenType &)));

    KAction *a = KStandardAction::addBookmark(this, SLOT(slotAddBookmark()), this);
    m_actionCollection->addAction(QL1S("rekonq_add_bookmark"), a);

    kDebug() << "Loading Bookmarks Manager... DONE!";
}


BookmarkProvider::~BookmarkProvider()
{
    delete m_actionCollection;
    delete m_owner;
    delete m_manager;
}


void BookmarkProvider::setupBookmarkBar(BookmarkToolBar *toolbar)
{
    if (m_bookmarkToolBars.contains(toolbar))
        return;

    kDebug() << "new bookmark bar...";

    m_bookmarkToolBars.append(toolbar);
    toolbar->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(toolbar, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(contextMenu(const QPoint &)));

    kDebug() << "new bookmark bar... DONE!";
}


void BookmarkProvider::removeToolBar(BookmarkToolBar *toolbar)
{
    m_bookmarkToolBars.removeOne(toolbar);
}


void BookmarkProvider::slotBookmarksChanged(const QString &group, const QString &caller)
{
    Q_UNUSED(group)
    Q_UNUSED(caller)

    foreach(BookmarkToolBar *bookmarkToolBar, m_bookmarkToolBars)
    {
        if (bookmarkToolBar)
        {
            bookmarkToolBar->clear();
            fillBookmarkBar(bookmarkToolBar);
        }
    }
}


QAction *BookmarkProvider::actionByName(const QString &name)
{
    QAction *action = m_actionCollection->action(name);
    if (action)
        return action;
    return new QAction(this);  // return empty object instead of NULL pointer
}


void BookmarkProvider::contextMenu(const QPoint &point)
{
    if (m_bookmarkToolBars.isEmpty())
        return;

    KToolBar *bookmarkToolBar = m_bookmarkToolBars.at(0);
    if (!bookmarkToolBar)
        return;

    KBookmarkActionInterface* action = dynamic_cast<KBookmarkActionInterface *>(bookmarkToolBar->actionAt(point));
    if (!action)
        return;

    BookmarksContextMenu menu(action->bookmark(), bookmarkManager(), bookmarkOwner());
    menu.exec(bookmarkToolBar->mapToGlobal(point));
}


KActionMenu* BookmarkProvider::bookmarkActionMenu(QWidget *parent)
{
    kDebug() << "new Bookmarks Menu...";

    KMenu *menu = new KMenu(parent);
    _bookmarkActionMenu = new KActionMenu(parent);
    _bookmarkActionMenu->setMenu(menu);
    _bookmarkActionMenu->setText(i18n("&Bookmarks"));
    new BookmarkMenu(m_manager, m_owner, menu, m_actionCollection);

    kDebug() << "new Bookmarks Menu...DONE";
    return _bookmarkActionMenu;
}


KAction* BookmarkProvider::bookmarkToolBarAction(KToolBar *t)
{
    KAction *bookmarkToolBarAction = new KAction(this);
    bookmarkToolBarAction->setDefaultWidget(t);     // The ownership is transferred to action
    bookmarkToolBarAction->setText(i18n("Bookmarks Bar"));
    bookmarkToolBarAction->setShortcutConfigurable(false);
    return bookmarkToolBarAction;
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
            toolBar->addAction(menuAction);
        }

        else if (bookmark.isSeparator())
        {
            toolBar->addSeparator();
        }

        else
        {
            KBookmarkAction* action = new KBookmarkAction(bookmark, m_owner, this);
            action->setIconText(action->iconText().replace('&', "&&"));
            connect(action, SIGNAL(hovered()), toolBar, SLOT(actionHovered()));
            toolBar->addAction(action);
        }
    }
}


KBookmarkGroup BookmarkProvider::rootGroup()
{
    return m_manager->root();
}


QList<KBookmark> BookmarkProvider::find(QString text)
{
    QList<KBookmark> list;
    KBookmarkGroup bookGroup = Application::bookmarkProvider()->rootGroup();
    if (bookGroup.isNull())
    {
        return list;
    }

    KBookmark bookmark = bookGroup.first();
    while (!bookmark.isNull())
    {
        list = find(list, bookmark, text);
        bookmark = bookGroup.next(bookmark);
    }

    return list;
}


QList<KBookmark> BookmarkProvider::find(QList<KBookmark> list, const KBookmark &bookmark, QString text)
{
    if (bookmark.isGroup())
    {
        KBookmarkGroup group = bookmark.toGroup();
        KBookmark bm = group.first();
        while (!bm.isNull())
        {
            list = find(list, bm, text); // it is .bookfolder
            bm = group.next(bm);
        }
    }
    else if (bookmark.url().url().contains(text) || bookmark.fullText().contains(text))
    {
        list << bookmark;
    }

    return list;
}


void BookmarkProvider::slotAddBookmark()
{
    KBookmarkGroup parentBookmark = rootGroup();
    parentBookmark.addBookmark(bookmarkOwner()->currentTitle(), bookmarkOwner()->currentUrl());
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
    m_bookmarkPanels.removeOne(panel);

    if (m_bookmarkPanels.isEmpty())
    {
        Application::bookmarkProvider()->bookmarkManager()->emitChanged();
    }
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
    KBookmark found;

    KBookmarkGroup root = rootGroup();
    if (root.isNull())
    {
        return found;
    }

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
