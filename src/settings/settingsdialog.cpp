/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2008-2012 by Andrea Diamantini <adjam7 at gmail dot com>
* Copyright (C) 2009-2011 by Lionel Chauvin <megabigbug@yahoo.fr>
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
#include "settingsdialog.h"
#include "settingsdialog.moc"

// Auto Includes
#include "rekonq.h"

// Local Includes
#include "searchengine.h"

// Widget Includes
#include "advancedwidget.h"
#include "appearancewidget.h"
#include "generalwidget.h"
#include "privacywidget.h"
#include "tabswidget.h"
#include "webkitwidget.h"

// KDE Includes
#include <KConfig>
#include <KStandardDirs>
#include <KPageWidgetItem>
#include <KCModuleInfo>
#include <KCModuleProxy>

// Qt Includes
#include <QtGui/QWidget>


class Private
{
private:
    Private(SettingsDialog *parent);

private:
    GeneralWidget *generalWidg;
    TabsWidget *tabsWidg;
    AppearanceWidget *appearanceWidg;
    WebKitWidget *webkitWidg;
    PrivacyWidget *privacyWidg;
    AdvancedWidget *advancedWidg;

    KCModuleProxy *ebrowsingModule;

    friend class SettingsDialog;
};


Private::Private(SettingsDialog *parent)
{
    KPageWidgetItem *pageItem;

    // -- 1
    generalWidg = new GeneralWidget(parent);
    generalWidg->layout()->setMargin(0);
    pageItem = parent->addPage(generalWidg, i18n("General"));
    pageItem->setIcon(KIcon("rekonq"));

    // -- 2
    tabsWidg = new TabsWidget(parent);
    tabsWidg->layout()->setMargin(0);
    pageItem = parent->addPage(tabsWidg, i18n("Tabs"));
    pageItem->setIcon(KIcon("tab-duplicate"));

    // -- 3
    appearanceWidg = new AppearanceWidget(parent);
    appearanceWidg->layout()->setMargin(0);
    pageItem = parent->addPage(appearanceWidg, i18n("Appearance"));
    pageItem->setIcon(KIcon("preferences-desktop-font"));

    // -- 4
    webkitWidg = new WebKitWidget(parent);
    webkitWidg->layout()->setMargin(0);
    pageItem = parent->addPage(webkitWidg, i18n("WebKit"));
    QString webkitIconPath = KStandardDirs::locate("appdata", "pics/webkit-icon.png");
    KIcon webkitIcon = KIcon(QIcon(webkitIconPath));
    pageItem->setIcon(webkitIcon);

    // -- 5
    privacyWidg = new PrivacyWidget(parent);
    privacyWidg->layout()->setMargin(0);
    pageItem = parent->addPage(privacyWidg, i18n("Privacy"));
    pageItem->setIcon(KIcon("view-media-artist"));

    // -- 6
    advancedWidg = new AdvancedWidget(parent);
    advancedWidg->layout()->setMargin(0);
    pageItem = parent->addPage(advancedWidg, i18n("Advanced"));
    pageItem->setIcon(KIcon("applications-system"));

    // -- 7
    KCModuleInfo ebrowsingInfo("ebrowsing.desktop");
    ebrowsingModule = new KCModuleProxy(ebrowsingInfo, parent);
    pageItem = parent->addPage(ebrowsingModule, i18n("Search Engines"));
    KIcon wsIcon("edit-web-search");
    if (wsIcon.isNull())
    {
        wsIcon = KIcon("preferences-web-browser-shortcuts");
    }
    pageItem->setIcon(wsIcon);

    // WARNING
    // remember wheh changing here that the smallest netbooks
    // have a 1024x576 resolution. So DON'T bother that limits!!
    parent->setMinimumSize(700, 525);
}


// -----------------------------------------------------------------------------------------------------


SettingsDialog::SettingsDialog(QWidget *parent)
    : KConfigDialog(parent, "rekonfig", ReKonfig::self())
    , d(new Private(this))
{
    showButtonSeparator(false);
    setWindowTitle(i18nc("Window title of the settings dialog", "Configure – rekonq"));

    // update buttons
    connect(d->generalWidg,     SIGNAL(changed(bool)), this, SLOT(updateButtons()));
    connect(d->tabsWidg,        SIGNAL(changed(bool)), this, SLOT(updateButtons()));
    connect(d->appearanceWidg,  SIGNAL(changed(bool)), this, SLOT(updateButtons()));
    connect(d->webkitWidg,      SIGNAL(changed(bool)), this, SLOT(updateButtons()));
    connect(d->ebrowsingModule, SIGNAL(changed(bool)), this, SLOT(updateButtons()));
    connect(d->advancedWidg,    SIGNAL(changed(bool)), this, SLOT(updateButtons()));
    connect(d->privacyWidg,     SIGNAL(changed(bool)), this, SLOT(updateButtons()));

    // save settings
    connect(this, SIGNAL(applyClicked()), this, SLOT(saveSettings()));
    connect(this, SIGNAL(okClicked()),    this, SLOT(saveSettings()));
    setHelp("Config-rekonq", "rekonq");
}


SettingsDialog::~SettingsDialog()
{
    kDebug() << "bye bye settings...";
    delete d;
}


// we need this function to SAVE settings in rc file..
void SettingsDialog::saveSettings()
{
    ReKonfig::self()->writeConfig();

    d->generalWidg->save();
    d->tabsWidg->save();
    d->appearanceWidg->save();
    d->webkitWidg->save();
    d->advancedWidg->save();
    d->privacyWidg->save();
    d->ebrowsingModule->save();

    d->privacyWidg->reload();

    SearchEngine::reload();

    updateButtons();
    emit settingsChanged("ReKonfig");
}


bool SettingsDialog::hasChanged()
{
    return KConfigDialog::hasChanged()
           || d->generalWidg->changed()
           || d->tabsWidg->changed()
           || d->appearanceWidg->changed()
           || d->webkitWidg->changed()
           || d->advancedWidg->changed()
           || d->privacyWidg->changed()
           || d->ebrowsingModule->changed()
    ;
}


bool SettingsDialog::isDefault()
{
    bool isDef = KConfigDialog::isDefault();

    if (isDef)
    {
        // check our private widget values
        isDef = d->appearanceWidg->isDefault();
    }
    return isDef;
}
