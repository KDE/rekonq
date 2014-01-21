/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2008-2014 by Andrea Diamantini <adjam7 at gmail dot com>
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
#include "bookmarkmanager.h"
#include "bookmarkowner.h"

#include "iconmanager.h"
#include "webwindow.h"

// KDE Includes
#include <KMenu>

// Qt Includes
#include <QFrame>
#include <QActionEvent>
#include <QApplication>


BookmarkToolBar::BookmarkToolBar(QWidget *parent)
    : KToolBar(parent, false, false)
    , m_currentMenu(0)
    , m_dragAction(0)
    , m_dropAction(0)
    , m_checkedAction(0)
    , m_filled(false)
{
    setContextMenuPolicy(Qt::CustomContextMenu);

    connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenu(QPoint)));
    connect(BookmarkManager::self()->manager(), SIGNAL(changed(QString,QString)), this, SLOT(hideMenu()));

    setAcceptDrops(true);
    installEventFilter(this);
    setShortcutEnabled(false);

    if (isVisible())
    {
        BookmarkManager::self()->fillBookmarkBar(this);
        m_filled = true;
    }
}


BookmarkToolBar::~BookmarkToolBar()
{
    clear();
}


void BookmarkToolBar::contextMenu(const QPoint &point)
{
    KBookmarkActionInterface *action = dynamic_cast<KBookmarkActionInterface*>(actionAt(point));
    KBookmark bookmark = BookmarkManager::self()->manager()->toolbar();
    bool nullAction = true;
    if (action)
    {
        bookmark = action->bookmark();
        nullAction = false;
    }

    BookmarksContextMenu menu(bookmark,
                              BookmarkManager::self()->manager(),
                              BookmarkManager::self()->owner(),
                              nullAction);

    menu.exec(mapToGlobal(point));
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
    if (m_currentMenu && m_currentMenu->isVisible()
            && !m_currentMenu->rect().contains(m_currentMenu->mapFromGlobal(QCursor::pos())))
    {
        // To switch root folders as in a menubar

        KBookmarkActionMenu* act = dynamic_cast<KBookmarkActionMenu *>(actionAt(mapFromGlobal(QCursor::pos())));

        if (event->type() == QEvent::MouseMove && act && act->menu() != m_currentMenu)
        {
            m_currentMenu->hide();
            QPoint pos = mapToGlobal(widgetForAction(act)->pos());
            act->menu()->popup(QPoint(pos.x(), pos.y() + widgetForAction(act)->height()));
        }
        else if (event->type() == QEvent::MouseButtonPress && act)
        {
            m_currentMenu->hide();
        }

        return QObject::eventFilter(watched, event);
    }

    switch (event->type())
    {
    case QEvent::Show:
    {
        if (!m_filled)
        {
            BookmarkManager::self()->fillBookmarkBar(this);
            m_filled = true;
        }
    }
    break;

    case QEvent::ActionRemoved:
    {
        QActionEvent *actionEvent = static_cast<QActionEvent*>(event);
        if (actionEvent && actionEvent->action() != m_dropAction)
        {
            QWidget *widget = widgetForAction(actionEvent->action());
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
            QWidget *widget = widgetForAction(actionEvent->action());
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
        if (dragEvent->mimeData()->hasFormat(BookmarkManager::bookmark_mime_type())
                || dragEvent->mimeData()->hasFormat("text/uri-list")
                || dragEvent->mimeData()->hasFormat("text/plain"))
        {
            QFrame* dropIndicatorWidget = new QFrame(this);
            dropIndicatorWidget->setFrameShape(QFrame::VLine);
            m_dropAction = insertWidget(actionAt(dragEvent->pos()), dropIndicatorWidget);

            dragEvent->accept();
        }
    }
    break;

    case QEvent::DragLeave:
    {
        QDragLeaveEvent *dragEvent = static_cast<QDragLeaveEvent*>(event);

        if (m_checkedAction)
        {
            m_checkedAction->setCheckable(false);
            m_checkedAction->setChecked(false);
        }

        delete m_dropAction;
        m_dropAction = 0;
        dragEvent->accept();
    }
    break;

    case QEvent::DragMove:
    {
        QDragMoveEvent *dragEvent = static_cast<QDragMoveEvent*>(event);
        if (dragEvent->mimeData()->hasFormat(BookmarkManager::bookmark_mime_type())
                || dragEvent->mimeData()->hasFormat("text/uri-list")
                || dragEvent->mimeData()->hasFormat("text/plain"))
        {
            QAction *overAction = actionAt(dragEvent->pos());
            KBookmarkActionInterface *overActionBK = dynamic_cast<KBookmarkActionInterface*>(overAction);
            QWidget *widgetAction = widgetForAction(overAction);

            if (overAction != m_dropAction && overActionBK && widgetAction && m_dropAction)
            {
                removeAction(m_dropAction);
                if (m_checkedAction)
                {
                    m_checkedAction->setCheckable(false);
                    m_checkedAction->setChecked(false);
                }

                if (!overActionBK->bookmark().isGroup())
                {
                    if ((dragEvent->pos().x() - widgetAction->pos().x()) > (widgetAction->width() / 2))
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
                }
                else
                {
                    if ((dragEvent->pos().x() - widgetAction->pos().x()) >= (widgetAction->width() * 0.75))
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
                    else if ((dragEvent->pos().x() - widgetAction->pos().x()) <= (widgetAction->width() * 0.25))
                    {
                        insertAction(overAction, m_dropAction);
                    }
                    else
                    {
                        overAction->setCheckable(true);
                        overAction->setChecked(true);
                        m_checkedAction = overAction;
                    }
                }

                dragEvent->accept();
            }
        }
    }
    break;


    case QEvent::Drop:
    {
        QDropEvent *dropEvent = static_cast<QDropEvent*>(event);
        KBookmark bookmark;
        KBookmarkGroup root = BookmarkManager::self()->manager()->toolbar();

        if (m_checkedAction)
        {
            m_checkedAction->setCheckable(false);
            m_checkedAction->setChecked(false);
        }

        if (dropEvent->mimeData()->hasFormat(BookmarkManager::bookmark_mime_type()))
        {
            QByteArray addresses = dropEvent->mimeData()->data(BookmarkManager::bookmark_mime_type());
            bookmark =  BookmarkManager::self()->findByAddress(QString::fromLatin1(addresses.data()));
            if (bookmark.isNull())
                return false;
        }
        else if (dropEvent->mimeData()->hasFormat("text/uri-list"))
        {
            // DROP is URL
            QString url = dropEvent->mimeData()->urls().at(0).toString();
            WebWindow *w = qobject_cast<WebWindow *>(parent());
            QString title = url.contains(w->url().url())
                            ? w->title()
                            : url;
            bookmark = root.addBookmark(title, url);
        }
        else if (dropEvent->mimeData()->hasFormat("text/plain"))
        {
            // DROP is TEXT
            QString url = dropEvent->mimeData()->text();
            KUrl u(url);
            if (u.isValid())
            {
                WebWindow *w = qobject_cast<WebWindow *>(parent());
                QString title = url.contains(w->url().url())
                                ? w->title()
                                : url;
                bookmark = root.addBookmark(title, url);
            }
        }
        else
        {
            return false;
        }

        QAction *destAction = actionAt(dropEvent->pos());
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

        if (destAction)
        {
            KBookmarkActionInterface *destBookmarkAction = dynamic_cast<KBookmarkActionInterface *>(destAction);
            QWidget *widgetAction = widgetForAction(destAction);

            if (destBookmarkAction && !destBookmarkAction->bookmark().isNull() && widgetAction
                    && bookmark.address() != destBookmarkAction->bookmark().address())
            {
                KBookmark destBookmark = destBookmarkAction->bookmark();

                if (!destBookmark.isGroup())
                {
                    if ((dropEvent->pos().x() - widgetAction->pos().x()) >= (widgetAction->width() / 2))
                    {
                        root.moveBookmark(bookmark, destBookmark);
                    }
                    else
                    {
                        root.moveBookmark(bookmark, destBookmark.parentGroup().previous(destBookmark));
                    }
                }
                else
                {
                    if ((dropEvent->pos().x() - widgetAction->pos().x()) >= (widgetAction->width() * 0.75))
                    {
                        root.moveBookmark(bookmark, destBookmark);
                    }
                    else if ((dropEvent->pos().x() - widgetAction->pos().x()) <= (widgetAction->width() * 0.25))
                    {
                        root.moveBookmark(bookmark, destBookmark.parentGroup().previous(destBookmark));
                    }
                    else
                    {
                        destBookmark.toGroup().addBookmark(bookmark);
                    }
                }


                BookmarkManager::self()->emitChanged();
            }
        }
        else
        {
            root.deleteBookmark(bookmark);
            bookmark = root.addBookmark(bookmark);
            if (dropEvent->pos().x() < widgetForAction(actions().first())->pos().x())
            {
                root.moveBookmark(bookmark, KBookmark());
            }

            BookmarkManager::self()->emitChanged();
        }
        dropEvent->accept();
    }
    break;

    default:
        break;
    }

    QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);

    // These events need to be handled only for Bookmark actions and not the bar
    if (watched != this && mouseEvent)
    {
        switch (event->type())
        {
        case QEvent::MouseButtonPress: // drag handling
        {
            QPoint pos = mapFromGlobal(QCursor::pos());
            KBookmarkActionInterface *action = dynamic_cast<KBookmarkActionInterface *>(actionAt(pos));

            if (action && mouseEvent->button() != Qt::MidButton)
            {
                m_dragAction = actionAt(pos);
                m_startDragPos = pos;

                // The menu is displayed only when the mouse button is released
                if (action->bookmark().isGroup())
                    return true;
            }
        }
        break;

        case QEvent::MouseMove:
        {
            int distance = (mapFromGlobal(QCursor::pos()) - m_startDragPos).manhattanLength();
            if (!m_currentMenu && distance >= QApplication::startDragDistance())
            {
                startDrag();
            }
        }
        break;

        case QEvent::MouseButtonRelease:
        {
            QPoint destPos = mapFromGlobal(QCursor::pos());
            int distance = (destPos - m_startDragPos).manhattanLength();
            KBookmarkActionInterface *action = dynamic_cast<KBookmarkActionInterface *>(actionAt(destPos));

            if (action)
            {
                if (action->bookmark().isGroup())
                {
                    if (mouseEvent->button() == Qt::MidButton)
                    {
                        BookmarkManager::self()->owner()->loadBookmarkFolder(action->bookmark());
                    }
                    else if (distance < QApplication::startDragDistance())
                    {
                        KBookmarkActionMenu *menu = dynamic_cast<KBookmarkActionMenu *>(actionAt(m_startDragPos));
                        QPoint actionPos = mapToGlobal(widgetForAction(menu)->pos());
                        menu->menu()->popup(QPoint(actionPos.x(), actionPos.y() + widgetForAction(menu)->height()));
                    }
                }
                else
                {
                    if (!action->bookmark().isNull() && !action->bookmark().isSeparator())
                    {
                        if (mouseEvent->button() == Qt::MidButton)
                        {
                            BookmarkManager::self()->owner()->loadBookmarkInNewTab(action->bookmark());
                        }
                    }
                }
            }
        }
        break;

        default:
            break;
        }
    }

    return QObject::eventFilter(watched, event);
}


void BookmarkToolBar::startDrag()
{
    KBookmarkActionInterface *action = dynamic_cast<KBookmarkActionInterface *>(m_dragAction);
    if (action)
    {
        QMimeData *mimeData = new QMimeData;
        KBookmark bookmark = action->bookmark();

        QByteArray address = bookmark.address().toLatin1();
        mimeData->setData(BookmarkManager::bookmark_mime_type(), address);
        bookmark.populateMimeData(mimeData);

        QDrag *drag = new QDrag(this);
        drag->setMimeData(mimeData);

        if (bookmark.isGroup())
        {
            drag->setPixmap(KIcon(bookmark.icon()).pixmap(24, 24));
        }
        else
        {
            drag->setPixmap(IconManager::self()->iconForUrl(action->bookmark().url()).pixmap(24, 24));
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
