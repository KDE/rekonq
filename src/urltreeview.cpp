/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2010 by Yoann Laissus <yoann dot laissus at gmail dot com>
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
#include "urltreeview.h"

// Local Includes
#include "application.h"

// Qt Includes
#include <QMouseEvent>
#include <QClipboard>


UrlTreeView::UrlTreeView(QWidget *parent)
    : QTreeView(parent)
{
    connect(this, SIGNAL(itemHovered(const QString &)), parent, SIGNAL(itemHovered(const QString &)));
    connect(this, SIGNAL(openUrl(const KUrl &, Rekonq::OpenType)), parent, SIGNAL(openUrl(const KUrl &, Rekonq::OpenType)));
    setMouseTracking(true);
    setExpandsOnDoubleClick(false);
}


UrlTreeView::~UrlTreeView()
{
}


void UrlTreeView::mousePressEvent(QMouseEvent *event)
{
    const QModelIndex index = indexAt(event->pos());
    bool expanded = isExpanded(index);

    QTreeView::mousePressEvent(event);

    // A change of an item expansion is handle by mouseReleaseEvent()
    // So toggle again the item
    if(expanded != isExpanded(index))
        setExpanded(index, !isExpanded(index));
    
    if(!index.isValid())
    {
        clearSelection();
        setCurrentIndex(QModelIndex());

        if(event->button() == Qt::RightButton)
            emit contextMenuEmptyRequested(event->pos());
        return;
    }

    if(event->button() == Qt::RightButton)
    {
        if(model()->rowCount(index) == 0)
        {
            // An empty group needs to be handle by the panels
            emit contextMenuItemRequested(event->pos());
        }
        else
        {
            emit contextMenuGroupRequested(event->pos());
        }
    }
}


void UrlTreeView::mouseReleaseEvent(QMouseEvent *event)
{
    QTreeView::mouseReleaseEvent(event);

    const QModelIndex index = indexAt(event->pos());
    if(!index.isValid())
        return;

    if(event->button() == Qt::MidButton)
        validOpenUrl(qVariantValue< KUrl >(index.data(Qt::UserRole)), Rekonq::NewCurrentTab);

    else if(event->button() == Qt::LeftButton)
    {
        if(model()->rowCount(index) == 0)
            validOpenUrl(qVariantValue< KUrl >(index.data(Qt::UserRole)));
        else
            setExpanded(index, !isExpanded(index));
    }
}


void UrlTreeView::keyPressEvent(QKeyEvent *event)
{
    QTreeView::keyPressEvent(event);
    QModelIndex index = currentIndex();

    if(!index.isValid())
        return;

    if(event->key() == Qt::Key_Return)
    {
        if(model()->rowCount(index) == 0)
            validOpenUrl(qVariantValue< KUrl >(index.data(Qt::UserRole)));
        else
            setExpanded(index, !isExpanded(index));
    }

    else if(event->key() == Qt::Key_Delete)
    {
        emit delKeyPressed();
    }
}


void UrlTreeView::validOpenUrl(const KUrl &url, Rekonq::OpenType openType)
{
    // To workaround a crash when the url is about:blank
    if(url.url() == "about:blank")
        emit openUrl(KUrl("about:home"), openType);
    else
        emit openUrl(url, openType);
}


void UrlTreeView::mouseMoveEvent(QMouseEvent *event)
{
    QTreeView::mouseMoveEvent(event);
    const QModelIndex index = indexAt(event->pos());
    if(!index.isValid())
    {
        emit itemHovered("");
        return;
    }
    emit itemHovered(qVariantValue< KUrl >(index.data(Qt::UserRole)).url());
}


void UrlTreeView::openInCurrentTab()
{
    QModelIndex index = currentIndex();
    if(!index.isValid())
        return;

    validOpenUrl(qVariantValue< KUrl >(index.data(Qt::UserRole)));
}


void UrlTreeView::copyToClipboard()
{
    QModelIndex index = currentIndex();
    if(!index.isValid())
        return;

    QClipboard *cb = QApplication::clipboard();
    cb->setText(qVariantValue< KUrl >(index.data(Qt::UserRole)).url());
}


void UrlTreeView::openInNewTab()
{
    QModelIndex index = currentIndex();
    if(!index.isValid())
        return;

    validOpenUrl(qVariantValue< KUrl >(index.data(Qt::UserRole)), Rekonq::SettingOpenTab);
}


void UrlTreeView::openInNewWindow()
{
    QModelIndex index = currentIndex();
    if(!index.isValid())
        return;

    validOpenUrl( qVariantValue< KUrl >(index.data(Qt::UserRole)), Rekonq::NewWindow);
}
