/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2013 by Radu Andries <admiral0 at tuxfamily dot org>
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
#include "syncsshsettingswidget.h"
#include "syncsshsettingswidget.moc"

// Auto Includes
#include "rekonq.h"

#include <config-qca2.h>

// Local Includes
#include "syncassistant.h"


int SyncSSHSettingsWidget::nextId() const
{
    // save
    ReKonfig::setSyncHost(kcfg_syncHost->text());
    ReKonfig::setSyncUser(kcfg_syncUser->text());
    ReKonfig::setSyncPass(kcfg_syncPass->text());
    ReKonfig::setSyncPath(kcfg_syncPath->text());
    ReKonfig::setSyncPort(kcfg_syncPort->value());

    return SyncAssistant::Page_Data;
}


SyncSSHSettingsWidget::SyncSSHSettingsWidget(QWidget* parent): QWizardPage(parent)
{
    setupUi(this);
    
    int port = ReKonfig::syncPort();
    if (port == -1)
    {
        port = 22;
    }
    
    kcfg_syncHost->setText(ReKonfig::syncHost());
    kcfg_syncUser->setText(ReKonfig::syncUser());
    kcfg_syncPass->setText(ReKonfig::syncPass());
    kcfg_syncPath->setText(ReKonfig::syncPath());
    kcfg_syncPort->setValue(port);

    if (kcfg_syncPass->text().isEmpty())
    {
        syncWithSSHKeys->setChecked(true);
        toggleUserPass(true);
    }

    kcfg_syncPass->setPasswordMode(true);
    
    connect(syncWithSSHKeys,SIGNAL(toggled(bool)),SLOT(toggleUserPass(bool)));
}


void SyncSSHSettingsWidget::toggleUserPass(bool enabled)
{
    if (enabled)
    {
        kcfg_syncPass->setText(QL1S(""));
        kcfg_syncPass->setEnabled(false); 
    }
    else
    {
        kcfg_syncPass->setEnabled(true); 
    }
}
