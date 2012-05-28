/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2011-2012 by Andrea Diamantini <adjam7 at gmail dot com>
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
#include "syncdatawidget.h"
#include "syncdatawidget.moc"

// Auto Includes
#include "rekonq.h"

// Local Includes
#include "syncmanager.h"
#include "syncassistant.h"


SyncDataWidget::SyncDataWidget(QWidget *parent)
    : QWizardPage(parent)
{
    setupUi(this);
}

void SyncDataWidget::initializePage()
{

    kcfg_syncBookmarks->setDisabled(true);
    kcfg_syncHistory->setDisabled(true);
    kcfg_syncPasswords->setDisabled(true);

    switch (ReKonfig::syncType())
    {
        //Ftp Sync Handler
    case 0:
        kcfg_syncBookmarks->setEnabled(true);
        kcfg_syncHistory->setEnabled(true);
        kcfg_syncPasswords->setEnabled(true);
        break;
        //Google Sync Handler
    case 1:
        //Opera Sync Handler
    case 2:
        kcfg_syncBookmarks->setEnabled(true);
        break;
    default:
        kDebug() << "Unknown sync type!";
    }

    kcfg_syncBookmarks->setChecked(ReKonfig::syncBookmarks());
    kcfg_syncHistory->setChecked(ReKonfig::syncHistory());
    kcfg_syncPasswords->setChecked(ReKonfig::syncPasswords());

}


int SyncDataWidget::nextId() const
{
    // save

    ReKonfig::setSyncBookmarks(kcfg_syncBookmarks->isChecked());
    ReKonfig::setSyncHistory(kcfg_syncHistory->isChecked());
    ReKonfig::setSyncPasswords(kcfg_syncPasswords->isChecked());

    return SyncAssistant::Page_Check;
}
