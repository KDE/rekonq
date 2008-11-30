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

#ifndef URLBAR_H
#define URLBAR_H

// Local Includes
#include "webview.h"

// KDE Includes
#include <KLineEdit>
#include <KComboBox>

// Qt Includes
#include <QWidget>
#include <QLabel>


class UrlBar : public QWidget
{
Q_OBJECT

public:
    UrlBar(QWidget *parent = 0);
    ~UrlBar();

    KLineEdit *lineEdit();
    void setWebView(WebView *webView);

// FIXME needed to color urlbar while loading urls
// protected:
//     void paintEvent(QPaintEvent *event);

private slots:
    void webViewUrlChanged(const QUrl &url);
    void webViewIconChanged();

private:
    QLinearGradient generateGradient(const QColor &color) const;

    WebView *m_webView;

    QLabel *m_iconLabel;
    KLineEdit *m_lineEdit;
    QColor m_defaultBaseColor;
};

#endif
