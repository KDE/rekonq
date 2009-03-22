/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2007-2008 Trolltech ASA. All rights reserved
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


#ifndef EDITTREEVIEW_H
#define EDITTREEVIEW_H

#include <QtGui/QTreeView>

class EditTreeView : public QTreeView
{
    Q_OBJECT

public:
    EditTreeView(QWidget *parent = 0);
    void keyPressEvent(QKeyEvent *event);

public slots:
    void removeOne();
    void removeAll();
};

#endif // EDITTREEVIEW_H

