/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2012 by Andrea Diamantini <adjam7 at gmail dot com>
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
#include "searchenginebar.h"
#include "searchenginebar.moc"

// Auto Includes
#include "rekonq.h"

// KDE Includes
#include <KIcon>
#include <KIconLoader>
#include <KAction>
#include <KLocalizedString>

// Qt Includes
#include <QProcess>


SearchEngineBar::SearchEngineBar(QWidget *parent)
    : KMessageWidget(parent)
{
    connect(this, SIGNAL(accepted()), this, SLOT(hideAndDelete()));
    connect(this, SIGNAL(accepted()), this, SLOT(slotAccepted()));

    connect(this, SIGNAL(rejected()), this, SLOT(hideAndDelete()));
    connect(this, SIGNAL(rejected()), this, SLOT(slotRejected()));

    setMessageType(KMessageWidget::Information);

    QSize sz = size();
    sz.setWidth(qobject_cast<QWidget *>(parent)->size().width());
    resize(sz);

    setCloseButtonVisible(false);

    setText(i18n("You do not have a default search engine set. Without it, rekonq will not show proper URL suggestions."));

    KAction *acceptAction = new KAction(i18n("Set it"), this);
    connect(acceptAction, SIGNAL(triggered(bool)), this, SIGNAL(accepted()));
    addAction(acceptAction);

    KAction *rejectAction = new KAction(i18n("Ignore"), this);
    connect(rejectAction, SIGNAL(triggered(bool)), this, SIGNAL(rejected()));
    addAction(rejectAction);
}


void SearchEngineBar::hideAndDelete()
{
    animatedHide();
    deleteLater();
}


void SearchEngineBar::slotAccepted()
{
    QProcess *proc = new QProcess(parent());
    QStringList args;
    args << QL1S("ebrowsing");
    proc->start(QL1S("kcmshell4"), args);
}


void SearchEngineBar::slotRejected()
{
    // Remember users choice
    ReKonfig::setCheckDefaultSearchEngine(false);
}
