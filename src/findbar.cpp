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

#include "findbar.h"
#include "moc_findbar.cpp"

#include <KLineEdit>
#include <KAction>
#include <KIconLoader>
#include <KToolBar>
#include <KStandardAction>

#include <QtGui>

FindBar::FindBar(QWidget *parent)
    : KToolBar(parent)
    , m_lineEdit(0)
{
    initializeFindWidget();

    // we start off hidden
    hide();
}


FindBar::~FindBar()
{
    delete m_lineEdit;
}


KLineEdit *FindBar::lineEdit()
{
    return m_lineEdit;
}


void FindBar::initializeFindWidget()
{
    addAction( KIcon("dialog-close") , "close" , this, SLOT( hide() ) );

    QLabel *label = new QLabel("Find: ");
    addWidget( label );

    m_lineEdit = new KLineEdit();
    connect( m_lineEdit, SIGNAL( returnPressed() ), this, SLOT( slotFindNext() ) );
    connect( m_lineEdit, SIGNAL( textEdited(const QString &) ), this, SLOT( slotFindNext() ) );
    addWidget( m_lineEdit );

    addAction( KStandardAction::findNext(this, SLOT( slotFindNext() ) , this ) );
    addAction( KStandardAction::findPrev(this, SLOT( slotFindPrevious() ) , this ) );

    QLabel *spaceLabel = new QLabel("                                                                         "); // FIXME
    addWidget( spaceLabel );
}



void FindBar::clear()
{
    m_lineEdit->setText(QString());
}


void FindBar::showFind()
{
    if (!isVisible()) 
    {
        show();
    }
    m_lineEdit->setFocus();
    m_lineEdit->selectAll();
}



void FindBar::frameChanged(int frame)
{
/*    if (!m_widget)
        return;
    m_widget->move(0, frame);
    int height = qMax(0, m_widget->y() + m_widget->height());
    setMinimumHeight(height);
    setMaximumHeight(height);*/
}


void FindBar::slotFindNext()
{}

void FindBar::slotFindPrevious()
{}

