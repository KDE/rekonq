/* ============================================================
 *
 * This file is a part of the reKonq project
 *
 * Copyright (C) 2008 by Andrea Diamantini <adjam7 at gmail dot com>
 *
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

// Local Includes
#include "urlbar.h"
#include "urlbar.moc"

#include "browserapplication.h"

// Qt Includes
#include <QtGui>


UrlBar::UrlBar(QWidget *parent)
    : QWidget(parent)
    , m_webView(0)
    , m_iconLabel(0)
    , m_lineEdit(0)
{
    // icon
    m_iconLabel = new QLabel;
    m_iconLabel->resize(16, 16);

    m_lineEdit = new KLineEdit;

    QHBoxLayout *layout = new QHBoxLayout;
    layout->addWidget(m_iconLabel);
    layout->addWidget(m_lineEdit);
    setLayout(layout);

    m_defaultBaseColor = palette().color(QPalette::Base);
    setPalette( QPalette(Qt::white) );
    setAutoFillBackground( true );

    webViewIconChanged();
}


UrlBar::~UrlBar()
{
//     delete m_webView;
    delete m_iconLabel;
    delete m_lineEdit;
}


KLineEdit *UrlBar::lineEdit()
{
    return m_lineEdit;
}


void UrlBar::setWebView(WebView *webView)
{
    Q_ASSERT(!m_webView);
    m_webView = webView;
//     m_iconLabel->m_webView = webView;
    connect(webView, SIGNAL(urlChanged(const QUrl &)), this, SLOT(webViewUrlChanged(const QUrl &)));
    connect(webView, SIGNAL(loadFinished(bool)), this, SLOT(webViewIconChanged()));
    connect(webView, SIGNAL(iconChanged()), this, SLOT(webViewIconChanged()));
    connect(webView, SIGNAL(loadProgress(int)), this, SLOT(update()));
}


void UrlBar::webViewUrlChanged(const QUrl &url)
{
    m_lineEdit->setText(url.toString());
    m_lineEdit->setCursorPosition(0);
}


void UrlBar::webViewIconChanged()
{
    QUrl url = (m_webView)  ? m_webView->url() : QUrl();
    QIcon icon = BrowserApplication::instance()->icon(url);
    QPixmap pixmap(icon.pixmap(16, 16));
    m_iconLabel->setPixmap(pixmap);
}


QLinearGradient UrlBar::generateGradient(const QColor &color) const
{
    QLinearGradient gradient(0, 0, 0, height());
    gradient.setColorAt(0, m_defaultBaseColor);
    gradient.setColorAt(0.15, color.lighter(120));
    gradient.setColorAt(0.5, color);
    gradient.setColorAt(0.85, color.lighter(120));
    gradient.setColorAt(1, m_defaultBaseColor);
    return gradient;
}

// FIXME
// void UrlBar::paintEvent(QPaintEvent *event)
// {
//     QPalette p = palette();
//     if (m_webView && m_webView->url().scheme() == QLatin1String("https")) {
//         QColor lightYellow(248, 248, 210);
//         p.setBrush(QPalette::Base, generateGradient(lightYellow));
//     } else {
//         p.setBrush(QPalette::Base, m_defaultBaseColor);
//     }
//     setPalette(p);
// 
//     QPainter painter(this);
//     QStyleOptionFrameV2 panel;
// //     initStyleOption(&panel);
//     QRect backgroundRect = style()->subElementRect(QStyle::SE_LineEditContents, &panel, this);
//     if (m_webView && !hasFocus()) 
//     {
//         int progress = m_webView->progress();
//         QColor loadingColor = QColor(116, 192, 250);
//         painter.setBrush(generateGradient(loadingColor));
//         painter.setPen(Qt::transparent);
//         int mid = backgroundRect.width() / 100 * progress;
//         QRect progressRect(backgroundRect.x(), backgroundRect.y(), mid, backgroundRect.height());
//         painter.drawRect(progressRect);
//     }
// }

