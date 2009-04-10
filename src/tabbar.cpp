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
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* ============================================================ */

//Self Includes
#include "tabbar.h"
#include "tabbar.moc"

// Local Includes
#include "application.h"
#include "mainwindow.h"
#include "history.h"
#include "urlbar.h"
#include "webview.h"

// KDE Includes
#include <KShortcut>
#include <KStandardShortcut>
#include <KMessageBox>
#include <KAction>
#include <KDebug>
#include <KGlobalSettings>

// Qt Includes
#include <QtGui>


TabBar::TabBar(QWidget *parent)
        : KTabBar(parent)
        , m_parent(parent)
{
    setElideMode(Qt::ElideRight);
    setContextMenuPolicy(Qt::CustomContextMenu);
    setAcceptDrops(true);
    connect(this, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(contextMenuRequested(const QPoint &)));

    QFont standardFont = KGlobalSettings::generalFont();
    QString fontFamily = standardFont.family();
    int dim = standardFont.pointSize();
    setFont(QFont(fontFamily, dim - 1));
}


TabBar::~TabBar()
{
}


QSize TabBar::tabSizeHint(int index) const
{
    QSize s = m_parent->sizeHint();
    int w;

    int n = count();

    if(n > 6)
    {
        w = s.width() / 5;
    }
    else
    {
        if (n > 3)
        {
            w = s.width() / 4;
        }
        else
        {
            w = s.width() / 3;
        }
    }
    int h = KTabBar::tabSizeHint(index).height();

    QSize ts = QSize(w, h);
    return ts;
}


void TabBar::contextMenuRequested(const QPoint &position)
{
    KMenu menu;
    menu.addAction(i18n("New &Tab"), this, SIGNAL(newTab()), QKeySequence::AddTab);
    int index = tabAt(position);
    if (-1 != index)
    {
        m_actualIndex = index;

        KAction *action = (KAction *) menu.addAction(i18n("Clone Tab"), this, SLOT(cloneTab()));
        menu.addSeparator();
        action = (KAction *) menu.addAction(i18n("&Close Tab"), this, SLOT(closeTab()), QKeySequence::Close);
        action = (KAction *) menu.addAction(i18n("Close &Other Tabs"), this, SLOT(closeOtherTabs()));
        menu.addSeparator();
        action = (KAction *) menu.addAction(i18n("Reload Tab"), this, SLOT(reloadTab()), QKeySequence::Refresh);
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
    emit cloneTab(m_actualIndex);
}


void TabBar::closeTab()
{
    emit closeTab(m_actualIndex);
}


void TabBar::closeOtherTabs()
{
    emit closeOtherTabs(m_actualIndex);
}


void TabBar::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_dragStartPos = event->pos();
    }
    KTabBar::mousePressEvent(event);
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
    KTabBar::mouseMoveEvent(event);
}


void TabBar::dragEnterEvent(QDragEnterEvent *event)
{
    const QMimeData *mimeData = event->mimeData();
    QStringList formats = mimeData->formats();

    if (formats.contains(QLatin1String("action")) && (mimeData->data(QLatin1String("action")) == "tab-reordering"))
    {
        event->acceptProposedAction();
    }
    KTabBar::dragEnterEvent(event);
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
    KTabBar::dropEvent(event);
}


void TabBar::reloadTab()
{
    emit reloadTab(m_actualIndex);
}
