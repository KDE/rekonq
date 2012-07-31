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
#include "synchosttypewidget.h"
#include "synchosttypewidget.moc"

// Auto Includes
#include "rekonq.h"

// Local Includes
#include "syncassistant.h"


SyncHostTypeWidget::SyncHostTypeWidget(QWidget *parent)
    : QWizardPage(parent)
    , _changed(false)
{
    setupUi(this);
    connect(kcfg_syncEnabled, SIGNAL(clicked()), this, SLOT(hasChanged()));

    kcfg_syncEnabled->setChecked(ReKonfig::syncEnabled());
    if (ReKonfig::syncType() == 0)
        ftpRadioButton->setChecked(true);
    else if (ReKonfig::syncType() == 1)
        googleRadioButton->setChecked(true);
    else if (ReKonfig::syncType() == 2)
        operaRadioButton->setChecked(true);
    else
        nullRadioButton->setChecked(true);

    enablewidgets(ReKonfig::syncEnabled());
}


bool SyncHostTypeWidget::changed()
{
    return _changed;
}


void SyncHostTypeWidget::hasChanged()
{
    enablewidgets(kcfg_syncEnabled->isChecked());

    _changed = true;
    emit changed(true);
}

int SyncHostTypeWidget::nextId() const
{
    ReKonfig::setSyncEnabled(kcfg_syncEnabled->isChecked());

    if (!ReKonfig::syncEnabled())
        return SyncAssistant::Page_Check;

    // save
    if (ftpRadioButton->isChecked())
    {
        ReKonfig::setSyncType(0);
        return SyncAssistant::Page_FTP_Settings;
    }
    else if (googleRadioButton->isChecked())
    {
        ReKonfig::setSyncType(1);
        return SyncAssistant::Page_Google_Settings;
    }
    else if (operaRadioButton->isChecked())
    {
        ReKonfig::setSyncType(2);
        return SyncAssistant::Page_Opera_Settings;
    }
    else
    {
        ReKonfig::setSyncType(3);
        return SyncAssistant::Page_Check;
    }

}

void SyncHostTypeWidget::enablewidgets(bool b)
{
    hostGroupBox->setEnabled(b);
}
