/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2009-2013 by Andrea Diamantini <adjam7 at gmail dot com>
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
#include "searchengine.h"
#include "urlresolver.h"

#include "listitem.h"
#include "urlbar.h"

// KDE Includes
#include <KGlobalSettings>
#include <KUrl>

// Qt Includes
#include <QApplication>
#include <QPoint>
#include <QSize>
#include <QEvent>

#include <QVBoxLayout>
#include <QKeyEvent>



CompletionWidget::CompletionWidget(QWidget *parent)
    : QFrame(parent, Qt::ToolTip)
    , _parent(parent)
    , _currentIndex(0)
    , _hasSuggestions(false)
{
    setFrameStyle(QFrame::Panel);
    setLayoutDirection(Qt::LeftToRight);
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setMargin(0);
    layout->setSpacing(0);
    setLayout(layout);
}


void CompletionWidget::insertItems(const UrlSuggestionList &list, const QString& text, int offset)
{
    Q_FOREACH(const UrlSuggestionItem & item, list)
    {
        ListItem *suggestion = ListItemFactory::create(item, text, this);
        suggestion->setBackgroundRole(offset % 2 ? QPalette::AlternateBase : QPalette::Base);

        connect(suggestion,
                SIGNAL(itemClicked(ListItem*,Qt::MouseButton,Qt::KeyboardModifiers)),
                this,
                SLOT(itemChosen(ListItem*,Qt::MouseButton,Qt::KeyboardModifiers)));

        connect(this, SIGNAL(nextItemSubChoice()), suggestion, SLOT(nextItemSubChoice()));

        suggestion->setObjectName(QString::number(offset++));
        layout()->addWidget(suggestion);
    }
}


void CompletionWidget::updateSuggestionList(const UrlSuggestionList &list, const QString& text)
{    
    if (_hasSuggestions || _typedString != text)
        return;
    _hasSuggestions = true;

    if (list.count() > 0)
    {
        clear();

        insertItems(list, text);
        _list = list;

        popup();
    }
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
    setFixedSize(_parent->width(), h + 5);

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
    if (_currentIndex >= 0)
        findChild<ListItem *>(QString::number(_currentIndex))->deactivate(); // deactivate previous

    --_currentIndex;
    if (_currentIndex < -1)
    {
        _currentIndex = _list.count() - 1;
    }

    activateCurrentListItem();
}


void CompletionWidget::down()
{
    if (_currentIndex >= 0)
        findChild<ListItem *>(QString::number(_currentIndex))->deactivate(); // deactivate previous

    ++_currentIndex;
    if (_currentIndex == _list.count())
        _currentIndex = -1;

    activateCurrentListItem();
}


void CompletionWidget::activateCurrentListItem()
{
    UrlBar *bar = qobject_cast<UrlBar *>(_parent);

    // activate "new" current
    ListItem *widget = findChild<ListItem *>(QString::number(_currentIndex));

    // update text of the url bar
    bar->blockSignals(true); // without compute suggestions
    if (widget)
    {
        widget->activate();
        bar->setQUrl(widget->text());
    }
    else
    {
        bar->setText(_typedString);
    }
    bar->blockSignals(false);
    bar->setFocus();
    bar->setCursorPosition(bar->text().length());
}


void CompletionWidget::clear()
{
    QLayoutItem *child;
    while ((child = layout()->takeAt(0)) != 0)
    {
        child->widget()->deleteLater();
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
            {
                w = qobject_cast<UrlBar *>(parent());

                if (kev->modifiers() == Qt::ControlModifier)
                {
                    QString append = QL1S(".com");
                    QUrl url(QL1S("http://") + w->text());
                    QString host = url.host();
                    if (!host.endsWith(append, Qt::CaseInsensitive))
                    {
                        host += append;
                        url.setHost(host);
                    }

                    if (url.isValid())
                    {
                        emit chosenUrl(url, Rekonq::CurrentTab);
                        kev->accept();
                        hide();
                        return true;
                    }
                }

                KUrl urlToLoad;
                Rekonq::OpenType type = Rekonq::CurrentTab;

                if (_currentIndex == -1)
                    _currentIndex = 0;
                child = findChild<ListItem *>(QString::number(_currentIndex));

                if (child) //the completionwidget is visible and the user had press down
                {
                    urlToLoad = child->url();
                }
                else //the user type too fast (completionwidget not visible or suggestion not downloaded)
                {
                    urlToLoad = UrlResolver::urlFromTextTyped(w->text());
                    kDebug() << "Fast typer for text: " << w->text();
                }

                if (kev->modifiers() == Qt::AltModifier)
                {
                    if (kev->key() == Qt::Key_Return || kev->key() == Qt::Key_Enter)
                    {
                        type = Rekonq::NewFocusedTab;
                    }
                }

                emit chosenUrl(urlToLoad, type);
                kev->accept();
                hide();

                if (type != Rekonq::CurrentTab)
                    w->clear();

                return true;
            }

            case Qt::Key_Escape:
                hide();

                w = qobject_cast<UrlBar *>(parent());
                w->setText(_typedString);

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
        qApp->installEventFilter(this);
    }
    else
    {
        qApp->removeEventFilter(this);
        clear();
    }

    QFrame::setVisible(visible);
}


void CompletionWidget::itemChosen(ListItem *item, Qt::MouseButton button, Qt::KeyboardModifiers modifier)
{
    KUrl u = item->url();

    if (button == Qt::MidButton || modifier == Qt::ControlModifier)
    {
        emit chosenUrl(u, Rekonq::NewFocusedTab);
    }
    else
    {
        emit chosenUrl(u, Rekonq::CurrentTab);
    }
    
    // do it AFTER launching chosenUrl to get sure item exists
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

    UrlSuggester *res = new UrlSuggester(text);
    UrlSuggestionList list = res->computeSuggestions();

    updateSuggestionList(list, text);

    delete res;
}


KUrl CompletionWidget::activeSuggestion()
{
    int index = _currentIndex;
    if (_currentIndex == -1)
        index = 0;

    ListItem *child = findChild<ListItem *>(QString::number(index));
    if (child)
        return child->url();

    kDebug() << "WARNING: NO URL to LOAD...";
    return KUrl();
}
