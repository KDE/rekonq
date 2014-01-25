/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2012-2014 by Andrea Diamantini <adjam7 at gmail dot com>
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


// Local Includes
#include "advancedwidget.h"

// Qt Includes
#include <QProcess>


AdvancedWidget::AdvancedWidget(QWidget *parent)
    : QWidget(parent)
    , _changed(false)
{
    setupUi(this);

    connect(proxyButton, SIGNAL(clicked()), this, SLOT(launchProxySettings()));
}


void AdvancedWidget::save()
{
}


bool AdvancedWidget::changed()
{
    return _changed;
}


void AdvancedWidget::hasChanged()
{
    _changed = true;
    emit changed(true);
}


void AdvancedWidget::launchProxySettings()
{
    QString program = QL1S("kcmshell4");
    QStringList arguments;
    arguments << QL1S("proxy");
    QProcess *proc = new QProcess(this);
    proc->start(program, arguments);
}
