/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2009 by Andrea Diamantini <adjam7 at gmail dot com>
* Copyright (C) 2009 by Paweł Prażak <pawelprazak at gmail dot com>
* Copyright (C) 2009 by Lionel Chauvin <megabigbug@yahoo.fr>
*
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License as
* published by the Free Software Foundation; either version 2 of
* the License or (at your option) version 3 or any later version
* accepted by the membership of KDE e.V. (or its successor approved
* by the membership of KDE e.V.), which shall act as a proxy 
* defined in Section 14 of version 3 of the license.
* 
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
* ============================================================ */


// Self Includes
#include "lineedit.h"
#include "lineedit.moc"

// KDE Includes
#include <klocalizedstring.h>
#include <KDebug>

// Qt Includes
#include <QtGui/QContextMenuEvent>
#include <QtGui/QFocusEvent>
#include <QtGui/QKeyEvent>
#include <QStyleOptionFrameV2>
#include <QPainter>


LineEdit::LineEdit(QWidget* parent)
    : KLineEdit(parent)
    , _icon( new IconButton(this) )
{
    // cosmetic
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setMinimumWidth(200);
    setMinimumHeight(26);
    updateStyles();
    
    // doesn't show the clear button
    setClearButtonShown(false);
    
    // trap Key_Enter & Key_Return events, while emitting the returnPressed signal
    setTrapReturnKey(true);
    
    // insert decoded URLs
    setUrlDropsEnabled(true);

    // accept focus, via tabbing, clicking & wheeling
    setFocusPolicy(Qt::WheelFocus);
    
    // disable completion object (we have our own :) )
    setCompletionObject(0);
}


LineEdit::~LineEdit()
{
    delete _icon;
}


void LineEdit::updateStyles()
{
    adjustSize();
    _icon->adjustSize();
    if(_icon->toolButtonStyle() == Qt::ToolButtonIconOnly)
        _icon->move( 4, 3);
    else
        _icon->move( 2, 1);
    
    int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
    setStyleSheet(QString("LineEdit { padding-left: %1px; } ").arg(_icon->sizeHint().width() + frameWidth + 1));

    update();
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


void LineEdit::mouseDoubleClickEvent(QMouseEvent *)
{
    selectAll();
}


IconButton *LineEdit::iconButton() const
{
    return _icon;
}


void LineEdit::paintEvent(QPaintEvent *event)
{
    KLineEdit::paintEvent(event);
    
    if (text().isEmpty() && !hasFocus()) 
    {
        QStyleOptionFrame option;
        initStyleOption(&option);
        QRect textRect = style()->subElementRect(QStyle::SE_LineEditContents, &option, this);
        QPainter painter(this);
        painter.setPen(Qt::gray);
        painter.drawText( textRect, 
                          Qt::AlignLeft | Qt::AlignVCenter, 
                          i18n("Search Bookmarks, History, Google.. and the Kitchen Sink!")
                        );
    }
}
