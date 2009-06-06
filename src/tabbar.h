/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2007-2008 Trolltech ASA. All rights reserved
* Copyright (C) 2008-2009 by Andrea Diamantini <adjam7 at gmail dot com>
* Copyright (C) 2009 by Paweł Prażak <pawelprazak at gmail dot com>
*
*
* This program is free software; you can redistribute it
* and/or modify it under the terms of the GNU General
* Public License as published by the Free Software Foundation;
* either version 3, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* ============================================================ */


#ifndef TABBAR_H
#define TABBAR_H

// KDE Includes
#include <KTabBar>

// Forward Declarations
class QPoint;

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
    void cloneTab(int index);
    void closeTab(int index);
    void closeOtherTabs(int index);
    void reloadTab(int index);
    void reloadAllTabs();

protected:
    /**
     * Added to fix tab dimension
     */
    virtual QSize tabSizeHint(int index) const;

private slots:
    void cloneTab();
    void closeTab();
    void closeOtherTabs();
    void reloadTab();
    void contextMenuRequested(const QPoint &position);

private:
    friend class MainView;

    QWidget *m_parent;

    /**
     * the index in which we are seeing a Context menu
     */
    int m_actualIndex;
};

#endif
