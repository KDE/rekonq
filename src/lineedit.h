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


#ifndef LINEEDIT_H
#define LINEEDIT_H

// Qt Includes

// KDE Includes
#include <KLineEdit>

// Local Includes

class QContextMenuEvent;
class QFocusEvent;
class QKeyEvent;

class LineEdit : public KLineEdit
{
public:
    explicit LineEdit(QWidget *parent = 0);
    virtual ~LineEdit();

protected:
  virtual void keyPressEvent (QKeyEvent*);
  virtual void contextMenuEvent (QContextMenuEvent*);
  virtual void focusInEvent (QFocusEvent*);
  virtual void focusOutEvent (QFocusEvent*);
};

#endif // LINEEDIT_H
