/* ============================================================
 *
 * This file is a part of the rekonq project
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

// Self Includes
#include "findbar.h"
#include "findbar.moc"

// KDE Includes
#include <KLineEdit>
#include <KAction>
#include <KIcon>
#include <KToolBar>
#include <KDialog>
#include <KPushButton>

// Qt Includes
#include <QtGui>


FindBar::FindBar(KXmlGuiWindow *parent)
    : KToolBar( "Find Bar" , parent, Qt::BottomToolBarArea, true, false, false)
    , m_lineEdit(0)
{
    KAction *close = new KAction(KIcon("dialog-close") , "close" , this);
    connect( close , SIGNAL( triggered() ), this, SLOT( hide() ) );
    addAction( close );

    QLabel *label = new QLabel("Find: ");
    addWidget( label );

    m_lineEdit = new KLineEdit();
    m_lineEdit->setMaximumWidth( 200 );

    connect( m_lineEdit, SIGNAL( returnPressed() ), parent, SLOT( slotFindNext() ) );
    connect( m_lineEdit, SIGNAL( textEdited(const QString &) ), parent, SLOT( slotFindNext() ) );
    addWidget( m_lineEdit );

    KPushButton *findNext = new KPushButton( KIcon("go-down"), "&Next", this );
    KPushButton *findPrev = new KPushButton( KIcon("go-up"), "&Previous", this );
    // perhaps we don't need working on style..
//     findNext->setStyle();
//     findPrev->setStyle();
    connect( findNext, SIGNAL( clicked() ), parent, SLOT( slotFindNext() ) );
    connect( findPrev, SIGNAL( clicked() ), parent, SLOT( slotFindPrevious() ) );
    addWidget( findNext );
    addWidget( findPrev );

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


void FindBar::clear()
{
    m_lineEdit->setText(QString());
}


void FindBar::showFindBar()
{
    if (!isVisible()) 
    {
        show();
    }
    m_lineEdit->setFocus();
    m_lineEdit->selectAll();
}


void FindBar::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Escape)
    {
        hide();
        return;
    }
    if(event->key() == Qt::Key_Return && ! ( m_lineEdit->text().isEmpty() ) )
    {
        emit searchString( m_lineEdit->text() );
        return;
    }
    QWidget::keyPressEvent(event);
}
