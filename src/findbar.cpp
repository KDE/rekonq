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


FindBar::FindBar(KXmlGuiWindow *mainwindow)
    : QWidget()
    , m_lineEdit(0)
{
    QHBoxLayout *layout = new QHBoxLayout;

    // cosmetic
    layout->setMargin(2);

    // hide button
    QToolButton *hideButton = new QToolButton(this);
    hideButton->setAutoRaise(true);
    hideButton->setIcon(KIcon("dialog-close"));
    connect(hideButton, SIGNAL(clicked()), this, SLOT(hide()));
    layout->addWidget(hideButton);
    layout->setAlignment( hideButton, Qt::AlignLeft|Qt::AlignTop );

    // label
    QLabel *label = new QLabel("Find: ");
    layout->addWidget(label);

    // lineEdit, focusProxy
    m_lineEdit = new KLineEdit(this);
    setFocusProxy(m_lineEdit);
    m_lineEdit->setMaximumWidth( 250 );
    connect( m_lineEdit, SIGNAL( returnPressed() ), mainwindow, SLOT( slotFindNext() ) );
    connect( m_lineEdit, SIGNAL( textEdited(const QString &) ), mainwindow, SLOT( slotFindNext() ) );
    layout->addWidget( m_lineEdit );

    // buttons
    KPushButton *findNext = new KPushButton( KIcon("go-down"), "&Next", this );
    KPushButton *findPrev = new KPushButton( KIcon("go-up"), "&Previous", this );
    connect( findNext, SIGNAL( clicked() ), mainwindow, SLOT( slotFindNext() ) );
    connect( findPrev, SIGNAL( clicked() ), mainwindow, SLOT( slotFindPrevious() ) );
    layout->addWidget( findNext );
    layout->addWidget( findPrev );
    
    // stretching widget on the left
    layout->addStretch();

    setLayout(layout);

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
    if(event->key() == Qt::Key_Return && !m_lineEdit->text().isEmpty() )
    {
        emit searchString( m_lineEdit->text() );
        return;
    }
    QWidget::keyPressEvent(event);
}
