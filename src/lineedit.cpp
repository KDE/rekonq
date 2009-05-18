/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2009 by Paweł Prażak <pawelprazak at gmail dot com>
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
#include "lineedit.h"
#include "lineedit.moc"

// KDE Includes
#include <KDebug>

// Qt Includes
#include <QtGui/QContextMenuEvent>
#include <QtGui/QFocusEvent>
#include <QtGui/QKeyEvent>


LineEdit::LineEdit(QWidget* parent)
        : KLineEdit(parent)
{
    setMinimumWidth(200);
    setFocusPolicy(Qt::WheelFocus);

    setHandleSignals(true);
}


LineEdit::~LineEdit()
{
}


void LineEdit::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape)
    {
        clearFocus();
        event->accept();
    }

    KLineEdit::keyPressEvent(event);
}


void LineEdit::contextMenuEvent(QContextMenuEvent *event)
{
    KLineEdit::contextMenuEvent(event);
}


void LineEdit::focusInEvent(QFocusEvent *event)
{
    selectAll();

    KLineEdit::focusInEvent(event);
}


void LineEdit::focusOutEvent(QFocusEvent *event)
{
    KLineEdit::focusOutEvent(event);

    // reset cursor state and deselect
    setCursorPosition(0);
    deselect();
}
