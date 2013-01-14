/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2003 by Andrea Diamantini <adjam7 at gmail dot com>
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


#include "maintoolbar.h"
#include "maintoolbar.moc"


#include <KAction>
#include <KActionCollection>
#include <KMenu>

#include <QPoint>


MainToolBar::MainToolBar(QWidget *parent)
    : KToolBar (parent, true, true)
{
    setContextMenuPolicy(Qt::CustomContextMenu);

    connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showCustomContextMenu(QPoint)));
}


void MainToolBar::showCustomContextMenu(QPoint p)
{
    KMenu menu(this);

    QList<KActionCollection *> lac = KActionCollection::allCollections();

    int lac_count = lac.count();
    for (int i = lac_count - 1; i >= 0; i--)
    {
        KActionCollection *ac = lac.at(i);

        QAction *a = ac->action("show_bookmarks_toolbar");
        if (a)
        {
            menu.addAction(a);
        }

        QAction *b = ac->action("configure_main_toolbar");
        if (b)
        {
            menu.addAction(b);
        }
    }

    // finally launch the menu...
    menu.exec(mapToGlobal(p));
}
