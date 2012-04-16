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


void RekonqMenu::setButtonWidget(QWidget *w)
{
    m_button = w;
}


void RekonqMenu::showEvent(QShowEvent* event)
{
    KMenu::showEvent(event);

    if (!m_button)
        return;
    
    // Adjust the position of the menu to be shown within the
    // rekonq window to reduce the cases that sub-menus might overlap
    // the right screen border.
    QPoint pos;
    if (layoutDirection() == Qt::RightToLeft)
    {
        pos = m_button->mapToGlobal(QPoint(0, m_button->height()));
    }
    else
    {
        pos = m_button->mapToGlobal(QPoint(m_button->width(), m_button->height()));
        pos.rx() -= width();
    }

    // Assure that the menu is not shown outside the screen boundaries and
    // that it does not overlap with the parent button.
    const QRect screen = QApplication::desktop()->screenGeometry(QCursor::pos());
    if (pos.x() < screen.x())
    {
        pos.rx() = screen.x();
    }
    else
    {
        if (pos.x() + width() > screen.x() + screen.width())
        {
            pos.rx() = screen.x() + screen.width() - width();
        }
    }

    if (pos.y() < screen.y())
    {
        pos.ry() = screen.y();
    }
    else
    {
        if (pos.y() + height() > screen.y() + screen.height())
        {
            pos.ry() = m_button->mapToGlobal(QPoint(0, 0)).y() + height();
        }
    }

    move(pos);
}
