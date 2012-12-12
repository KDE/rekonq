/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2011 by Peter Penz <peter.penz19@gmail.com>
* Copyright (C) 2012 by Andrea Diamantini <adjam7 at gmail dot com>
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
#include "rekonqmenu.h"
#include "rekonqmenu.moc"

// Qt Includes
#include <QApplication>
#include <QDesktopWidget>
#include <QWidget>


RekonqMenu::RekonqMenu(QWidget *parent)
    : KMenu(parent)
{
}


void RekonqMenu::showEvent(QShowEvent* event)
{
    KMenu::showEvent(event);

    // Adjust the position of the menu to be shown within the
    // rekonq window to reduce the cases that sub-menus might overlap
    // the right screen border.
    QPoint position = pos();
    int y = position.y();

    int w = width();

    QWidget *parentWidget = qobject_cast<QWidget *>(parent());
    QPoint widgetGlobalPos = parentWidget->mapToGlobal(QPoint(0, 0));
    int pw = parentWidget->width();
    int px = widgetGlobalPos.x();

    QPoint newPosition = QPoint(px + pw - w, y);

    // Finally, move it there...
    move(newPosition);
}
