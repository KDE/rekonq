/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2008-2010 by Andrea Diamantini <adjam7 at gmail dot com>
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
#include "bookmarkstoolbar.h"
#include "bookmarkstoolbar.moc"

// Local Includes
#include "bookmarkscontextmenu.h"
#include "mainwindow.h"
#include "application.h"
#include "bookmarksmanager.h"


BookmarkMenu::BookmarkMenu(KBookmarkManager *manager,
                           KBookmarkOwner *owner,
                           KMenu *menu,
                           KActionCollection* actionCollection)
        : KBookmarkMenu(manager, owner, menu, actionCollection)
{
}


BookmarkMenu::BookmarkMenu(KBookmarkManager  *manager,
                           KBookmarkOwner  *owner,
                           KMenu  *parentMenu,
                           const QString &parentAddress)
        : KBookmarkMenu(manager, owner, parentMenu, parentAddress)
{
}


BookmarkMenu::~BookmarkMenu()
{
}


KMenu * BookmarkMenu::contextMenu(QAction *act)
{

    KBookmarkActionInterface* action = dynamic_cast<KBookmarkActionInterface *>(act);
    if (!action)
        return 0;
    return new BookmarksContextMenu(action->bookmark(), manager(), static_cast<BookmarkOwner*>(owner()));
}


QAction * BookmarkMenu::actionForBookmark(const KBookmark &bookmark)
{
    if (bookmark.isGroup())
    {
        KBookmarkActionMenu *actionMenu = new KBookmarkActionMenu(bookmark, this);
        BookmarkMenu *menu = new BookmarkMenu(manager(), owner(), actionMenu->menu(), bookmark.address());
        // An hack to get rid of bug 219274
        connect(actionMenu, SIGNAL(hovered()), menu, SLOT(slotAboutToShow()));
        return actionMenu;
    }
    else if (bookmark.isSeparator())
    {
        return KBookmarkMenu::actionForBookmark(bookmark);
    }
    else
    {
        KBookmarkAction *action = new KBookmarkAction(bookmark, owner(), this);
        connect(action, SIGNAL(hovered()), this, SLOT(actionHovered()));
        return action;
    }
}


void BookmarkMenu::refill()
{
    clear();
    fillBookmarks();

    if (parentMenu()->actions().count() > 0)
        parentMenu()->addSeparator();

    if (isRoot())
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

    if (!group.first().isNull())
    {
        KBookmark bookmark = group.first();

        while (bookmark.isGroup() || bookmark.isSeparator())
        {
            bookmark = group.next(bookmark);
        }

        if (!bookmark.isNull())
        {
            action = new KAction(KIcon("tab-new"), i18n("Open Folder in Tabs"), this);
            action->setHelpText(i18n("Open all bookmarks in this folder as new tabs."));
            connect(action, SIGNAL(triggered(bool)), this, SLOT(slotOpenFolderInTabs()));
            parentMenu()->addAction(action);
        }
    }
}


void BookmarkMenu::actionHovered()
{
    KBookmarkActionInterface* action = dynamic_cast<KBookmarkActionInterface *>(sender());
    if (action)
        Application::instance()->mainWindow()->notifyMessage(action->bookmark().url().url());
}


// ------------------------------------------------------------------------------------------------------

#include <QActionEvent>

BookmarkToolBar::BookmarkToolBar( const QString &objectName,
                                  QMainWindow *parentWindow,
                                  Qt::ToolBarArea area,
                                  bool newLine,
                                  bool isMainToolBar,
                                  bool readConfig
                                )
    : KToolBar(objectName, parentWindow, area, newLine, isMainToolBar, readConfig)
    , m_filled(false)
    , m_currentMenu(0)
    , m_dragAction(0)
    , m_dropAction(0)
{
    connect(Application::bookmarkProvider()->bookmarkManager(), SIGNAL(changed(QString, QString)), this, SLOT(hideMenu()));
    setAcceptDrops(true);
}


BookmarkToolBar::~BookmarkToolBar()
{
}


void BookmarkToolBar::setVisible(bool visible)
{
    if (visible && !m_filled)
    {
        m_filled = true;
        Application::bookmarkProvider()->fillBookmarkBar(this);
    }
    KToolBar::setVisible(visible);
}


void BookmarkToolBar::menuDisplayed()
{
    qApp->installEventFilter(this);
    m_currentMenu = qobject_cast<KMenu*>(sender());
}


void BookmarkToolBar::menuHidden()
{
    qApp->removeEventFilter(this);
    m_currentMenu = 0;
}


void BookmarkToolBar::hideMenu()
{
    if(m_currentMenu)
        m_currentMenu->hide();
}


bool BookmarkToolBar::eventFilter(QObject *watched, QEvent *event)
{

    if (m_currentMenu && m_currentMenu->isVisible())
    {
        // To switch root folders as in a menubar
        KBookmarkActionMenu* act = dynamic_cast<KBookmarkActionMenu *>(this->actionAt(this->mapFromGlobal(QCursor::pos())));
        if (event->type() == QEvent::MouseMove && act && m_currentMenu && act->menu() != m_currentMenu)
        {
            m_currentMenu->hide();
            QPoint pos = mapToGlobal(widgetForAction(act)->pos());
            act->menu()->popup(QPoint(pos.x(), pos.y() + widgetForAction(act)->height()));
        }
    }
    else
    {
        // Drag handling
        if (event->type() == QEvent::MouseButtonPress)
        {
            QPoint pos = mapFromGlobal(QCursor::pos());
            KBookmarkActionInterface* action = dynamic_cast<KBookmarkActionInterface *>(actionAt(pos));

            if (action && !action->bookmark().isGroup())
            {
                m_dragAction = actionAt(pos);
                m_startDragPos = pos;
            }
        }
        else if (event->type() == QEvent::MouseMove)
        {
            int distance = (mapFromGlobal(QCursor::pos()) - m_startDragPos).manhattanLength();
            if (distance >= QApplication::startDragDistance())
            {
                startDrag();
            }
        }
    }
    return KToolBar::eventFilter(watched, event);
}


void BookmarkToolBar::actionHovered()
{
    KBookmarkActionInterface* action = dynamic_cast<KBookmarkActionInterface *>(sender());
    if (action)
        Application::instance()->mainWindow()->notifyMessage(action->bookmark().url().url());
}


void BookmarkToolBar::actionEvent(QActionEvent *event)
{
    KToolBar::actionEvent(event);

    QWidget *widget = widgetForAction(event->action());
    if (!widget || event->action() == m_dropAction)
        return;

    if (event->type() == QEvent::ActionAdded)
    {
        widget->installEventFilter(this);
    }
    else if (event->type() == QEvent::ActionRemoved)
    {
        widget->removeEventFilter(this);
    }
    else if (event->type() == QEvent::ParentChange)
    {
        widget->removeEventFilter(this);
    }
}


void BookmarkToolBar::startDrag()
{
    KBookmarkActionInterface *action = dynamic_cast<KBookmarkActionInterface *>(m_dragAction);
    if (action && !action->bookmark().isGroup())
    {
        QMimeData *mimeData = new QMimeData;

        QByteArray address = action->bookmark().address().toLatin1();
        mimeData->setData("application/rekonq-bookmark", address);
        action->bookmark().populateMimeData(mimeData);

        QDrag *drag = new QDrag(this);
        drag->setMimeData(mimeData);
        drag->setPixmap(KIcon(action->bookmark().icon()).pixmap(24, 24));

        drag->start(Qt::MoveAction);
        connect(drag, SIGNAL(destroyed()), this, SLOT(dragDestroyed()));
    }
}


void BookmarkToolBar::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat("application/rekonq-bookmark"))
    {
        QByteArray addresses = event->mimeData()->data("application/rekonq-bookmark");
        KBookmark bookmark = Application::bookmarkProvider()->bookmarkManager()->findByAddress(QString::fromLatin1(addresses.data()));

        if (!bookmark.isNull())
        {
            QFrame* dropIndicatorWidget = new QFrame(this);
            dropIndicatorWidget->setFrameShape(QFrame::VLine);
            m_dropAction = insertWidget(actionAt(event->pos()), dropIndicatorWidget);

            event->accept();
        }
    }

    KToolBar::dragEnterEvent(event);
}


void BookmarkToolBar::dragMoveEvent(QDragMoveEvent *event)
{
    if (event->mimeData()->hasFormat("application/rekonq-bookmark"))
    {
        QByteArray addresses = event->mimeData()->data("application/rekonq-bookmark");
        KBookmark bookmark = Application::bookmarkProvider()->bookmarkManager()->findByAddress(QString::fromLatin1(addresses.data()));
        QAction *overAction = actionAt(event->pos());
        KBookmarkActionInterface *overActionBK = dynamic_cast<KBookmarkActionInterface*>(overAction);
        QWidget *widgetAction = widgetForAction(overAction);

        if (!bookmark.isNull() && overAction != m_dropAction && overActionBK && widgetAction && m_dropAction)
        {
            removeAction(m_dropAction);

            if ((event->pos().x() - widgetAction->pos().x()) > (widgetAction->width() / 2))
            {
                if (actions().count() >  actions().indexOf(overAction) + 1)
                {
                    insertAction(actions().at(actions().indexOf(overAction) + 1), m_dropAction);
                }
                else
                {
                    addAction(m_dropAction);
                }
            }
            else
            {
                insertAction(overAction, m_dropAction);
            }

            event->accept();
        }
    }

    KToolBar::dragMoveEvent(event);
}


void BookmarkToolBar::dragLeaveEvent(QDragLeaveEvent *event)
{
    delete m_dropAction;
    m_dropAction = 0;
    event->accept();
    KToolBar::dragLeaveEvent(event);
}


void BookmarkToolBar::dropEvent(QDropEvent *event)
{
    if (event->mimeData()->hasFormat("application/rekonq-bookmark"))
    {
        QByteArray addresses = event->mimeData()->data("application/rekonq-bookmark");
        KBookmark bookmark = Application::bookmarkProvider()->bookmarkManager()->findByAddress(QString::fromLatin1(addresses.data()));

        QAction *destAction = actionAt(event->pos());
        if (destAction && destAction == m_dropAction)
        {
            if (actions().indexOf(m_dropAction) > 0)
            {
                destAction = actions().at(actions().indexOf(m_dropAction) - 1);
            }
            else
            {
                destAction = actions().at(1);
            }
        }

        KBookmarkActionInterface *destBookmarkAction = dynamic_cast<KBookmarkActionInterface *>(destAction);
        QWidget *widgetAction = widgetForAction(destAction);

        if (!bookmark.isNull() && destBookmarkAction && !destBookmarkAction->bookmark().isNull()
            && widgetAction && bookmark.address() != destBookmarkAction->bookmark().address())
        {
            KBookmarkGroup root = Application::bookmarkProvider()->rootGroup();
            KBookmark destBookmark = destBookmarkAction->bookmark();
            // To fix an issue with panel's drags
            root.deleteBookmark(bookmark);

            if ((event->pos().x() - widgetAction->pos().x()) > (widgetAction->width() / 2))
            {
                root.moveBookmark(bookmark, destBookmark);
            }
            else
            {
                root.moveBookmark(bookmark, destBookmark.parentGroup().previous(destBookmark));
            }

            Application::bookmarkProvider()->bookmarkManager()->emitChanged();
            event->accept();
        }
    }

    KToolBar::dropEvent(event);
}


void BookmarkToolBar::dragDestroyed()
{
    // A workaround to get rid of the checked state of the dragged action
    if (m_dragAction)
    {
        m_dragAction->setVisible(false);
        m_dragAction->setVisible(true);
        m_dragAction = 0;
    }
    delete m_dropAction;
    m_dropAction = 0;
}
