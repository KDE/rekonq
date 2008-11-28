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

#ifndef FINDBAR_H
#define FINDBAR_H

#include <KLineEdit>
#include <KToolBar>

#include <QWidget>

class FindBar : public KToolBar
{
    Q_OBJECT

public:
    FindBar(QWidget *parent = 0);
    ~FindBar();
    KLineEdit *lineEdit();

public slots:
    void clear();
    void showFind();
    void slotFindNext();
    void slotFindPrevious();

private slots:
    void frameChanged(int frame);

private:
    void initializeFindWidget();

    KLineEdit *m_lineEdit;
    QWidget *m_centralWidget;
};

#endif
