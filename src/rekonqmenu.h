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



#ifndef REKONQ_MENU_H
#define REKONQ_MENU_H


// Rekonq Includes
#include "rekonq_defines.h"

// KDE Includes
#include <KMenu>


/**
 * Menu shown inside rekonq window.
 * Inspired by Dolphin solution.
 *
 */
class REKONQ_TESTS_EXPORT RekonqMenu : public KMenu
{
    Q_OBJECT

public:
    RekonqMenu(QWidget *parent);

    void setButtonWidget(QWidget *);

protected:
    virtual void showEvent(QShowEvent* event);

private:
    QWidget *m_button;
};

#endif // REKONQ_MENU_H
