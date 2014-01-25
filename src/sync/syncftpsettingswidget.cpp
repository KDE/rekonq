/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2011-2014 by Andrea Diamantini <adjam7 at gmail dot com>
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
#include "syncftpsettingswidget.h"

// Auto Includes
#include "rekonq.h"

// Local Includes
#include "syncassistant.h"


SyncFTPSettingsWidget::SyncFTPSettingsWidget(QWidget *parent)
    : QWizardPage(parent)
{
    setupUi(this);
    
    int port = ReKonfig::syncPort();
    if (port == -1)
    {
        port = 21;
    }
    
    kcfg_syncHost->setText(ReKonfig::syncHost());
    kcfg_syncUser->setText(ReKonfig::syncUser());
    kcfg_syncPass->setText(ReKonfig::syncPass());
    kcfg_syncPath->setText(ReKonfig::syncPath());
    kcfg_syncPort->setValue(port);

    kcfg_syncPass->setPasswordMode(true);
}


int SyncFTPSettingsWidget::nextId() const
{
    // save
    ReKonfig::setSyncHost(kcfg_syncHost->text());
    ReKonfig::setSyncUser(kcfg_syncUser->text());
    ReKonfig::setSyncPass(kcfg_syncPass->text());
    ReKonfig::setSyncPath(kcfg_syncPath->text());
    ReKonfig::setSyncPort(kcfg_syncPort->value());

    return SyncAssistant::Page_Data;
}
