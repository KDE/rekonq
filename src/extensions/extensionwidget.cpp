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
#include "extensionwidget.h"
#include "extensionwidget.moc"

// Auto Includes
#include "rekonq.h"

// Local Includes
#include "extensionmanager.h"
#include "extension.h"
#include "extensionwidgetitem.h"

// KDE Includes
#include <KFileDialog>
#include <KMessageBox>
#include <KStandardDirs>
#include <KIcon>

// Qt Includes
#include <QString>
#include <QWhatsThis>
#include <QListWidgetItem>


ExtensionWidget::ExtensionWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUi(this);
    
    setMinimumSize(600,450);
    
    listWidget->setAlternatingRowColors(true);
    listWidget->setVerticalScrollMode(QListView::ScrollPerPixel);
    
    loadButton->setText( i18n("Load") );
    loadButton->setIcon( KIcon("document-open") );
    connect(loadButton, SIGNAL(clicked()), this, SLOT(loadUnpackedExtension()));
    
    ghnsButton->setText( i18n("Get New Extensions...") );
    ghnsButton->setIcon( KIcon("get-hot-new-stuff") );
    connect(ghnsButton, SIGNAL(clicked()), this, SLOT(chooseExtension()));
    
    load();
}


void ExtensionWidget::load()
{
    KSharedConfig::Ptr extensionConfig = KSharedConfig::openConfig("extensions", KConfig::SimpleConfig, "appdata");

    QStringList extIdList = extensionConfig->groupList();
    
    Q_FOREACH(const QString &idString, extIdList)
    {
        Extension *ext = ExtensionManager::self()->extension(idString);
        if (!ext)
            continue;
        
        QListWidgetItem *item = new QListWidgetItem();       
        listWidget->addItem(item);
        item->setSizeHint(QSize(0,48));
        ExtensionWidgetItem *wItem = new ExtensionWidgetItem(ext, listWidget);
        listWidget->setItemWidget(item, wItem);
    }
}


void ExtensionWidget::loadUnpackedExtension()
{
    QString extPath = KFileDialog::getExistingDirectory();
    QString actualDir = extPath.section( QL1C('/'), -1 );
    int n = extPath.count() - actualDir.count();
    extPath.truncate(n);
    
    kDebug() << extPath;
    kDebug() << actualDir;
    
    Extension* ext = ExtensionManager::self()->addExtension(extPath, actualDir, true);

    if (ext)
    {
        QListWidgetItem *item = new QListWidgetItem();       
        listWidget->addItem(item);
        item->setSizeHint(QSize(0,48));
        ExtensionWidgetItem *wItem = new ExtensionWidgetItem(ext, listWidget);
        listWidget->setItemWidget(item, wItem);
        
        KSharedConfig::Ptr extensionConfig = KSharedConfig::openConfig("extensions", KConfig::SimpleConfig, "appdata");
        KConfigGroup extGroup(extensionConfig, actualDir);

        extGroup.writeEntry("path", extPath);
        extGroup.writeEntry("enabled", ext->isEnabled());

        return;
    }
    
    KMessageBox::error( this,  i18n("Problems loading extension %1 from dir %2", actualDir, extPath) );
}


void ExtensionWidget::chooseExtension()
{
    kDebug() << "Needs implementation...";
}
