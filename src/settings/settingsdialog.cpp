/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2008-2010 by Andrea Diamantini <adjam7 at gmail dot com>
* Copyright (C) 2009-2010 by Lionel Chauvin <megabigbug@yahoo.fr>
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
#include "application.h"
#include "mainwindow.h"
#include "webtab.h"
#include "adblockwidget.h"
#include "networkwidget.h"

//Ui Includes
#include "ui_settings_general.h"
#include "ui_settings_tabs.h"
#include "ui_settings_fonts.h"
#include "ui_settings_webkit.h"

// KDE Includes
#include <KConfig>
#include <KStandardDirs>
#include <KPageWidgetItem>
#include <KShortcutsEditor>
#include <KCModuleInfo>
#include <KCModuleProxy>

// Qt Includes
#include <QtGui/QWidget>


class Private
{
private:
    
    Ui::general generalUi;
    Ui::tabs tabsUi;
    Ui::fonts fontsUi;
    Ui::webkit webkitUi;

    AdBlockWidget *adBlockWidg;
    NetworkWidget *networkWidg;
    
    KCModuleProxy *ebrowsingModule;

    KShortcutsEditor *shortcutsEditor;
    
    Private(SettingsDialog *parent);

    friend class SettingsDialog;
};


Private::Private(SettingsDialog *parent)
{
    QWidget *widget;
    KPageWidgetItem *pageItem;

    widget = new QWidget;
    generalUi.setupUi(widget);
    widget->layout()->setMargin(0);
    pageItem = parent->addPage(widget , i18n("General"));
    pageItem->setIcon(KIcon("rekonq"));

    widget = new QWidget;
    tabsUi.setupUi(widget);
    widget->layout()->setMargin(0);
    pageItem = parent->addPage(widget , i18n("Tabs"));
    pageItem->setIcon(KIcon("tab-duplicate"));

    widget = new QWidget;
    fontsUi.setupUi(widget);
    widget->layout()->setMargin(0);
    pageItem = parent->addPage(widget , i18n("Fonts"));
    pageItem->setIcon(KIcon("preferences-desktop-font"));
        
    widget = new QWidget;
    webkitUi.setupUi(widget);
    widget->layout()->setMargin(0);
    pageItem = parent->addPage(widget , i18n("WebKit"));
    QString webkitIconPath = KStandardDirs::locate("appdata", "pics/webkit-icon.png");
    KIcon webkitIcon = KIcon(QIcon(webkitIconPath));
    pageItem->setIcon(webkitIcon);

    networkWidg = new NetworkWidget(parent);
    networkWidg->layout()->setMargin(0);
    pageItem = parent->addPage(networkWidg , i18n("Network"));
    pageItem->setIcon( KIcon("preferences-system-network") );
    
    adBlockWidg = new AdBlockWidget(parent);
    adBlockWidg->layout()->setMargin(0);
    pageItem = parent->addPage(adBlockWidg , i18n("Ad Block"));
    pageItem->setIcon( KIcon("preferences-web-browser-adblock") );
    
    shortcutsEditor = new KShortcutsEditor(Application::instance()->mainWindow()->actionCollection(), parent);
    pageItem = parent->addPage(shortcutsEditor , i18n("Shortcuts"));
    pageItem->setIcon(KIcon("configure-shortcuts"));

    KCModuleInfo ebrowsingInfo("ebrowsing.desktop");
    ebrowsingModule = new KCModuleProxy(ebrowsingInfo,parent);
    pageItem = parent->addPage(ebrowsingModule, i18n(ebrowsingInfo.moduleName().toLocal8Bit()));
    pageItem->setIcon(KIcon(ebrowsingInfo.icon()));

    // WARNING remember wheh changing here that the smallest netbooks
    // have a 1024x576 resolution. So DON'T bother that limits!!
    parent->setMinimumSize(700,525);    
}


// -----------------------------------------------------------------------------------------------------


SettingsDialog::SettingsDialog(QWidget *parent)
        : KConfigDialog(parent, "rekonfig", ReKonfig::self())
        , d(new Private(this))
{
    showButtonSeparator(false);
    setWindowTitle(i18nc("Window title of the settings dialog", "Configure – rekonq"));
    setModal(true);

    readConfig();

    connect(d->generalUi.setHomeToCurrentPageButton, SIGNAL(clicked()), this, SLOT(setHomeToCurrentPage()));
    
    // update buttons
    connect(d->adBlockWidg,     SIGNAL(changed(bool)), this, SLOT(updateButtons()));
    connect(d->networkWidg,     SIGNAL(changed(bool)), this, SLOT(updateButtons()));
    connect(d->ebrowsingModule, SIGNAL(changed(bool)), this, SLOT(updateButtons()));
    
    connect(d->shortcutsEditor, SIGNAL(keyChange()),   this, SLOT(updateButtons()));
    
    // save settings
    connect(this, SIGNAL(applyClicked()), this, SLOT(saveSettings()));
    connect(this, SIGNAL(okClicked()),    this, SLOT(saveSettings()));
        
    setWebSettingsToolTips();
}


SettingsDialog::~SettingsDialog()
{
    delete d;
}


void SettingsDialog::setWebSettingsToolTips()
{
    d->webkitUi.kcfg_autoLoadImages->setToolTip(i18n("Specifies whether images are automatically loaded in web pages."));
    d->webkitUi.kcfg_javascriptEnabled->setToolTip(i18n("Enables the execution of JavaScript programs."));
    d->webkitUi.kcfg_javaEnabled->setToolTip(i18n("Enables support for Java applets."));
    d->webkitUi.kcfg_pluginsEnabled->setToolTip(i18n("Enables support for plugins in web pages."));
    d->webkitUi.kcfg_javascriptCanOpenWindows->setToolTip(i18n("If enabled, JavaScript programs are allowed to open new windows."));
    d->webkitUi.kcfg_javascriptCanAccessClipboard->setToolTip(i18n("If enabled, JavaScript programs are allowed to read from and to write to the clipboard."));
    d->webkitUi.kcfg_linksIncludedInFocusChain->setToolTip(i18n("If enabled, hyperlinks are included in the keyboard focus chain."));
    d->webkitUi.kcfg_zoomTextOnly->setToolTip(i18n("If enabled, the zoom factor on a frame is only applied to the text."));
    d->webkitUi.kcfg_printElementBackgrounds->setToolTip(i18n("If enabled, background colors and images are also drawn when the page is printed."));
    d->webkitUi.kcfg_offlineStorageDatabaseEnabled->setToolTip(i18n("Enables support for the HTML 5 offline storage feature."));
    d->webkitUi.kcfg_offlineWebApplicationCacheEnabled->setToolTip(i18n("Enables support for the HTML 5 web application cache feature."));
    d->webkitUi.kcfg_localStorageDatabaseEnabled->setToolTip(i18n("Enables support for the HTML 5 local storage feature."));
}


// we need this function to UPDATE the config widget data..
void SettingsDialog::readConfig()
{
    // ======= Fonts
    d->fontsUi.kcfg_fixedFont->setOnlyFixed(true);
}


// we need this function to SAVE settings in rc file..
void SettingsDialog::saveSettings()
{
    if (!hasChanged())
        return;
    
    ReKonfig::self()->writeConfig();
    d->ebrowsingModule->save();
    d->shortcutsEditor->save();
    d->adBlockWidg->save();
    d->networkWidg->save();

    updateButtons();
    emit settingsChanged("ReKonfig");
}


bool SettingsDialog::hasChanged()
{
    return KConfigDialog::hasChanged() 
            || d->adBlockWidg->changed()
            || d->networkWidg->changed()
            || d->ebrowsingModule->changed()
            || d->shortcutsEditor->isModified();
            ;
}


void SettingsDialog::setHomeToCurrentPage()
{
    MainWindow *mw = static_cast<MainWindow*>(parent());
    WebTab *webTab = mw->currentTab();
    if (webTab)
    {
        d->generalUi.kcfg_homePage->setText(webTab->url().prettyUrl());
    }
}
