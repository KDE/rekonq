/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2012 by Siteshwar Vashisht <siteshwar at gmail dot com>
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
#include "syncoperasettingswidget.h"

// Auto Includes
#include "rekonq.h"

// Config Includes
#include <config-qca2.h>
#include <config-qtoauth.h>

// Local Includes
#include "syncassistant.h"




SyncOperaSettingsWidget::SyncOperaSettingsWidget(QWidget *parent)
    : QWizardPage(parent)
{
    setupUi(this);

#if (defined HAVE_QCA2 && defined HAVE_QTOAUTH)

    kcfg_syncUser->setText(ReKonfig::syncUser());
    kcfg_syncPass->setText(ReKonfig::syncPass());

    kcfg_syncPass->setPasswordMode(true);

#else

    kcfg_syncUser->setEnabled(false);
    kcfg_syncPass->setEnabled(false);

    infoLabel->setText(i18n("Rekonq has been compiled without support for Opera Sync"));

#endif
}


int SyncOperaSettingsWidget::nextId() const
{
#if (defined HAVE_QCA2 && defined HAVE_QTOAUTH)

    // save
    ReKonfig::setSyncHost("http://link.opera.com/");
    ReKonfig::setSyncUser(kcfg_syncUser->text());
    ReKonfig::setSyncPass(kcfg_syncPass->text());

    ReKonfig::setSyncHistory(false);
    ReKonfig::setSyncPasswords(false);

    return SyncAssistant::Page_Data;

#else

    return SyncAssistant::Page_Check;

#endif
}
