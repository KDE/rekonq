/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2008-2009 by Andrea Diamantini <adjam7 at gmail dot com>
* Copyright (C) 2009 rekonq team. Please, see AUTHORS file for details
*
*
* This program is free software; you can redistribute it
* and/or modify it under the terms of the GNU General
* Public License as published by the Free Software Foundation;
* either version 2, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* ============================================================ */


#ifndef URLBAR_H
#define URLBAR_H


// KDE Includes
#include <KHistoryComboBox>

// Forward Declarations
class WebView;

class QWidget;
class QLineEdit;
class QUrl;
class QLinearGradient;
class QColor;


class UrlBar : public KHistoryComboBox
{
    Q_OBJECT

public:
    UrlBar(QWidget *parent = 0);
    ~UrlBar();

    QLineEdit *lineEdit();
    void setWebView(WebView *webView);

private slots:
    void webViewUrlChanged(const QUrl &url);
    void webViewIconChanged();

protected:
//      void paintEvent( QPaintEvent * );

private:
    QLinearGradient generateGradient(const QColor &color) const;

    WebView* m_webView;
    QLineEdit* m_lineEdit;
    QColor m_defaultBaseColor;
};

#endif
