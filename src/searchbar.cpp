/* ============================================================
 *
 * This file is a part of the reKonq project
 *
 * Copyright 2008 Benjamin C. Meyer <ben@meyerhome.net>
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

#include "searchbar.h"
#include "moc_searchbar.cpp"

#include <KLineEdit>
#include <KAction>
#include <KIconLoader>
#include <KToolBar>
#include <KStandardAction>

#include <QHBoxLayout>
#include <QLabel>
#include <QEvent>
#include <QPixmap>
#include <QShortcut>
#include <QResizeEvent>

SearchBar::SearchBar(QWidget *parent)
    : QWidget(parent)
    , m_object(0)
    , m_widget(0)
    , m_lineEdit(0)
{
    initializeSearchWidget();

    // we start off hidden
    setMaximumHeight(0);
//     m_widget->setGeometry(0, -1 * m_widget->height(), m_widget->width(), m_widget->height());
    hide();

    new QShortcut(QKeySequence(Qt::Key_Escape), this, SLOT(hide()));
}

SearchBar::~SearchBar()
{
    delete m_object;
    delete m_widget;
    delete m_lineEdit;
}


void SearchBar::initializeSearchWidget()
{
    QHBoxLayout *layout = new QHBoxLayout();

    KToolBar *bar1 = new KToolBar(this);
    bar1->addAction( KStandardAction::close(this, SLOT( hide() ) , this ) );
    layout->addWidget( bar1 );

    QLabel *label = new QLabel("Find: ");
    layout->addWidget( label );

    m_lineEdit = new KLineEdit();
    connect( m_lineEdit, SIGNAL( returnPressed() ), this, SLOT( slotFindNext() ) );
    connect( m_lineEdit, SIGNAL( textEdited(const QString &) ), this, SLOT( slotFindNext() ) );
    layout->addWidget( m_lineEdit );

    KToolBar *bar2 = new KToolBar(this);
    bar2->addAction( KStandardAction::findNext(this, SLOT( slotFindNext() ) , this ) );
    bar2->addAction( KStandardAction::findPrev(this, SLOT( slotFindPrevious() ) , this ) );
    layout->addWidget( bar2 );

    layout->addStretch();

    setLayout(layout);
}


void SearchBar::setSearchBar(QObject *object)
{
    m_object = object;
}


QObject *SearchBar::getSearchBar() const
{
    return m_object;
}


void SearchBar::clear()
{
    m_lineEdit->setText(QString());
}


void SearchBar::showFind()
{
    if (!isVisible()) 
    {
        show();
    }
    m_lineEdit->setFocus();
    m_lineEdit->selectAll();
}


void SearchBar::resizeEvent(QResizeEvent *event)
{
/*    if (event->size().width() != m_widget->width())
        m_widget->resize(event->size().width(), m_widget->height());
    QWidget::resizeEvent(event);*/
}


void SearchBar::frameChanged(int frame)
{
    if (!m_widget)
        return;
    m_widget->move(0, frame);
    int height = qMax(0, m_widget->y() + m_widget->height());
    setMinimumHeight(height);
    setMaximumHeight(height);
}


void SearchBar::slotFindNext()
{}

void SearchBar::slotFindPrevious()
{}

