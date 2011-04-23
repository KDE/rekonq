/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2008-2011 by Andrea Diamantini <adjam7 at gmail dot com>
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
#include "iconmanager.h"
#include "bookmarkscontextmenu.h"
#include "mainwindow.h"
#include "application.h"
#include "bookmarkprovider.h"
#include "bookmarkowner.h"
#include "webtab.h"

// Qt Includes
#include <QtGui/QFrame>
#include <QtGui/QActionEvent>


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
    kDebug() << "Deleting BookmarkMenu.. See http://svn.reviewboard.kde.org/r/5606/ about.";
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
        action->setIcon(rApp->iconManager()->iconForUrl(KUrl(bookmark.url())));
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
            parentMenu()->addAction(rApp->bookmarkProvider()->bookmarkOwner()->createAction(group, BookmarkOwner::OPEN_FOLDER));
        }
    }
}


void BookmarkMenu::actionHovered()
{
    KBookmarkActionInterface* action = dynamic_cast<KBookmarkActionInterface *>(sender());
    if (action)
        rApp->mainWindow()->notifyMessage(action->bookmark().url().url());
}


// ------------------------------------------------------------------------------------------------------


BookmarkToolBar::BookmarkToolBar(KToolBar *toolBar, QObject *parent)
        : QObject(parent)
        , m_toolBar(toolBar)
        , m_currentMenu(0)
        , m_dragAction(0)
        , m_dropAction(0)
        , m_filled(false)
{
    toolBar->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(toolBar, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(contextMenu(const QPoint &)));
    connect(rApp->bookmarkProvider()->bookmarkManager(), SIGNAL(changed(QString, QString)), this, SLOT(hideMenu()));
    toolBar->setAcceptDrops(true);
    toolBar->installEventFilter(this);
    toolBar->setShortcutEnabled(false);

    if (toolBar->isVisible())
    {
        rApp->bookmarkProvider()->fillBookmarkBar(this);
        m_filled = true;
    }
}


KToolBar* BookmarkToolBar::toolBar()
{
    return m_toolBar;
}


void BookmarkToolBar::contextMenu(const QPoint &point)
{
    KBookmarkActionInterface *action = dynamic_cast<KBookmarkActionInterface*>(toolBar()->actionAt(point));
    KBookmark bookmark = rApp->bookmarkProvider()->bookmarkManager()->toolbar();
    bool nullAction = true;
    if (action)
    {
        bookmark = action->bookmark();
        nullAction = false;
    }

    BookmarksContextMenu menu(bookmark,
                              rApp->bookmarkProvider()->bookmarkManager(),
                              rApp->bookmarkProvider()->bookmarkOwner(),
                              nullAction);
    menu.exec(toolBar()->mapToGlobal(point));
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
    if (m_currentMenu)
        m_currentMenu->hide();
}


bool BookmarkToolBar::eventFilter(QObject *watched, QEvent *event)
{
    // To switch root folders as in a menubar
    if (m_currentMenu && m_currentMenu->isVisible() && event->type() == QEvent::MouseMove
            && !m_currentMenu->rect().contains(m_currentMenu->mapFromGlobal(QCursor::pos())))
    {
        KBookmarkActionMenu* act = dynamic_cast<KBookmarkActionMenu *>(toolBar()->actionAt(toolBar()->mapFromGlobal(QCursor::pos())));
        if (act && act->menu() != m_currentMenu)
        {
            m_currentMenu->hide();
            QPoint pos = toolBar()->mapToGlobal(toolBar()->widgetForAction(act)->pos());
            act->menu()->popup(QPoint(pos.x(), pos.y() + toolBar()->widgetForAction(act)->height()));
        }
        
        return QObject::eventFilter(watched, event);
    }
    
    switch (event->type())
    {
        case QEvent::Show:
        {
            if (!m_filled)
            {
                rApp->bookmarkProvider()->fillBookmarkBar(this);
                m_filled = true;
            }
        }
            break;

        case QEvent::ActionRemoved:
        {
            QActionEvent *actionEvent = static_cast<QActionEvent*>(event);
            if (actionEvent && actionEvent->action() != m_dropAction)
            {
                QWidget *widget = toolBar()->widgetForAction(actionEvent->action());
                if (widget)
                {
                    widget->removeEventFilter(this);
                }
            }
        }
            break;

        case QEvent::ParentChange:
        {
            QActionEvent *actionEvent = static_cast<QActionEvent*>(event);
            if (actionEvent && actionEvent->action() != m_dropAction)
            {
                QWidget *widget = toolBar()->widgetForAction(actionEvent->action());
                if (widget)
                {
                    widget->removeEventFilter(this);
                }
            }
        }
            break;

        case QEvent::DragEnter:
        {
            QDragEnterEvent *dragEvent = static_cast<QDragEnterEvent*>(event);
            if (dragEvent->mimeData()->hasFormat("application/rekonq-bookmark") || dragEvent->mimeData()->hasFormat("text/uri-list") || dragEvent->mimeData()->hasFormat("text/plain"))
            {
                QFrame* dropIndicatorWidget = new QFrame(toolBar());
                dropIndicatorWidget->setFrameShape(QFrame::VLine);
                m_dropAction = toolBar()->insertWidget(toolBar()->actionAt(dragEvent->pos()), dropIndicatorWidget);

                dragEvent->accept();
            }
        }
            break;
        
        case QEvent::DragMove:
        {
            QDragMoveEvent *dragEvent = static_cast<QDragMoveEvent*>(event);
            if (dragEvent->mimeData()->hasFormat("application/rekonq-bookmark") || dragEvent->mimeData()->hasFormat("text/uri-list") || dragEvent->mimeData()->hasFormat("text/plain"))
            {
                QAction *overAction = toolBar()->actionAt(dragEvent->pos());
                KBookmarkActionInterface *overActionBK = dynamic_cast<KBookmarkActionInterface*>(overAction);
                QWidget *widgetAction = toolBar()->widgetForAction(overAction);

                if (overAction != m_dropAction && overActionBK && widgetAction && m_dropAction)
                {
                    toolBar()->removeAction(m_dropAction);

                    if ((dragEvent->pos().x() - widgetAction->pos().x()) > (widgetAction->width() / 2))
                    {
                        if (toolBar()->actions().count() >  toolBar()->actions().indexOf(overAction) + 1)
                        {
                            toolBar()->insertAction(toolBar()->actions().at(toolBar()->actions().indexOf(overAction) + 1), m_dropAction);
                        }
                        else
                        {
                            toolBar()->addAction(m_dropAction);
                        }
                    }
                    else
                    {
                        toolBar()->insertAction(overAction, m_dropAction);
                    }

                    dragEvent->accept();
                }
            }
        }
            break;

        case QEvent::DragLeave:
        {
            QDragLeaveEvent *dragEvent = static_cast<QDragLeaveEvent*>(event);
            delete m_dropAction;
            m_dropAction = 0;
            dragEvent->accept();
        }
            break;

        case QEvent::Drop:
        {
            QDropEvent *dropEvent = static_cast<QDropEvent*>(event);
            KBookmark bookmark;
            KBookmarkGroup root = rApp->bookmarkProvider()->bookmarkManager()->toolbar();

            if (dropEvent->mimeData()->hasFormat("application/rekonq-bookmark"))
            {
                QByteArray addresses = dropEvent->mimeData()->data("application/rekonq-bookmark");
                bookmark =  rApp->bookmarkProvider()->bookmarkManager()->findByAddress(QString::fromLatin1(addresses.data()));
                if (bookmark.isNull())
                    return false;
            }
            else if (dropEvent->mimeData()->hasFormat("text/uri-list"))
            {
                kDebug() << "DROP is URL";
                QString url = dropEvent->mimeData()->urls().at(0).toString();
                QString title = url.contains( rApp->mainWindow()->currentTab()->url().url() )
                    ? rApp->mainWindow()->currentTab()->view()->title()
                    : url;
                bookmark = root.addBookmark(title, url);
            }
            else if (dropEvent->mimeData()->hasFormat("text/plain"))
            {
                kDebug() << "DROP is TEXT";
                QString url = dropEvent->mimeData()->text();
                KUrl u(url);
                if (u.isValid())
                {
                    QString title = url.contains( rApp->mainWindow()->currentTab()->url().url() )
                        ? rApp->mainWindow()->currentTab()->view()->title()
                        : url;
                    bookmark = root.addBookmark(title, url);
                }
            }
            else
            {
                return false;
            }

            QAction *destAction = toolBar()->actionAt(dropEvent->pos());
            if (destAction && destAction == m_dropAction)
            {
                if (toolBar()->actions().indexOf(m_dropAction) > 0)
                {
                    destAction = toolBar()->actions().at(toolBar()->actions().indexOf(m_dropAction) - 1);
                }
                else
                {
                    destAction = toolBar()->actions().at(1);
                }
            }

            if (destAction)
            {
                KBookmarkActionInterface *destBookmarkAction = dynamic_cast<KBookmarkActionInterface *>(destAction);
                QWidget *widgetAction = toolBar()->widgetForAction(destAction);

                if (destBookmarkAction && !destBookmarkAction->bookmark().isNull() && widgetAction
                        && bookmark.address() != destBookmarkAction->bookmark().address())
                {
                    KBookmark destBookmark = destBookmarkAction->bookmark();

                    if ((dropEvent->pos().x() - widgetAction->pos().x()) > (widgetAction->width() / 2))
                    {
                        root.moveBookmark(bookmark, destBookmark);
                    }
                    else
                    {
                        root.moveBookmark(bookmark, destBookmark.parentGroup().previous(destBookmark));
                    }

                    rApp->bookmarkProvider()->bookmarkManager()->emitChanged();
                }
            }
            else
            {
                root.deleteBookmark(bookmark);
                bookmark = root.addBookmark(bookmark);
                if (dropEvent->pos().x() < toolBar()->widgetForAction(toolBar()->actions().first())->pos().x())
                {
                    root.moveBookmark(bookmark, KBookmark());
                }

                rApp->bookmarkProvider()->bookmarkManager()->emitChanged();
            }
            dropEvent->accept();
        }
            break;

        default:
            break;
    }

    // These events need to be handled only for Bookmark actions and not the bar
    if (watched != toolBar())
    {
        switch (event->type())
        {
            case QEvent::MouseButtonPress: // drag handling
            {
                QPoint pos = toolBar()->mapFromGlobal(QCursor::pos());
                KBookmarkActionInterface *action = dynamic_cast<KBookmarkActionInterface *>(toolBar()->actionAt(pos));

                if (action)
                {
                    m_dragAction = toolBar()->actionAt(pos);
                    m_startDragPos = pos;

                    // The menu is displayed only when the mouse button is released
                    if (action->bookmark().isGroup())
                        return true;
                }
            }
                break;

            case QEvent::MouseMove:
            {
                int distance = (toolBar()->mapFromGlobal(QCursor::pos()) - m_startDragPos).manhattanLength();
                if (!m_currentMenu && distance >= QApplication::startDragDistance())
                {
                    startDrag();
                }
            }
                break;

            case QEvent::MouseButtonRelease:
            {
                int distance = (toolBar()->mapFromGlobal(QCursor::pos()) - m_startDragPos).manhattanLength();
                KBookmarkActionInterface *action = dynamic_cast<KBookmarkActionInterface *>(toolBar()->actionAt(m_startDragPos));

                if (action && action->bookmark().isGroup() && distance < QApplication::startDragDistance())
                {
                    KBookmarkActionMenu *menu = dynamic_cast<KBookmarkActionMenu *>(toolBar()->actionAt(m_startDragPos));
                    QPoint actionPos = toolBar()->mapToGlobal(toolBar()->widgetForAction(menu)->pos());
                    menu->menu()->popup(QPoint(actionPos.x(), actionPos.y() + toolBar()->widgetForAction(menu)->height()));
                }
            }
                break;

            default:
                break;
        }
    }

    return QObject::eventFilter(watched, event);
}


void BookmarkToolBar::actionHovered()
{
    KBookmarkActionInterface* action = dynamic_cast<KBookmarkActionInterface *>(sender());
    if (action)
        rApp->mainWindow()->notifyMessage(action->bookmark().url().url());
}


void BookmarkToolBar::startDrag()
{
    KBookmarkActionInterface *action = dynamic_cast<KBookmarkActionInterface *>(m_dragAction);
    if (action)
    {
        QMimeData *mimeData = new QMimeData;
        KBookmark bookmark = action->bookmark();

        QByteArray address = bookmark.address().toLatin1();
        mimeData->setData("application/rekonq-bookmark", address);
        bookmark.populateMimeData(mimeData);

        QDrag *drag = new QDrag(toolBar());
        drag->setMimeData(mimeData);

        if (bookmark.isGroup())
        {
            drag->setPixmap(KIcon(bookmark.icon()).pixmap(24, 24));
        }
        else
        {
            drag->setPixmap(rApp->iconManager()->iconForUrl(action->bookmark().url()).pixmap(24, 24));
        }

        drag->start(Qt::MoveAction);
        connect(drag, SIGNAL(destroyed()), this, SLOT(dragDestroyed()));
    }
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
