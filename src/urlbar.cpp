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


UrlBar::UrlBar(KHistoryComboBox *parent)
    : KHistoryComboBox(parent)
    , m_webView(0)
    , m_lineEdit(0)
{
    m_lineEdit = new QLineEdit;
    setLineEdit( m_lineEdit );

    webViewIconChanged();
}


UrlBar::~UrlBar()
{
}


QLineEdit *UrlBar::lineEdit()
{
    return m_lineEdit;
}


void UrlBar::setWebView(WebView *webView)
{
    Q_ASSERT(!m_webView);
    m_webView = webView;
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
    KUrl url = (m_webView)  ? m_webView->url() : KUrl();
    QIcon icon = BrowserApplication::instance()->icon(url);
    QPixmap pixmap(icon.pixmap(16, 16));
    QIcon urlIcon = QIcon(pixmap);
    
    // FIXME simple hack to show Icon in the urlbar, as calling changeUrl() doesn't affect it
    removeItem( 0 );
    insertUrl( 0 , urlIcon , url );
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
