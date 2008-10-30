/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the demonstration applications of the Qt Toolkit.
**
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License versions 2.0 or 3.0 as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file.  Please review the following information
** to ensure GNU General Public Licensing requirements will be met:
** http://www.fsf.org/licensing/licenses/info/GPLv2.html and
** http://www.gnu.org/copyleft/gpl.html.  In addition, as a special
** exception, Nokia gives you certain additional rights. These rights
** are described in the Nokia Qt GPL Exception version 1.3, included in
** the file GPL_EXCEPTION.txt in this package.
**
** Qt for Windows(R) Licensees
** As a special exception, Nokia, as the sole copyright holder for Qt
** Designer, grants users of the Qt/Eclipse Integration plug-in the
** right for the Qt/Eclipse Integration to link to functionality
** provided by Qt Designer and its related libraries.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
**
****************************************************************************/

#include "chasewidget.h"

#include <QtCore/QPoint>

#include <QtGui/QApplication>
#include <QtGui/QHideEvent>
#include <QtGui/QPainter>
#include <QtGui/QPaintEvent>
#include <QtGui/QShowEvent>

ChaseWidget::ChaseWidget(QWidget *parent, QPixmap pixmap, bool pixmapEnabled)
    : QWidget(parent)
    , m_segment(0)
    , m_delay(100)
    , m_step(40)
    , m_timerId(-1)
    , m_animated(false)
    , m_pixmap(pixmap)
    , m_pixmapEnabled(pixmapEnabled)
{
}

void ChaseWidget::setAnimated(bool value)
{
    if (m_animated == value)
        return;
    m_animated = value;
    if (m_timerId != -1) {
        killTimer(m_timerId);
        m_timerId = -1;
    }
    if (m_animated) {
        m_segment = 0;
        m_timerId = startTimer(m_delay);
    }
    update();
}

void ChaseWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter p(this);
    if (m_pixmapEnabled && !m_pixmap.isNull()) {
        p.drawPixmap(0, 0, m_pixmap);
        return;
    }

    const int extent = qMin(width() - 8, height() - 8);
    const int displ = extent / 4;
    const int ext = extent / 4 - 1;

    p.setRenderHint(QPainter::Antialiasing, true);

    if(m_animated)
        p.setPen(Qt::gray);
    else
        p.setPen(QPen(palette().dark().color()));

    p.translate(width() / 2, height() / 2); // center

    for (int segment = 0; segment < segmentCount(); ++segment) {
        p.rotate(QApplication::isRightToLeft() ? m_step : -m_step);
        if(m_animated)
            p.setBrush(colorForSegment(segment));
        else
            p.setBrush(palette().background());
        p.drawEllipse(QRect(displ, -ext / 2, ext, ext));
    }
}

QSize ChaseWidget::sizeHint() const
{
    return QSize(32, 32);
}

void ChaseWidget::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == m_timerId) {
        ++m_segment;
        update();
    }
    QWidget::timerEvent(event);
}

QColor ChaseWidget::colorForSegment(int seg) const
{
    int index = ((seg + m_segment) % segmentCount());
    int comp = qMax(0, 255 - (index * (255 / segmentCount())));
    return QColor(comp, comp, comp, 255);
}

int ChaseWidget::segmentCount() const
{
    return 360 / m_step;
}

void ChaseWidget::setPixmapEnabled(bool enable)
{
    m_pixmapEnabled = enable;
}

