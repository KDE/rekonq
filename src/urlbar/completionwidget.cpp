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

// Auto Includes
#include "rekonq.h"

// Local Includes
#include "application.h"
#include "urlresolver.h"
#include "searchengine.h"
#include "urlbar.h"

// KDE Includes
#include <KGlobalSettings>
#include <KUrl>

// Qt Includes
#include <QtCore/QPoint>
#include <QtCore/QSize>
#include <QtCore/QEvent>

#include <QtGui/QVBoxLayout>
#include <QtGui/QKeyEvent>



CompletionWidget::CompletionWidget(QWidget *parent)
        : QFrame(parent, Qt::ToolTip)
        , _parent(parent)
        , _currentIndex(0)
        , _searchEngine(SearchEngine::defaultEngine())
        , _hasSuggestions(false)
{
    setFrameStyle(QFrame::Panel);
    setLayoutDirection(Qt::LeftToRight);
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setMargin(0);
    layout->setSpacing(0);
    setLayout(layout);
}


void CompletionWidget::insertSearchList(const UrlSearchList &list, const QString& text)
{
    if (!isVisible())
    {
        _searchEngine = SearchEngine::defaultEngine();
    }

    _list = list;
    int i = 0;
    foreach(const UrlSearchItem &item, _list)
    {
        ListItem *suggestion = ListItemFactory::create(item, text, this);
        suggestion->setBackgroundRole(i % 2 ? QPalette::AlternateBase : QPalette::Base);
        connect(suggestion, 
                SIGNAL(itemClicked(ListItem *, Qt::MouseButton, Qt::KeyboardModifiers)), 
                this, 
                SLOT(itemChosen(ListItem *, Qt::MouseButton, Qt::KeyboardModifiers)));
        connect(this, SIGNAL(nextItemSubChoice()), suggestion, SLOT(nextItemSubChoice()));
        
        suggestion->setObjectName(QString::number(i++));
        layout()->addWidget(suggestion);
    }
}


void CompletionWidget::updateSearchList(const UrlSearchList &list, const QString& text)
{
    if(_hasSuggestions || _typedString != text || !isVisible())
        return;
    _hasSuggestions = true;
    
    UrlSearchList sugList = list.mid(0,4);

    // add new suggestions to the list
    int offset = _list.count();
    Q_FOREACH(const UrlSearchItem &item, sugList)
    {
        ListItem *suggestion = ListItemFactory::create(item, text, this);
        suggestion->setBackgroundRole(offset % 2 ? QPalette::AlternateBase : QPalette::Base);
        connect(suggestion, 
                SIGNAL(itemClicked(ListItem *, Qt::MouseButton, Qt::KeyboardModifiers)), 
                this, 
                SLOT(itemChosen(ListItem *, Qt::MouseButton, Qt::KeyboardModifiers)));
        connect(this, SIGNAL(nextItemSubChoice()), suggestion, SLOT(nextItemSubChoice()));
        
        suggestion->setObjectName(QString::number(offset++));
        layout()->addWidget(suggestion);
    }
    _list.append(sugList);
    sizeAndPosition();
}


void CompletionWidget::sizeAndPosition()
{
    setFixedWidth(_parent->width());

    int h = 0;
    for (int i = 0; i < layout()->count(); i++)
    {
        QWidget *widget = layout()->itemAt(i)->widget();
        h += widget->sizeHint().height();
    }
    setFixedSize(_parent->width(),h+5);

    // position
    QPoint p = _parent->mapToGlobal(QPoint(0, 0));
    move(p.x(), p.y() + _parent->height());
}


void CompletionWidget::popup()
{
    findChild<ListItem *>(QString::number(0))->activate(); //activate first listitem
    sizeAndPosition();
    if (!isVisible())
        show();
}


void CompletionWidget::up()
{
    // deactivate previous
    findChild<ListItem *>(QString::number(_currentIndex))->deactivate(); // deactivate previous

    if (_currentIndex > 0)
        _currentIndex--;
    else
        _currentIndex = layout()->count() - 1;

    activateCurrentListItem();
}


void CompletionWidget::down()
{
    findChild<ListItem *>(QString::number(_currentIndex))->deactivate(); // deactivate previous

    if (_currentIndex < _list.count() - 1)
        _currentIndex++;
    else
        _currentIndex = 0;

    activateCurrentListItem();
}


void CompletionWidget::activateCurrentListItem()
{
    UrlBar *bar = qobject_cast<UrlBar *>(_parent);

    // activate "new" current
    ListItem *widget = findChild<ListItem *>(QString::number(_currentIndex));
    widget->activate();

    //update text of the url bar
    bar->blockSignals(true); //without compute suggestions
    bar->setQUrl(widget->text());
    bar->blockSignals(false);
    bar->setFocus();
    bar->setCursorPosition(bar->text().length());
}


void CompletionWidget::clear()
{
    QLayoutItem *child;
    while ((child = layout()->takeAt(0)) != 0)
    {
        delete child->widget();
        delete child;
    }
    _currentIndex = 0;
    _hasSuggestions = false;
}


bool CompletionWidget::eventFilter(QObject *obj, QEvent *ev)
{
    int type = ev->type();
    QWidget *wid = qobject_cast<QWidget*>(obj);

    if (obj == this)
    {
        return false;
    }

    // hide conditions of the CompletionWidget
    if (wid
        && ((wid == _parent 
            && (type == QEvent::Move || type == QEvent::Resize)) || ((wid->windowFlags() & Qt::Window)
            && (type == QEvent::Move || type == QEvent::Hide || type == QEvent::WindowDeactivate)
            && wid == _parent->window()) || (type == QEvent::MouseButtonPress && !isAncestorOf(wid)))
       )
    {
        hide();
        return false;
    }

    //actions on the CompletionWidget
    if (wid && wid->isAncestorOf(_parent) && isVisible())
    {
        ListItem *child;
        UrlBar *w;
        
        if (type == QEvent::KeyPress)
        {
            QKeyEvent *kev = static_cast<QKeyEvent *>(ev);
            switch (kev->key())
            {
            case Qt::Key_Up:
            case Qt::Key_Backtab:
                if (kev->modifiers() == Qt::NoButton || (kev->modifiers() & Qt::ShiftModifier))
                {
                    up();
                    kev->accept();
                    return true;
                }
                break;

            case Qt::Key_Down:
            case Qt::Key_Tab:
                if (kev->modifiers() == Qt::NoButton)
                {
                    down();
                    kev->accept();
                    return true;
                }
                if (kev->modifiers() & Qt::ControlModifier)
                {
                    emit nextItemSubChoice();
                    kev->accept();
                    return true;
                }
                break;

            case Qt::Key_Enter:
            case Qt::Key_Return:
                w = qobject_cast<UrlBar *>(parent());
                if( _currentIndex == -1)
                    _currentIndex = 0;
                child = findChild<ListItem *>(QString::number(_currentIndex));
                if(child)
                {
                    emit chosenUrl(child->url(), Rekonq::CurrentTab);
                }
                else
                {
                    // this will be used just on fast typing..
                    emit chosenUrl(KUrl(w->text()), Rekonq::CurrentTab);
                }
                kev->accept();
                hide();
                return true;
                
            case Qt::Key_Escape:
                hide();
                return true;
            }
        }
    }

    return QFrame::eventFilter(obj, ev);
}


void CompletionWidget::setVisible(bool visible)
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


void CompletionWidget::itemChosen(ListItem *item, Qt::MouseButton button, Qt::KeyboardModifiers modifier)
{
    if (button == Qt::MidButton || modifier == Qt::ControlModifier)
        emit chosenUrl(item->url(), Rekonq::NewFocusedTab);
    else
        emit chosenUrl(item->url(), Rekonq::CurrentTab);
    hide();
}


void CompletionWidget::suggestUrls(const QString &text)
{
    _typedString = text;
    
    QWidget *w = qobject_cast<QWidget *>(parent());
    if (!w->hasFocus())
        return;

    if (text.isEmpty())
    {
        hide();
        return;
    }

    UrlResolver *res = new UrlResolver(text);
    connect(res, SIGNAL(suggestionsReady(const UrlSearchList &, const QString &)), this, SLOT(updateSearchList(const UrlSearchList &, const QString &)));
    UrlSearchList list = res->orderedSearchItems();
    if (list.count() > 0)
    {
        clear();
        insertSearchList(list, text);
        popup();
    }
}
