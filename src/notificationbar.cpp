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


// Self includes
#include "notificationbar.h"
#include "notificationbar.moc"

// Qt Includes

#include <QLayout>
#include <QDebug>


NotificationBar::NotificationBar(QWidget *parent)
        : QWidget(parent)
        , m_blinkEffect(new BlinkEffect(this))
        , m_opacityAnimation(new QPropertyAnimation(m_blinkEffect, "opacity", this))
{
    m_blinkEffect->setOpacity(0);
    setGraphicsEffect(m_blinkEffect);
}


void NotificationBar::notifyUser(int animationDuration)
{
    m_opacityAnimation->setDuration(animationDuration);
    m_opacityAnimation->setStartValue(0.9);
    m_opacityAnimation->setEndValue(0.0);
    m_opacityAnimation->start();

}


void NotificationBar::destroy()
{
    qDebug() << Q_FUNC_INFO << "deleting the bar" << this;
    if (parentWidget() && parentWidget()->layout())
        parentWidget()->layout()->removeWidget(this);
    deleteLater();
}
