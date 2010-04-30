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

// KDE Includes
#include <KGlobalSettings>
#include <KUrl>

// Qt Includes
#include <QPoint>
#include <QSize>
#include <QVBoxLayout>
#include <QString>
#include <QEvent>
#include <QKeyEvent>


CompletionWidget::CompletionWidget(QWidget *parent)
        : QFrame(parent, Qt::ToolTip)
        , _parent(parent)
        , _currentIndex(-1)
        , _searchEngine(SearchEngine::defaultEngine())
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
    _list = list;
    int i = 0;
    foreach(UrlSearchItem item, _list)
    {
        ListItem *suggestion = ListItemFactory::create(item, text, this);
        suggestion->setBackgroundRole(i % 2 ? QPalette::AlternateBase : QPalette::Base);
        connect(suggestion, SIGNAL(itemClicked(ListItem *, Qt::MouseButton)), this, SLOT(itemChosen(ListItem *, Qt::MouseButton)));
        connect(this, SIGNAL(nextItemSubChoice()), suggestion, SLOT(nextItemSubChoice()));
        suggestion->setObjectName(QString::number(i++));
        layout()->addWidget(suggestion);
    }
}


void CompletionWidget::sizeAndPosition()
{
    setFixedWidth(_parent->width());
    adjustSize();

    // position
    QPoint p = _parent->mapToGlobal(QPoint(0, 0));
    move(p.x(), p.y() + _parent->height());
}


void CompletionWidget::popup()
{
    down();
    sizeAndPosition();
    if (!isVisible())
        show();
}


void CompletionWidget::up()
{
    // deactivate previous
    if (_currentIndex != -1)
    {
        ListItem *widget = findChild<ListItem *>(QString::number(_currentIndex));
        widget->deactivate();
    }

    if (_currentIndex > 0)
        _currentIndex--;
    else
        _currentIndex = layout()->count() - 1;

    // activate "new" current
    ListItem *widget = findChild<ListItem *>(QString::number(_currentIndex));
    widget->activate();
}


void CompletionWidget::down()
{
    // deactivate previous
    if (_currentIndex != -1)
    {
        ListItem *widget = findChild<ListItem *>(QString::number(_currentIndex));
        widget->deactivate();
    }

    if (_currentIndex < _list.count() - 1)
        _currentIndex++;
    else
        _currentIndex = 0;

    // activate "new" current
    ListItem *widget = findChild<ListItem *>(QString::number(_currentIndex));
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


bool CompletionWidget::eventFilter(QObject *o, QEvent *e)
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
        ListItem *child;

        if (type == QEvent::KeyPress)
        {
            QKeyEvent *ev = static_cast<QKeyEvent *>(e);
            switch (ev->key())
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
                if (ev->modifiers() & Qt::ControlModifier)
                {
                    emit nextItemSubChoice();
                    ev->accept();
                    return true;
                }
                break;

            case Qt::Key_Enter:
            case Qt::Key_Return:
                child = findChild<ListItem *>(QString::number(_currentIndex));
                emit chosenUrl(child->url(), Rekonq::CurrentTab);
                ev->accept();
                hide();
                return true;

            case Qt::Key_Escape:
                hide();
                return true;
            }
        }
    }

    return QFrame::eventFilter(o, e);
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


void CompletionWidget::itemChosen(ListItem *item, Qt::MouseButton button)
{
    if (button == Qt::MidButton)
        emit chosenUrl(item->url(), Rekonq::NewCurrentTab);
    else
        emit chosenUrl(item->url(), Rekonq::CurrentTab);
    hide();
}


void CompletionWidget::suggestUrls(const QString &text)
{
    QWidget *w = qobject_cast<QWidget *>(parent());
    if (!w->hasFocus())
        return;

    if (text.isEmpty())
    {
        hide();
        return;
    }

    UrlResolver res(text);
    UrlSearchList list = res.orderedSearchItems();
    if (list.count() > 0)
    {
        clear();
        insertSearchList(list, text);
        popup();
    }
}


