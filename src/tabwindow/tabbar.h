/***************************************************************************
 *   Copyright (C) 2012 by Andrea Diamantini <adjam7@gmail.com>                            *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/


#ifndef TAB_BAR
#define TAB_BAR


// KDE Includes
#include <KTabBar>


class TabBar : public KTabBar
{
    Q_OBJECT

public:
    TabBar(QWidget *parent);

    static const int genericTabNumber = 6;

protected:
    virtual QSize tabSizeHint(int index) const;

Q_SIGNALS:
    void cloneTab(int);
    void closeTab(int);
    void closeOtherTabs(int);
    void reloadTab(int);
    void detachTab(int);
    void restoreClosedTab(int);

private Q_SLOTS:
    void cloneTab();
    void closeTab();
    void closeOtherTabs();
    void reloadTab();
    void detachTab();
    void reopenLastClosedTab();

    void contextMenu(int, const QPoint &);
    void emptyAreaContextMenu(const QPoint &);
};

#endif // TAB_BAR
