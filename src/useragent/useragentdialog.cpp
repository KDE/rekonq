/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2010-2014 by Andrea Diamantini <adjam7 at gmail dot com>
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
#include "useragentdialog.h"

// KDE Includes
#include <KSharedConfig>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KProtocolManager>


UserAgentDialog::UserAgentDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(i18nc("@title:window", "User Agent Settings"));

    // the button box
    _buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok);    
    connect(_buttonBox, SIGNAL(accepted()), this, SLOT(accept()));

    // the user agent widget
    QWidget *widget = new QWidget(this);
    _userAgent.setupUi(widget);
    
    // insert everything inside the dialog...
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(widget);
    mainLayout->addWidget(_buttonBox);
    setLayout(mainLayout);

    // ------------------
    connect(_userAgent.deleteButton, SIGNAL(clicked()), this, SLOT(deleteUserAgent()));
    connect(_userAgent.deleteAllButton, SIGNAL(clicked()), this, SLOT(deleteAll()));

    KSharedConfig::Ptr config = KSharedConfig::openConfig( QL1S("kio_httprc"), KConfig::NoGlobals);

    QStringList hosts = config->groupList();
    Q_FOREACH(const QString & host, hosts)
    {
        QStringList tmp;
        tmp << host;

        KConfigGroup hostGroup(config, host);
        tmp <<  hostGroup.readEntry(QL1S("UserAgent"), QString());

        QTreeWidgetItem *item = new QTreeWidgetItem(_userAgent.sitePolicyTreeWidget, tmp);
        _userAgent.sitePolicyTreeWidget->addTopLevelItem(item);
    }
}


void UserAgentDialog::deleteUserAgent()
{
    QTreeWidgetItem *item = _userAgent.sitePolicyTreeWidget->currentItem();
    if (!item)
        return;

    _userAgent.sitePolicyTreeWidget->takeTopLevelItem(_userAgent.sitePolicyTreeWidget->indexOfTopLevelItem(item));

    QString host = item->text(0);

    KSharedConfig::Ptr config = KSharedConfig::openConfig( QL1S("kio_httprc"), KConfig::NoGlobals);
    KConfigGroup group(config, host);
    if (group.exists())
    {
        group.deleteGroup();
        KProtocolManager::reparseConfiguration();
    }
}


void UserAgentDialog::deleteAll()
{
    _userAgent.sitePolicyTreeWidget->clear();

    KSharedConfig::Ptr config = KSharedConfig::openConfig( QL1S("kio_httprc"), KConfig::NoGlobals);

    QStringList list = config->groupList();
    Q_FOREACH(const QString & groupName, list)
    {
        KConfigGroup group(config, groupName);
        group.deleteGroup();
    }
    KConfigGroup group(config, QString());
    group.deleteGroup();

    KProtocolManager::reparseConfiguration();
}
