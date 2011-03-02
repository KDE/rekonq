/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2011 Tröscher Johannes <fritz_van_tom@hotmail.com>
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


//Self Includes
#include "tabhighlighteffect.h"
#include "tabhighlighteffect.moc"

//Qt Includes
#include <QVariant>
#include <tabbar.h>


const QByteArray prep("hAnim");


TabHighlightEffect::TabHighlightEffect(TabBar *tabBar)
        : QGraphicsEffect(tabBar)
        , m_tabBar(tabBar)
        , m_highlightColor(tabBar->palette().highlight().color().lighter())
{
    Q_ASSERT(m_tabBar);
}


void TabHighlightEffect::draw(QPainter *painter)
{
    const QPixmap &pixmap = sourcePixmap();
    
    if (pixmap.isNull())
        return;
    
    painter->drawPixmap(QPoint(0, 0), pixmap);

    Q_FOREACH(const QByteArray &propertyName, dynamicPropertyNames())
    {
        if (!propertyName.startsWith(prep))
            continue;

        int index = propertyName.right(propertyName.size() - prep.size()).toInt();
        qreal opacity = property(propertyName).toReal();
        QRect textRect =  m_tabBar->tabTextRect(index);

        QString tabText = m_tabBar->fontMetrics().elidedText(m_tabBar->tabText(index), Qt::ElideRight,
                          textRect.width(), Qt::TextShowMnemonic);

        painter->setOpacity(opacity);
        painter->setPen(m_highlightColor);
        painter->drawText(textRect, Qt::AlignCenter | Qt::TextShowMnemonic, tabText);
    }
}


bool TabHighlightEffect::event(QEvent* event)
{
    if (event->type() == QEvent::DynamicPropertyChange)
    {
        QDynamicPropertyChangeEvent *pChangeEv = dynamic_cast<QDynamicPropertyChangeEvent*>(event);

        if (pChangeEv->propertyName().startsWith(prep))
        {
            update();
            return true;
        }
    }

    return QGraphicsEffect::event(event);
}
