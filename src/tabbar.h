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



#ifndef TABBAR_H
#define TABBAR_H

// KDE Includes
#include <KTabBar>

// Qt Includes
#include <QShortcut>

/**
 * Tab bar with a few more features such as 
 * a context menu and shortcuts
 *
 */

class TabBar : public KTabBar
{
    Q_OBJECT


public:
    TabBar(QWidget *parent = 0);
    ~TabBar();

signals:
    void newTab();
    void cloneTab(int index);
    void closeTab(int index);
    void closeOtherTabs(int index);
    void reloadTab(int index);
    void reloadAllTabs();
    void tabMoveRequested(int fromIndex, int toIndex);

protected:
    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);

    /**
     * Added to fix tab dimension
     *
     */
    virtual QSize tabSizeHint (int index) const;

private slots:
    void selectTabAction();
    void cloneTab();
    void closeTab();
    void closeOtherTabs();
    void reloadTab();
    void contextMenuRequested(const QPoint &position);

private:

    QList<QShortcut*> m_tabShortcuts;
    friend class MainView;

    QWidget *m_parent;
    QPoint m_dragStartPos;
    int m_dragCurrentIndex;

    /**
     * the index in which we are seeing a Context menu
     */
    int m_actualIndex;
};

#endif
