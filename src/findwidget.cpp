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

#include "findwidget.h"

#include <QHBoxLayout>
#include <QLabel>

#include <KLineEdit>
#include <KToolBar>
#include <KAction>
#include <KStandardAction>

FindWidget::FindWidget(QWidget * parent) : QWidget(parent)
{
    QHBoxLayout *layout = new QHBoxLayout();

    KToolBar *bar1 = new KToolBar(this);
    bar1->addAction( KStandardAction::close(this, SLOT( searchNow() ) , this ) );
    layout->addWidget( bar1 );

    QLabel *label = new QLabel("Find: ");
    layout->addWidget( label );

    KLineEdit *findLineEdit = new KLineEdit();
    layout->addWidget( findLineEdit );

    KToolBar *bar2 = new KToolBar(this);
    bar2->addAction( KStandardAction::findNext(this, SLOT( searchNow() ) , this ) );
    bar2->addAction( KStandardAction::findPrev(this, SLOT( searchNow() ) , this ) );
    layout->addWidget( bar2 );

    layout->addStretch();

    setLayout(layout);
}

QString FindWidget::searchNow()
{
    QString prova("adjam");
    return prova;
}
