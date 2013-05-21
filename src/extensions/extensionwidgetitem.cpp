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


// Self Includes
#include "extensionwidgetitem.h"
#include "extensionwidgetitem.moc"

// Local Includes
#include "extensionmanager.h"

// KDE Includes
#include <KConfig>
#include <KConfigGroup>

// Qt Includes
#include <QHBoxLayout>
#include <QVBoxLayout>

#include <QCheckBox>
#include <QLabel>
#include <QIcon>

#include <QString>


ExtensionWidgetItem::ExtensionWidgetItem(Extension *ext, QWidget *parent)
    : QWidget(parent)
    , _extension(ext)
{
    QCheckBox *checkBox = new QCheckBox(this);
    checkBox->setChecked(ext->isEnabled());
    connect(checkBox, SIGNAL(toggled(bool)), this, SLOT(toogleExtensionState(bool)));
    
    QLabel *icon = new QLabel(this);

    _extPath = ext->extensionPath();
    _extId = ext->id();
    QIcon ic = QIcon(_extPath + _extId + ext->icon());
    QPixmap px = ic.pixmap(32);
    if (px.isNull())
    {
        KIcon kic = KIcon("applications-other");
        icon->setPixmap(kic.pixmap(32));
    }
    else
    {
        icon->setPixmap(px);
    }
    
    QLabel *name = new QLabel( QL1S("<b>") + ext->name() +  QL1S("<b>"), this);
    QLabel *desc = new QLabel( QL1S("<i>") + ext->description() +  QL1S("<i>"), this);
    
    QVBoxLayout *vLayout = new QVBoxLayout;
    vLayout->addWidget(name);
    vLayout->addWidget(desc);
    
    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->addWidget(checkBox);
    hLayout->addWidget(icon);
    hLayout->addLayout(vLayout, 5);
    setLayout(hLayout);
}


void ExtensionWidgetItem::toogleExtensionState(bool on)
{
    kDebug() << "Toogle: " << on;
    _extension->setEnabled(on);
    
    KSharedConfig::Ptr extensionConfig = KSharedConfig::openConfig("extensions", KConfig::SimpleConfig, "appdata");
    KConfigGroup extGroup(extensionConfig, _extension->id());

    extGroup.writeEntry("enabled", on);
}
