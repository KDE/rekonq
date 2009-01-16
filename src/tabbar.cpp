/* ============================================================
 *
 * This file is a part of the rekonq project
 *
 * Copyright (C) 2007-2008 Trolltech ASA. All rights reserved
 * Copyright (C) 2008 by Andrea Diamantini <adjam7 at gmail dot com>
 *
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

//Self Includes
#include "tabbar.h"
#include "tabbar.moc"

// Local Includes
#include "browserapplication.h"
#include "mainwindow.h"
#include "history.h"
#include "urlbar.h"
#include "webview.h"

// KDE Includes
#include <KShortcut>
#include <KStandardShortcut>
#include <KMessageBox>
#include <KAction>

// Qt Includes
#include <QtGui>
#include <QDebug>


TabBar::TabBar(QWidget *parent)
    : KTabBar(parent)
{
    setElideMode(Qt::ElideRight);
    setContextMenuPolicy(Qt::CustomContextMenu);
    setAcceptDrops(true);
    connect(this, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(contextMenuRequested(const QPoint &)));

    QString alt = QLatin1String("Alt+%1");
    for (int i = 1; i <= 10; ++i)
    {
        int key = i;
        if (key == 10)
        {
            key = 0;
        }
        QShortcut *shortCut = new QShortcut(alt.arg(key), this);
        m_tabShortcuts.append(shortCut);
        connect(shortCut, SIGNAL(activated()), this, SLOT(selectTabAction()));
    }
}


TabBar::~TabBar()
{
}


void TabBar::selectTabAction()
{
    if (QShortcut *shortCut = qobject_cast<QShortcut*>(sender()))
    {
        int index = m_tabShortcuts.indexOf(shortCut);
        if (index == 0)
            index = 10;
        setCurrentIndex(index);
    }
}


void TabBar::contextMenuRequested(const QPoint &position)
{
    // FIXME: use right actions
    KMenu menu;
    menu.addAction(i18n("New &Tab"), this, SIGNAL( newTab() ), QKeySequence::AddTab);
    int index = tabAt(position);
    if (-1 != index)
    {
        KAction *action = (KAction * ) menu.addAction(i18n("Clone Tab"), this, SLOT(cloneTab()));
        action->setData(index);

        menu.addSeparator();

        action = (KAction * ) menu.addAction(i18n("&Close Tab"), this, SLOT(closeTab()), QKeySequence::Close);
        action->setData(index);

        action = (KAction * ) menu.addAction(i18n("Close &Other Tabs"), this, SLOT(closeOtherTabs()));
        action->setData(index);

        menu.addSeparator();

        action = (KAction * ) menu.addAction(i18n("Reload Tab"), this, SLOT(reloadTab()), QKeySequence::Refresh);
        action->setData(index);
    } 
    else 
    {
        menu.addSeparator();
    }
    menu.addAction(i18n("Reload All Tabs"), this, SIGNAL(reloadAllTabs()));
    menu.exec(QCursor::pos());
}


void TabBar::cloneTab()
{
    if (KAction *action = qobject_cast<KAction*>(sender())) 
    {
        int index = action->data().toInt();
        emit cloneTab(index);
    }
}


void TabBar::closeTab()
{
    if (KAction *action = qobject_cast<KAction*>(sender())) 
    {
        int index = action->data().toInt();
        emit closeTab(index);
    }
}


void TabBar::closeOtherTabs()
{
    if (KAction *action = qobject_cast<KAction*>(sender()))
    {
        int index = action->data().toInt();
        emit closeOtherTabs(index);
    }
}


void TabBar::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
        m_dragStartPos = event->pos();
    QTabBar::mousePressEvent(event);
}


void TabBar::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() == Qt::LeftButton && (event->pos() - m_dragStartPos).manhattanLength() > QApplication::startDragDistance())
    {
        QDrag *drag = new QDrag(this);
        QMimeData *mimeData = new QMimeData;
        QList<QUrl> urls;
        int index = tabAt(event->pos());
        QUrl url = tabData(index).toUrl();
        urls.append(url);
        mimeData->setUrls(urls);
        mimeData->setText(tabText(index));
        mimeData->setData(QLatin1String("action"), "tab-reordering");
        drag->setMimeData(mimeData);
        drag->exec();
    }
    QTabBar::mouseMoveEvent(event);
}


void TabBar::dragEnterEvent(QDragEnterEvent *event)
{
    const QMimeData *mimeData = event->mimeData();
    QStringList formats = mimeData->formats();

    if (formats.contains(QLatin1String("action")) && (mimeData->data(QLatin1String("action")) == "tab-reordering"))
    {
        event->acceptProposedAction();
    }
    QTabBar::dragEnterEvent(event);
}


void TabBar::dropEvent(QDropEvent *event)
{
    int fromIndex = tabAt(m_dragStartPos);
    int toIndex = tabAt(event->pos());
    if (fromIndex != toIndex)
    {
        emit tabMoveRequested(fromIndex, toIndex);
        event->acceptProposedAction();
    }
    QTabBar::dropEvent(event);
}


void TabBar::reloadTab()
{
    if (KAction *action = qobject_cast<KAction*>(sender())) 
    {
        int index = action->data().toInt();
        emit reloadTab(index);
    }
}

