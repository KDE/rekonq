/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2007-2008 Trolltech ASA. All rights reserved
* Copyright (C) 2008-2009 by Andrea Diamantini <adjam7 at gmail dot com>
* Copyright (C) 2009 rekonq team. Please, see AUTHORS file for details
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



#include "edittreeview.h"

#include <QtGui/QKeyEvent>

EditTreeView::EditTreeView(QWidget *parent)
        : QTreeView(parent)
{
}

void EditTreeView::keyPressEvent(QKeyEvent *event)
{
    if ((event->key() == Qt::Key_Delete
            || event->key() == Qt::Key_Backspace)
            && model())
    {
        removeOne();
    }
    else
    {
        QAbstractItemView::keyPressEvent(event);
    }
}

void EditTreeView::removeOne()
{
    if (!model())
        return;
    QModelIndex ci = currentIndex();
    int row = ci.row();
    model()->removeRow(row, ci.parent());
}

void EditTreeView::removeAll()
{
    if (!model())
        return;
    model()->removeRows(0, model()->rowCount(rootIndex()), rootIndex());
}

