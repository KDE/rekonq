/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2010-2011 by Pierre Rossi <pierre dot rossi at gmail dot com>
* Copyright (C) 2011 by Andrea Diamantini <adjam7 at gmail dot com>
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
#include "messagebar.h"
#include "messagebar.moc"

// KDE Includes
#include <KIcon>
#include <KIconLoader>
#include <KLocalizedString>

// Qt Includes
#include <QAction>


MessageBar::MessageBar(const QString &message, QWidget *parent)
    : KMessageWidget(parent)
{
    connect(this, SIGNAL(accepted()), this, SLOT(hideAndDelete()));
    connect(this, SIGNAL(rejected()), this, SLOT(hideAndDelete()));
    
    setMessageType(KMessageWidget::Error);
    
    QSize sz = size();
    sz.setWidth( qobject_cast<QWidget *>(parent)->size().width() );
    resize(sz);
    
    setCloseButtonVisible(false);
    
    setText( message );

    QAction *acceptAction = new QAction( i18n("Yes"), this );
    connect(acceptAction, SIGNAL(triggered(bool)), this, SIGNAL(accepted()));
    addAction(acceptAction);

    QAction *rejectAction = new QAction( i18n("No"), this );
    connect(rejectAction, SIGNAL(triggered(bool)), this, SIGNAL(rejected()));
    addAction(rejectAction);
}


void MessageBar::hideAndDelete()
{
    animatedHide();
    deleteLater();
}
