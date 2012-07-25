/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2011 by Vyacheslav Blinov <blinov dot vyacheslav at gmail dot com>
* Copyright (C) 2011-2012 by Andrea Diamantini <adjam7 at gmail dot com>
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
#include "tabpreviewpopup.h"
#include "tabpreviewpopup.moc"

// Rekonq Includes
#include "tabbar.h"

// Qt Includes
#include <QLabel>
#include <QVBoxLayout>
#include <QPalette>
#include <QBitmap>
#include <QPoint>
#include <QPaintEvent>
#include <QStylePainter>
#include <QStyleOptionFrame>


TabPreviewPopup::TabPreviewPopup(const QPixmap &pixmap, const QString &urlText, QWidget *parent)
    : KPassivePopup(parent),
      m_thumbLabel(new QLabel(this)),
      m_urlLabel(new QLabel(this))
{
    m_thumbLabel->setAlignment(Qt::AlignHCenter);
    m_urlLabel->setAlignment(Qt::AlignHCenter);

    QVBoxLayout *vb = new QVBoxLayout(this);
    vb->addWidget(m_thumbLabel);
    vb->addWidget(m_urlLabel);
    this->setLayout(vb);

    layout()->setAlignment(Qt::AlignTop);
    layout()->setMargin(0);

    setPopupStyle(KPassivePopup::CustomStyle + 1);

    // use ToolTip appearance
    QPalette p;

    // adjust background color to use tooltip colors
    p.setColor(backgroundRole(), p.color(QPalette::ToolTipBase));
    p.setColor(QPalette::Base, p.color(QPalette::ToolTipBase));

    // adjust foreground color to use tooltip colors
    p.setColor(foregroundRole(), p.color(QPalette::ToolTipText));
    p.setColor(QPalette::Text, p.color(QPalette::ToolTipText));

    setPalette(p);

    // window flags and attributes
    setWindowFlags(Qt::ToolTip);
    setAttribute(Qt::WA_TranslucentBackground);
    setWindowOpacity(style()->styleHint(QStyle::SH_ToolTipLabel_Opacity, 0, this) / 255.0);

    // margins
    const int margin = 1 + style()->pixelMetric(QStyle::PM_ToolTipLabelFrameWidth, 0, this);
    setContentsMargins(margin, margin, margin, margin);

    m_thumbLabel->setPixmap(pixmap);
    m_urlLabel->setText(urlText);

    setFixedSize(pixmap.width(), pixmap.height() + m_urlLabel->heightForWidth(pixmap.width()));
}


TabPreviewPopup::~TabPreviewPopup()
{
    delete m_thumbLabel;
    delete m_urlLabel;
}


void TabPreviewPopup::setFixedSize(int w, int h)
{
    KPassivePopup::setFixedSize(w, h);
    const int margin = 1 + style()->pixelMetric(QStyle::PM_ToolTipLabelFrameWidth, 0, this);
    m_urlLabel->setText(m_urlLabel->fontMetrics().elidedText(m_urlLabel->text(), Qt::ElideMiddle, this->width() - margin * 2));

    //calculate mask
    QStyleOptionFrame opt;
    opt.init(this);

    QStyleHintReturnMask mask;
    style()->styleHint(QStyle::SH_ToolTip_Mask, &opt, this, &mask);
    setMask(mask.region);
}


void TabPreviewPopup::paintEvent(QPaintEvent* event)
{
    QStyleOptionFrame opt;
    opt.init(this);

    QStylePainter painter(this);
    painter.setClipRegion(event->region());
    painter.drawPrimitive(QStyle::PE_PanelTipLabel, opt);
}
