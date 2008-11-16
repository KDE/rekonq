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
#include "searchbar.h"
#include "searchbar.moc"

// Qt Includes
#include <QVBoxLayout>

SearchBar::SearchBar(QWidget *parent) : 
    QWidget(parent),
    m_lineEdit(0)
{
    m_lineEdit = new KLineEdit(this);
    m_lineEdit->setClearButtonShown( true );

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(m_lineEdit);
    setLayout(layout);

    connect( lineEdit() , SIGNAL( returnPressed() ) , this , SLOT( searchNow() ) );
}

SearchBar::~SearchBar()
{
}

void SearchBar::searchNow()
{
    QString searchText = m_lineEdit->text();

    QUrl url(QLatin1String("http://www.google.com/search"));
    url.addQueryItem(QLatin1String("q"), searchText);
    url.addQueryItem(QLatin1String("ie"), QLatin1String("UTF-8"));
    url.addQueryItem(QLatin1String("oe"), QLatin1String("UTF-8"));
    url.addQueryItem(QLatin1String("client"), QLatin1String("reKonq"));
    emit search(url);

}

KLineEdit *SearchBar::lineEdit()
{
    return m_lineEdit;
}

