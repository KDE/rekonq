/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2010-2011 by Pierre Rossi <pierre dot rossi at gmail dot com>
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

#ifndef MESSAGEBAR_H
#define MESSAGEBAR_H

// Rekonq Includes
#include "rekonq_defines.h"
#include "notificationbar.h"

// Qt Includes
#include <QMessageBox>

// Forward Declarations
class QLabel;
class QPushButton;


class REKONQ_TESTS_EXPORT MessageBar : public NotificationBar
{
    Q_OBJECT

    Q_FLAGS(StandardButtons)

public:

    enum StandardButton
    {
        NoButton = 0x00000000,
        Ok       = 0x00000001,
        Cancel   = 0x00000002,
        Yes      = 0x00000004,
        No       = 0x00000008,
        Continue = 0x00000010
    };

    Q_DECLARE_FLAGS(StandardButtons, StandardButton)

    explicit MessageBar(const QString & message, QWidget *parent
                        , QMessageBox::Icon icon = QMessageBox::NoIcon
                                                   , StandardButtons buttons = NoButton);
    ~MessageBar();

Q_SIGNALS:
    void accepted();
    void rejected();

private:
    QLabel *m_icon;
    QLabel *m_text;
    QList<QPushButton *> m_buttons;

};

Q_DECLARE_OPERATORS_FOR_FLAGS(MessageBar::StandardButtons)

#endif // MESSAGEBAR_H
