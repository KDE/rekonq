/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2008-2009 by Andrea Diamantini <adjam7 at gmail dot com>
* Copyright (C) 2009 by Domrachev Alexandr <alexandr.domrachev@gmail.com>
* Copyright (C) 2009 by Paweł Prażak <pawelprazak at gmail dot com>
*
*
* This program is free software; you can redistribute it
* and/or modify it under the terms of the GNU General
* Public License as published by the Free Software Foundation;
* either version 3, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* ============================================================ */


#ifndef URLBAR_H
#define URLBAR_H


// Local Includes
#include "lineedit.h"

// KDE Includes
#include <KUrl>
#include <KHistoryComboBox>

// Qt Includes
#include <QIcon>
#include <QPointer>
#include <QUrl>

// Forward Declarations
class QLinearGradient;
class QWidget;


class UrlBar : public KHistoryComboBox
{
    Q_OBJECT

public:
    UrlBar(QWidget *parent = 0);
    ~UrlBar();

    void selectAll() const;
    KUrl url() const;
    QSize sizeHint() const;
    void setBackgroundColor(QColor);

signals:
    void activated(const KUrl&);

public slots:
    void setUrl(const QUrl &url);
    void slotUpdateProgress(int progress);

private slots:
    void slotActivated(const QString&);
    void slotLoadFinished(bool);
    void slotCleared();
    void slotUpdateUrl();

protected:
    virtual void paintEvent(QPaintEvent *event);
    virtual void focusOutEvent(QFocusEvent *event);

private:
    void setupLineEdit();

    KLineEdit *lineEdit() const;

    static QLinearGradient generateGradient(const QColor &color, int height);

    static QColor s_defaultBaseColor;

    LineEdit *m_lineEdit;

    KUrl m_currentUrl;
    int m_progress;
};

#endif
