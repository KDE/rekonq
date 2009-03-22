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
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* ============================================================ */


// Self Includes
#include "searchbar.h"
#include "searchbar.moc"

// Local Includes
#include "application.h"
#include "mainwindow.h"

// KDE Includes
#include <KUrl>
#include <KDebug>

// Qt Includes
#include <QtCore>
#include <QtGui>


SearchBar::SearchBar(QWidget *parent) : 
    KLineEdit(parent)
{
    setMinimumWidth(180);
    kWarning() << "setting fixed minimum width.." ;

    setFocusPolicy( Qt::WheelFocus );
    setMouseTracking( true );
    setAcceptDrops(true);

    QSizePolicy policy = sizePolicy();
    setSizePolicy(QSizePolicy::Preferred, policy.verticalPolicy());

    setClearButtonShown( true );

    QPalette p;
    p.setColor( QPalette::Text , Qt::lightGray );
    setPalette( p );
    setText( i18n("Search..") );

    connect( this, SIGNAL( returnPressed() ) , this , SLOT( searchNow() ) );
}


SearchBar::~SearchBar()
{
}


void SearchBar::searchNow()
{
    QString searchText = text();

    KUrl url(QLatin1String("http://www.google.com/search"));
    url.addQueryItem(QLatin1String("q"), searchText);
    url.addQueryItem(QLatin1String("ie"), QLatin1String("UTF-8"));
    url.addQueryItem(QLatin1String("oe"), QLatin1String("UTF-8"));
    url.addQueryItem(QLatin1String("client"), QLatin1String("rekonq"));
    emit search(url);
}


void SearchBar::focusInEvent(QFocusEvent *event)
{
    KLineEdit::focusInEvent(event);

    QPalette p;
    p.setColor( QPalette::Text , Qt::black );
    setPalette( p );
    clear();
}

