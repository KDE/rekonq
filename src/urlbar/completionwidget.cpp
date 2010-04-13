/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2009 by Andrea Diamantini <adjam7 at gmail dot com>
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
#include "completionwidget.h"
#include "completionwidget.moc"

// Local Includes
#include "application.h"

// KDE Includes
#include <KGlobalSettings>
#include <KDebug>
#include <KUrl>

// Qt Includes
#include <QPoint>
#include <QSize>
#include <QVBoxLayout>
#include <QString>
#include <QEvent>
#include <QKeyEvent>


CompletionWidget::CompletionWidget( QWidget *parent)
    :QFrame( parent, Qt::ToolTip)
    , _parent(parent)
    , _currentIndex(-1)
{
    QPalette p(palette());
    p.setColor(QPalette::Background, Qt::white); // TODO: choose the correct color
    setPalette(p);
    setFrameStyle(QFrame::Panel);
    setLayoutDirection(Qt::LeftToRight);
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setMargin(0);
    setLayout(layout);

}


void CompletionWidget::insertSearchList(const UrlSearchList &list)
{
    _list = list;
    int i=0;
    foreach(UrlSearchItem item, _list)
    {
        ListItem *suggestion = new ListItem(item);
        connect(suggestion, SIGNAL(itemClicked(ListItem *, Qt::MouseButton)), this, SLOT(itemChosen(ListItem *, Qt::MouseButton)));
        suggestion->setObjectName( QString::number(i++) );
        layout()->addWidget( suggestion );
    }
}


void CompletionWidget::sizeAndPosition()
{
    // size
    int h = 34;
    ListItem *widget;
    for(int i = 0; i < layout()->count(); ++i)
    {
        widget = findChild<ListItem *>( QString::number(i) );
        h = qMax(widget->sizeHint().height(), h);
    }
    setFixedHeight(layout()->count() * (h + 10) );
    setFixedWidth( _parent->width() );

    // position
    QPoint p = _parent->mapToGlobal( QPoint(0,0) );
    move(p.x(), p.y() + _parent->height());
}


void CompletionWidget::popup()
{
    sizeAndPosition();
    if (!isVisible()) 
        show();
}


void CompletionWidget::up()
{
    // deactivate previous
    if(_currentIndex != -1)
    {
        ListItem *widget = findChild<ListItem *>( QString::number(_currentIndex) );
        widget->deactivate();
    }

    if(_currentIndex > 0)
        _currentIndex--;
    else
        _currentIndex=layout()->count()-1;       

    // activate "new" current
    ListItem *widget = findChild<ListItem *>( QString::number(_currentIndex) );
    widget->activate();
}


void CompletionWidget::down()
{
    // deactivate previous
    if(_currentIndex != -1)
    {
        ListItem *widget = findChild<ListItem *>( QString::number(_currentIndex) );
        widget->deactivate();
    }
    
    if(_currentIndex < _list.count() -1)
        _currentIndex++;
    else
        _currentIndex=0;
            
    // activate "new" current
    ListItem *widget = findChild<ListItem *>( QString::number(_currentIndex) );
    widget->activate();
}


void CompletionWidget::clear()
{
    QLayoutItem *child;
    while ((child = layout()->takeAt(0)) != 0) 
    {
        delete child->widget(); 
        delete child;
    }
    _currentIndex = -1;
}


bool CompletionWidget::eventFilter( QObject *o, QEvent *e )
{
    int type = e->type();
    QWidget *wid = qobject_cast<QWidget*>(o);
    
    if (o == this) 
    {
        return false;
    }

    //hide conditions of the CompletionWidget
    if (wid 
        && ((wid == _parent && (type == QEvent::Move || type == QEvent::Resize))  
        || ((wid->windowFlags() & Qt::Window) 
            && (type == QEvent::Move || type == QEvent::Hide || type == QEvent::WindowDeactivate) 
            && wid == _parent->window())
        || (type == QEvent::MouseButtonPress && !isAncestorOf(wid)))
       )
    {
        hide();
        return false;
    }

    //actions on the CompletionWidget
    if (wid && wid->isAncestorOf(_parent) && isVisible()) 
    {
        if ( type == QEvent::KeyPress ) 
        {
            QKeyEvent *ev = static_cast<QKeyEvent *>( e );
            switch ( ev->key() ) 
            {
                case Qt::Key_Up:
                case Qt::Key_Backtab:
                    if (ev->modifiers() == Qt::NoButton || (ev->modifiers() & Qt::ShiftModifier)) 
                    {
                        up();
                        ev->accept();
                        return true;
                    }
                    break;

                case Qt::Key_Down:
                case Qt::Key_Tab:
                    if (ev->modifiers() == Qt::NoButton)
                    {
                        down();
                        ev->accept();
                        return true;
                    }
                    break;

                case Qt::Key_Enter:
                case Qt::Key_Return:
                        hide();
                        if(_currentIndex >= 0)
                            emit chosenUrl(_list.at(_currentIndex).url, Rekonq::CurrentTab);
                        else
                            emit loadTypedUrl();
                        ev->accept();
                        return true;
                    break;
            }
        }
    }

    return QFrame::eventFilter(o,e);
}


void CompletionWidget::setVisible( bool visible )
{
    if (visible) 
    {
        Application::instance()->installEventFilter(this);
    }
    else
    {
        Application::instance()->removeEventFilter(this);
    }
    
    QFrame::setVisible(visible);
}


void CompletionWidget::itemChosen(ListItem *item, Qt::MouseButton button)
{
    if(button == Qt::MidButton)
        emit chosenUrl(_list.at(layout()->indexOf(item)).url, Rekonq::NewCurrentTab);
    else
        emit chosenUrl(_list.at(layout()->indexOf(item)).url, Rekonq::CurrentTab);
    hide();
}
