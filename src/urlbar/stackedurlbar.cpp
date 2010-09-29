/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2010 by Andrea Diamantini <adjam7 at gmail dot com>
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
#include "stackedurlbar.h"
#include "stackedurlbar.moc"

// Local Includes
#include "urlbar.h"


StackedUrlBar::StackedUrlBar(QWidget *parent)
        : QStackedWidget(parent)
{
    // cosmetic
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setMinimumWidth(200);
    setMinimumHeight(26);   // FIXME in Qt 4.7 we can probably move using MinimumWidth 20
}


StackedUrlBar::~StackedUrlBar()
{
}


UrlBar *StackedUrlBar::currentUrlBar()
{
    return urlBar(currentIndex());
}


UrlBar *StackedUrlBar::urlBar(int index)
{
    UrlBar *urlBar = qobject_cast<UrlBar*>(QStackedWidget::widget(index));
    if (!urlBar)
    {
        kWarning() << "URL bar with index" << index << "not found. Returning NULL.  line:" << __LINE__;
    }

    return urlBar;
}


void StackedUrlBar::moveBar(int from, int to)
{
    QWidget *fromBar = widget(from);
    removeWidget(fromBar);
    insertWidget(to, fromBar);
}
