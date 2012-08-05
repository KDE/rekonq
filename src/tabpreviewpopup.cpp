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


//Self Includes
#include "tabpreviewpopup.h"

// Rekonq Includes
#include "webtab.h"

//Qt Includes
#include <QLabel>
#include <QVBoxLayout>
#include <QPalette>
#include <QBitmap>
#include <QPoint>
#include <QPaintEvent>
#include <QStylePainter>
#include <QStyleOptionFrame>


TabPreviewPopup::TabPreviewPopup(WebTab* tab, QWidget* parent)
    : KPassivePopup(parent),
      m_thumbnail(new QLabel(this)),
      m_url(new QLabel(this))
{
    m_thumbnail->setAlignment(Qt::AlignHCenter);
    m_url->setAlignment(Qt::AlignHCenter);

    QVBoxLayout *vb = new QVBoxLayout(this);
    vb->addWidget(m_thumbnail);
    vb->addWidget(m_url);
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

    setWebTab(tab);
}


TabPreviewPopup::~TabPreviewPopup()
{
    delete m_thumbnail;
    delete m_url;
}


QSize TabPreviewPopup::thumbnailSize() const
{
    return m_thumbnail->pixmap()->size();
}


void TabPreviewPopup::setWebTab(WebTab* tab)
{
    // The ratio of the tab
    double ratio = (double) tab->size().height() / tab->size().width();

    int w = previewBaseSize;
    int h = previewBaseSize;

    // Apply the ratio to the width or the weight to not exceed previewBaseSize
    if (ratio < 1)
        h *= ratio;
    else if (ratio > 1)
        w *= (1 / ratio);

    const QPixmap preview = tab->tabPreview(w, h);

    if (!preview.isNull())
    {
        setThumbnail(preview);
        setUrl(tab->url().prettyUrl());
        setFixedSize(preview.width(), preview.height() + m_url->heightForWidth(preview.width()));
    }
}


void TabPreviewPopup::setThumbnail(const QPixmap& pixmap)
{
    m_thumbnail->setPixmap(pixmap);
}


void TabPreviewPopup::setUrl(const QString& text)
{
    m_url->setText(text);
}


void TabPreviewPopup::setFixedSize(int w, int h)
{
    KPassivePopup::setFixedSize(w, h);
    const int margin = 1 + style()->pixelMetric(QStyle::PM_ToolTipLabelFrameWidth, 0, this);
    m_url->setText(m_url->fontMetrics().elidedText(m_url->text(), Qt::ElideMiddle, this->width() - margin * 2));

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
