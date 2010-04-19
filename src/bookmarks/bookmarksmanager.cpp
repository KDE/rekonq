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
#include "bookmarkcontextmenu.h"

// KDE Includes
#include <KActionCollection>
#include <KBookmarkAction>
#include <KBookmarkGroup>
#include <KToolBar>
#include <KDebug>
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
        emit openUrl(bookmark.url(), Rekonq::SettingOpenTab);
    }
    else
    {
        emit openUrl(bookmark.url(), Rekonq::CurrentTab);
    }
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

    if(urlList.length() > 8)
    {
        if(!(KMessageBox::warningContinueCancel(Application::instance()->mainWindow(), i18n("You are about to open %1 tabs.\nAre you sure  ?", QString::number(urlList.length()))) == KMessageBox::Continue))
            return;
    }

    QList<KUrl>::iterator url;
    for (url = urlList.begin(); url != urlList.end(); ++url)
    {
        emit openUrl(*url, Rekonq::NewCurrentTab);
    }
}


QList< QPair<QString, QString> > BookmarkOwner::currentBookmarkList() const
{
    QList< QPair<QString, QString> > bkList;
    int tabNumber = Application::instance()->mainWindow()->mainView()->count();

    for(int i = 0; i < tabNumber; i++)
    {
        QPair<QString, QString> item;
        item.first = Application::instance()->mainWindow()->mainView()->webTab(i)->view()->title();
        item.second = Application::instance()->mainWindow()->mainView()->webTab(i)->url().url();
        bkList += item;
    }
    return bkList;
}


// ------------------------------------------------------------------------------------------------------


BookmarkMenu::BookmarkMenu(KBookmarkManager *manager,
                           KBookmarkOwner *owner,
                           KMenu *menu,
                           KActionCollection* actionCollection)
    : KBookmarkMenu(manager, owner, menu, actionCollection)
{
    KAction *a = KStandardAction::addBookmark(this, SLOT(slotAddBookmark()), this);
    actionCollection->addAction(QLatin1String("rekonq_add_bookmark"),a);
    refill();
}


BookmarkMenu::BookmarkMenu(KBookmarkManager  *manager,
                           KBookmarkOwner  *owner,
                           KMenu  *parentMenu,
                           const QString &parentAddress)
    : KBookmarkMenu(manager, owner, parentMenu, parentAddress)
{
    refill();
}


BookmarkMenu::~BookmarkMenu()
{
}


KMenu * BookmarkMenu::contextMenu(QAction *act)
{

    KBookmarkActionInterface* action = dynamic_cast<KBookmarkActionInterface *>(act);
     if (!action)
         return 0;
     return new BookmarkContextMenu(action->bookmark(), manager(), owner());
}


void BookmarkMenu::slotAddBookmark()
{
    KAction *action = qobject_cast<KAction *>(sender());
    if (action && !action->data().isNull())
    {
        KBookmarkGroup parentBookmark = manager()->findByAddress(parentAddress()).toGroup();
        /// TODO Add bookmark Icon
        parentBookmark.addBookmark(owner()->currentTitle(), action->data().toUrl());
        manager()->emitChanged();
        return;
    }

    KBookmarkMenu::slotAddBookmark();
}


QAction * BookmarkMenu::actionForBookmark(const KBookmark &bookmark)
{
    if(bookmark.isGroup())
    {
        KBookmarkActionMenu *actionMenu = new KBookmarkActionMenu(bookmark, this);
        new BookmarkMenu(manager(), owner(), actionMenu->menu(), bookmark.address());
        return actionMenu;
    }
    else if(bookmark.isSeparator())
    {
        return KBookmarkMenu::actionForBookmark(bookmark);
    }
    else
    {
        Application::bookmarkProvider()->completionObject()->addItem(bookmark.url().url());
        return  new KBookmarkAction( bookmark, owner(), this );
    }
}


void BookmarkMenu::refill()
{
    fillBookmarks();

    if(parentMenu()->actions().count() > 0)
        parentMenu()->addSeparator();

    if(isRoot())
    {
        addAddBookmark();
        addAddBookmarksList();
        addNewFolder();
        addEditBookmarks();

    }
    else
    {
        addOpenFolderInTabs();
        addAddBookmark();
        addAddBookmarksList();
        addNewFolder();
    }
}


void BookmarkMenu::addOpenFolderInTabs()
{
    KAction *action;
    KBookmarkGroup group = manager()->findByAddress(parentAddress()).toGroup();

    if(!group.first().isNull())
    {
        KBookmark bookmark = group.first();

        while(bookmark.isGroup() || bookmark.isSeparator())
        {
            bookmark = group.next(bookmark);
        }

        if(!bookmark.isNull())
        {
            action = new KAction(KIcon("tab-new"), i18n("Open Folder in Tabs"), this);
            action->setHelpText( i18n( "Open all bookmarks in this folder as a new tab." ) );
            connect(action, SIGNAL(triggered(bool)), this, SLOT(slotOpenFolderInTabs()));
            parentMenu()->addAction(action);
        }
    }
}


// ------------------------------------------------------------------------------------------------------


BookmarkProvider::BookmarkProvider(QObject *parent)
        : QObject(parent)
        , m_manager(0)
        , m_owner(0)
        , m_actionCollection(new KActionCollection(this))
        , m_bookmarkMenu(0)
        , m_completion(0)
{
    // take care of the completion object
    m_completion = new KCompletion;
    m_completion->setOrder( KCompletion::Weighted );

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
}


BookmarkProvider::~BookmarkProvider()
{
    delete m_bookmarkMenu;
    delete m_actionCollection;
    delete m_owner;
    delete m_manager;
}


void BookmarkProvider::setupBookmarkBar(KToolBar *toolbar)
{
    KToolBar *bookmarkToolBar = toolbar;
    m_bookmarkToolBars.append(bookmarkToolBar);
    bookmarkToolBar->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(bookmarkToolBar, SIGNAL(customContextMenuRequested(const QPoint &)),
            this, SLOT(contextMenu(const QPoint &)));

    slotBookmarksChanged("", "");
}


void BookmarkProvider::removeToolBar(KToolBar *toolbar)
{
    m_bookmarkToolBars.removeOne(toolbar);
}


void BookmarkProvider::slotBookmarksChanged(const QString &group, const QString &caller)
{
    Q_UNUSED(group)
    Q_UNUSED(caller)

    m_completion->clear();

    foreach(KToolBar *bookmarkToolBar, m_bookmarkToolBars)
    {
        if (bookmarkToolBar)
        {
            KBookmarkGroup toolBarGroup = m_manager->toolbar();
            if (toolBarGroup.isNull())
                return;

            bookmarkToolBar->clear();

            KBookmark bookmark = toolBarGroup.first();
            while (!bookmark.isNull())
            {
                bookmarkToolBar->addAction(fillBookmarkBar(bookmark));
                bookmark = toolBarGroup.next(bookmark);
            }
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
    if(m_bookmarkToolBars.isEmpty())
        return;

    KToolBar *bookmarkToolBar = m_bookmarkToolBars.at(0);
    if(!bookmarkToolBar)
        return;

    KBookmarkActionInterface* action = dynamic_cast<KBookmarkActionInterface *>(bookmarkToolBar->actionAt(point));
    if (!action)
        return;

    KMenu *menu = new BookmarkContextMenu(action->bookmark(), bookmarkManager(), bookmarkOwner());
    if (!menu)
        return;

    menu->popup(bookmarkToolBar->mapToGlobal(point));
}


KActionMenu* BookmarkProvider::bookmarkActionMenu(QWidget *parent)
{
    KMenu *menu = new KMenu(parent);
    m_bookmarkMenu = new BookmarkMenu(m_manager, m_owner, menu, m_actionCollection);
    KActionMenu *bookmarkActionMenu = new KActionMenu(parent);
    bookmarkActionMenu->setMenu(menu);
    bookmarkActionMenu->setText(i18n("&Bookmarks"));
    return bookmarkActionMenu;
}


KAction *BookmarkProvider::fillBookmarkBar(const KBookmark &bookmark)
{
    if (bookmark.isGroup())
    {
        KBookmarkActionMenu *menuAction = new KBookmarkActionMenu(bookmark.toGroup(), this);
        menuAction->setDelayed(false);
        new BookmarkMenu(bookmarkManager(), bookmarkOwner(), menuAction->menu(), bookmark.address());

        return menuAction;
    }
 
    if(bookmark.isSeparator())
    {
        KAction *a = new KBookmarkAction(bookmark, m_owner, this);
        a->setText("");
        a->setIcon(QIcon());
        a->setSeparator(true);
        return a;
    }
    else
    {
        return new KBookmarkAction(bookmark, m_owner, this);
    }
}


KBookmarkGroup BookmarkProvider::rootGroup()
{
    return m_manager->root();
}


KCompletion *BookmarkProvider::completionObject() const
{
    return m_completion;
}


QString BookmarkProvider::titleForBookmarkUrl(QString url)
{
    QString title = "";
    KBookmarkGroup bookGroup = Application::bookmarkProvider()->rootGroup();
    if (bookGroup.isNull())
    {
        return title;
    }

    KBookmark bookmark = bookGroup.first();
    while (!bookmark.isNull() && title.isEmpty())
    {
        title = titleForBookmarkUrl(bookmark, url);
        bookmark = bookGroup.next(bookmark);
    }
    
    if (title.isEmpty())
    {
        title = url;
    }
    
    return title;
}


QString BookmarkProvider::titleForBookmarkUrl(const KBookmark &bookmark, QString url)
{
    QString title = "";
    if (bookmark.isGroup())
    {
        KBookmarkGroup group = bookmark.toGroup();
        KBookmark bm = group.first();
        while (!bm.isNull() && title.isEmpty())
        {
            title = titleForBookmarkUrl(bm, url); // it is .bookfolder
            bm = group.next(bm);
        }
    }
    else if(!bookmark.isSeparator() && bookmark.url()==url)
    {
        title = bookmark.fullText();
    }
    
    return title;
}
