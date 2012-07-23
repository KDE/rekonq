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


#include "tabbar.h"
#include "tabbar.moc"

#include "tabwindow.h"

#include <KDebug>
#include <KAcceleratorManager>
#include <KAction>
#include <KLocalizedString>
#include <KMenu>


TabBar::TabBar(QWidget *parent)
    : KTabBar(parent)
{
    setElideMode(Qt::ElideRight);

    setTabsClosable(true);
    setMovable(true);
    setAcceptDrops(true);
    
    // avoid ambiguos shortcuts. See BUG:275858
    KAcceleratorManager::setNoAccel(this);

    // context menu(s)
    setContextMenuPolicy(Qt::CustomContextMenu);

    connect(this, SIGNAL(contextMenu(int, QPoint)), this, SLOT(contextMenu(int, QPoint)));
    connect(this, SIGNAL(emptyAreaContextMenu(QPoint)), this, SLOT(emptyAreaContextMenu(QPoint)));
}


QSize TabBar::tabSizeHint(int index) const
{
    Q_UNUSED(index);

    QWidget* p = qobject_cast<QWidget *>(parent());

    int maxTabBarWidth = p->size().width();

    int baseTabWidth = maxTabBarWidth / genericTabNumber;

    int minTabWidth =  p->sizeHint().width() / genericTabNumber;

    int w = baseTabWidth;
    if (count() >= genericTabNumber)
    {
        w = minTabWidth;
    }

    int h = size().height();

    QSize ts = QSize(w, h);
    return ts;
}


void TabBar::cloneTab()
{
    KAction *a = qobject_cast<KAction *>(sender());
    if (a)
    {
        int index = a->data().toInt();
        emit cloneTab(index);
    }
}


void TabBar::closeTab()
{
    KAction *a = qobject_cast<KAction *>(sender());
    if (a)
    {
        int index = a->data().toInt();
        emit closeTab(index);
    }
}


void TabBar::closeOtherTabs()
{
    KAction *a = qobject_cast<KAction *>(sender());
    if (a)
    {
        int index = a->data().toInt();
        emit closeOtherTabs(index);
    }
}


void TabBar::reloadTab()
{
    KAction *a = qobject_cast<KAction *>(sender());
    if (a)
    {
        int index = a->data().toInt();
        emit reloadTab(index);
    }
}


void TabBar::detachTab()
{
    KAction *a = qobject_cast<KAction *>(sender());
    if (a)
    {
        int index = a->data().toInt();
        emit detachTab(index);
    }
}


void TabBar::reopenLastClosedTab()
{
    KAction *a = qobject_cast<KAction *>(sender());
    if (a)
    {
        int index = a->data().toInt();
        emit restoreClosedTab(index);
    }
}


void TabBar::contextMenu(int tab, const QPoint &pos)
{
    TabWindow *w = qobject_cast<TabWindow *>(parent());
    
    KAction *a;
    
    KMenu menu;

    a = new KAction(KIcon("tab-new"), i18n("New &Tab"), this);
    connect(a, SIGNAL(triggered(bool)), w, SLOT(newCleanTab()));
    menu.addAction(a);

    menu.addSeparator();    // ----------------------------------------------------------------

    a = new KAction(KIcon("tab-duplicate"), i18n("Clone"), this);
    a->setData(tab);
    connect(a, SIGNAL(triggered(bool)), this, SLOT(cloneTab()));
    menu.addAction(a);

    a = new KAction(KIcon("view-refresh"), i18n("Reload"), this);
    connect(a, SIGNAL(triggered(bool)), this, SLOT(reloadTab()));
    a->setData(tab);
    menu.addAction(a);

    if (count() > 1)
    {
        a = new KAction(KIcon("tab-detach"), i18n("Detach"), this);
        connect(a, SIGNAL(triggered(bool)), this, SLOT(detachTab()));
        a->setData(tab);
        menu.addAction(a);
    }

    menu.addSeparator();    // ----------------------------------------------------------------

    a = new KAction(KIcon("tab-close"), i18n("&Close"), this);
    a->setData(tab);
    connect(a, SIGNAL(triggered(bool)), this, SLOT(closeTab()));
    menu.addAction(a);

    if (count() > 1)
    {
        a = new KAction(KIcon("tab-close-other"), i18n("Close &Other Tabs"), this);
        connect(a, SIGNAL(triggered(bool)), this, SLOT(closeOtherTabs()));
        a->setData(tab);
        menu.addAction(a);
    }
    
    menu.addSeparator();


    a = new KAction(KIcon("tab-new"), i18n("Open Last Closed Tab"), this);
    a->setData(0);  // last closed tab has index 0!
    connect(a, SIGNAL(triggered(bool)), this, SLOT(reopenLastClosedTab()));
    menu.addAction(a);

    if (count() > 1)
    {
        a = new KAction(KIcon("bookmark-new"), i18n("Bookmarks all tabs"), this);
        menu.addAction(a);
    }
    else
    {
        a = new KAction(KIcon("bookmark-new"), i18n("Bookmark"), this);
        menu.addAction(a);
    }
    
    menu.exec(pos);
}


void TabBar::emptyAreaContextMenu(const QPoint &pos)
{
    TabWindow *w = qobject_cast<TabWindow *>(parent());

    KAction *a;

    KMenu menu;

    a = new KAction(KIcon("tab-new"), i18n("New &Tab"), this);
    connect(a, SIGNAL(triggered(bool)), w, SLOT(newCleanTab()));
    menu.addAction(a);

    a = new KAction(KIcon("tab-new"), i18n("Open Last Closed Tab"), this);
    a->setData(0);  // last closed tab has index 0!
    menu.addAction(a);

    if (count() > 1)
    {
        a = new KAction(KIcon("bookmark-new"), i18n("Bookmarks all tabs"), this);
        menu.addAction(a);
    }
    else
    {
        a = new KAction(KIcon("bookmark-new"), i18n("Bookmark"), this);
        menu.addAction(a);
    }

    menu.exec(pos);
}
