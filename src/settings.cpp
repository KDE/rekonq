/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2007-2008 Trolltech ASA. All rights reserved
* Copyright (C) 2008 by Andrea Diamantini <adjam7 at gmail dot com>
*
*
* This program is free software; you can redistribute it
* and/or modify it under the terms of the GNU General
* Public License as published by the Free Software Foundation;
* either version 2, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* ============================================================ */



// Self Includes
#include "settings.h"
#include "settings.moc"

// Auto Includes
#include "rekonq.h"

// Local Includes
#include "application.h"
#include "mainwindow.h"
#include "cookiejar.h"
#include "history.h"
#include "networkaccessmanager.h"
#include "webview.h"

//Ui Includes
#include "ui_settings_general.h"
#include "ui_settings_fonts.h"
#include "ui_settings_privacy.h"
#include "ui_settings_proxy.h"

// KDE Includes
#include <KConfig>
#include <KConfigSkeleton>
#include <KPageWidgetItem>
#include <KFontDialog>
#include <KUrl>

// Qt Includes
#include <QtGui>
#include <QtWebKit>


class Private
{
private:
    Ui::general generalUi;
    Ui::fonts fontsUi;
    Ui::privacy privacyUi;
    Ui::proxy proxyUi;

    Private(SettingsDialog *parent);

friend class SettingsDialog;
};


Private::Private(SettingsDialog *parent)
{
    QWidget *widget;
    KPageWidgetItem *pageItem;

    widget = new QWidget;
    generalUi.setupUi( widget );
    widget->layout()->setMargin(0);
    pageItem = parent->addPage( widget , i18n("General") );
    pageItem->setIcon( KIcon("rekonq") );

    widget = new QWidget;
    fontsUi.setupUi( widget );
    widget->layout()->setMargin(0);
    pageItem = parent->addPage( widget , i18n("Fonts") );
    pageItem->setIcon( KIcon("preferences-desktop-font") );

    widget = new QWidget;
    privacyUi.setupUi( widget );
    widget->layout()->setMargin(0);
    pageItem = parent->addPage( widget , i18n("Privacy") );
    pageItem->setIcon( KIcon("preferences-desktop-personal") );

    widget = new QWidget;
    proxyUi.setupUi( widget );
    widget->layout()->setMargin(0);
    pageItem = parent->addPage( widget , i18n("Proxy") );
    pageItem->setIcon( KIcon("preferences-system-network") );
}

// -----------------------------------------------------------------------------------------------------

SettingsDialog::SettingsDialog(QWidget *parent)
    : KConfigDialog(parent, "settings", ReKonfig::self() )
    , d(new Private(this) ) 
{
    setFaceType(KPageDialog::List);
    showButtonSeparator(true);

    setWindowTitle( i18n("rekonfig..") );
    setModal(true);

    readConfig();

    connect( d->generalUi.setHomeToCurrentPageButton, SIGNAL(clicked()), this, SLOT( setHomeToCurrentPage() ) );
    connect( d->privacyUi.exceptionsButton, SIGNAL(clicked()), this, SLOT( showExceptions() ) );
    connect( d->privacyUi.cookiesButton, SIGNAL(clicked()), this, SLOT( showCookies() ) );
}



SettingsDialog::~SettingsDialog()
{
    delete d;
}

// we need this function to UPDATE the config widget data..
void SettingsDialog::readConfig()
{
    // ======= General
    d->generalUi.downloadDirUrlRequester->setMode(KFile::Directory | KFile::ExistingOnly | KFile::LocalOnly);
    d->generalUi.downloadDirUrlRequester->setUrl( ReKonfig::downloadDir() );
    connect(d->generalUi.downloadDirUrlRequester, SIGNAL(textChanged(QString)),this, SLOT(saveSettings()));

    // ======= Fonts
    QFont stdFont = ReKonfig::standardFont();
    d->fontsUi.standardFont->setCurrentFont(stdFont);

    QFont fxFont = ReKonfig::fixedFont();
    d->fontsUi.fixedFont->setOnlyFixed(true);
    d->fontsUi.fixedFont->setCurrentFont(fxFont);

    int fnSize = ReKonfig::fontSize();
    d->fontsUi.fontSize->setValue( fnSize );

    // ======= Proxy
    bool proxyEnabled = ReKonfig::isProxyEnabled();
    d->proxyUi.groupBox->setEnabled(proxyEnabled);
    connect(d->proxyUi.kcfg_isProxyEnabled, SIGNAL(clicked(bool)), d->proxyUi.groupBox, SLOT(setEnabled(bool)));
}


// we need this function to SAVE settings in rc file..
void SettingsDialog::saveSettings()
{
    // General
    ReKonfig::setDownloadDir( d->generalUi.downloadDirUrlRequester->url().prettyUrl() );

    // Fonts
    ReKonfig::setStandardFont( d->fontsUi.standardFont->currentFont() );
    ReKonfig::setFixedFont( d->fontsUi.fixedFont->currentFont() );
    ReKonfig::setFontSize( d->fontsUi.fontSize->value() );

    // Save
    ReKonfig::self()->writeConfig();
}


// ----------------------------------------------------------------------------------------------


void SettingsDialog::showCookies()
{
    CookiesDialog *dialog = new CookiesDialog(Application::cookieJar(), this);
    dialog->exec();
}


void SettingsDialog::showExceptions()
{
    CookiesExceptionsDialog *dialog = new CookiesExceptionsDialog(Application::cookieJar(), this);
    dialog->exec();
}


void SettingsDialog::setHomeToCurrentPage()
{
    MainWindow *mw = static_cast<MainWindow*>(parent());
    WebView *webView = mw->currentTab();
    if (webView)
    {
        d->generalUi.kcfg_homePage->setText( webView->url().prettyUrl() );
    }
}
