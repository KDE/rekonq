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
#include <KStandardDirs>
#include <KIconLoader>

// Qt Includes
#include <QtGui/QContextMenuEvent>
#include <QtGui/QFocusEvent>
#include <QtGui/QKeyEvent>
#include <QStyleOptionFrameV2>
#include <QPainter>


IconButton::IconButton(QWidget *parent)
    : QToolButton(parent)
{
    setToolButtonStyle(Qt::ToolButtonIconOnly);
    setStyleSheet("IconButton { background-color:transparent; border: none; padding: 0px}");
    setCursor(Qt::ArrowCursor);
}


// -----------------------------------------------------------------------------------------------------------


LineEdit::LineEdit(QWidget* parent)
    : KLineEdit(parent)
    , _icon( new IconButton(this) )
{
    // cosmetic
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setMinimumWidth(200);
    setMinimumHeight(26);

    // initial style
    setStyleSheet( QString("LineEdit { padding: 0 0 0 %1px;} ").arg(_icon->sizeHint().width()) );
    
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
    // you need this before our code to draw inside the line edit..
    KLineEdit::paintEvent(event);
    
    if (text().isEmpty()) 
    {       
        QStyleOptionFrame option;
        initStyleOption(&option);
        QRect textRect = style()->subElementRect(QStyle::SE_LineEditContents, &option, this);
        QPainter painter(this);
        painter.setPen(Qt::gray);
        painter.drawText( textRect, 
                          Qt::AlignCenter, 
                          i18n("Search Bookmarks, History, Google.. just start typing here!")
                        );
    }
}


IconButton *LineEdit::addRightIcon(LineEdit::icon ic)
{
    IconButton *rightIcon = new IconButton(this);
    
    switch(ic)
    {
    case LineEdit::KGet:
        rightIcon->setIcon( KIcon("download") );
        rightIcon->setToolTip( i18n("List all links with KGet") );
        break;
    case LineEdit::RSS:
        rightIcon->setIcon( KIcon("application-rss+xml") );
        rightIcon->setToolTip( i18n("List all available RSS feeds") );
        break;
    case LineEdit::SSL:
        rightIcon->setIcon( KIcon("object-locked") );
        rightIcon->setToolTip( i18n("Show SSL Infos") );
        break;
    default:
        kDebug() << "ERROR.. default non extant case!!";
        break;
    }
    
    _rightIconsList << rightIcon;
    int iconsCount = _rightIconsList.count();
    rightIcon->move( width() - 23*iconsCount, 6);
    rightIcon->show();
    
    return rightIcon;
}


void LineEdit::clearRightIcons()
{
    qDeleteAll(_rightIconsList);
    _rightIconsList.clear();
}


void LineEdit::resizeEvent(QResizeEvent *event)
{
    int newHeight = ( height() - 19 )/2;
    _icon->move(4, newHeight );
    
    int iconsCount = _rightIconsList.count();
    int w = width();
    
    for(int i = 0; i < iconsCount; ++i)
    {
        IconButton *bt = _rightIconsList.at(i);
        bt->move( w - 25*(i+1), newHeight );
    }

    KLineEdit::resizeEvent(event);

}
