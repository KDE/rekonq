/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2013 by Andrea Diamantini <adjam7 at gmail dot com>
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


// Auto Includes
#include "extensionpopup.h"
#include "extensionpopup.moc"

#include <QSizePolicy>
#include <QWebFrame>
#include <QVBoxLayout>


ExtensionPopup::ExtensionPopup(const QUrl &url, QWidget *parent)
    : QWidget(parent, Qt::Popup)
    , _view(0)
    , _url(url)
{
    setAttribute(Qt::WA_DeleteOnClose);

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->setMargin(0);
    layout->setSpacing(0);
        
    _view = new QWebView(this);
    _view->setContentsMargins(0,0,0,0);
    
    QWebFrame *frame = _view->page()->mainFrame();
    connect(frame, SIGNAL(contentsSizeChanged(QSize)), this, SLOT(changeContentSize(QSize)));
    
    layout->addWidget(_view);
    setLayout(layout);
    setContentsMargins(0,0,0,0);
       
    _view->load(_url);
}


void ExtensionPopup::showAt(const QPoint &pos)
{
    adjustSize();

    int l = size().width();
    QPoint p(pos.x() - l + 10, pos.y() + 10);
    move(p);
    show();
}


void ExtensionPopup::changeContentSize(QSize newSize)
{
    _view->resize(newSize);
    adjustSize();
}
