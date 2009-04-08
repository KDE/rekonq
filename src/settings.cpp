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
#include "ui_settings_webkit.h"

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
    Ui::webkit webkitUi;

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
    fontsUi.setupUi(widget);
    widget->layout()->setMargin(0);
    pageItem = parent->addPage(widget , i18n("Fonts"));
    pageItem->setIcon(KIcon("preferences-desktop-font"));

    widget = new QWidget;
    privacyUi.setupUi(widget);
    widget->layout()->setMargin(0);
    pageItem = parent->addPage(widget , i18n("Privacy"));
    pageItem->setIcon(KIcon("preferences-desktop-personal"));

    widget = new QWidget;
    proxyUi.setupUi(widget);
    widget->layout()->setMargin(0);
    pageItem = parent->addPage(widget , i18n("Proxy"));
    pageItem->setIcon(KIcon("preferences-system-network"));

    widget = new QWidget;
    webkitUi.setupUi(widget);
    widget->layout()->setMargin(0);
    pageItem = parent->addPage(widget , i18n("Webkit"));
    pageItem->setIcon(KIcon("applications-internet"));
}

// -----------------------------------------------------------------------------------------------------

SettingsDialog::SettingsDialog(QWidget *parent)
        : KConfigDialog(parent, "rekonfig", ReKonfig::self())
        , d(new Private(this))
{
    setFaceType(KPageDialog::List);
    showButtonSeparator(true);

    setWindowTitle(i18n("rekonfig.."));
    setModal(true);

    readConfig();

    connect(d->generalUi.setHomeToCurrentPageButton, SIGNAL(clicked()), this, SLOT(setHomeToCurrentPage()));
    connect(d->privacyUi.exceptionsButton, SIGNAL(clicked()), this, SLOT(showExceptions()));
    connect(d->privacyUi.cookiesButton, SIGNAL(clicked()), this, SLOT(showCookies()));

    setWebSettingsToolTips();
}



SettingsDialog::~SettingsDialog()
{
    delete d;
}


void SettingsDialog::setWebSettingsToolTips()
{
    d->webkitUi.kcfg_autoLoadImages->setToolTip(i18n("Specifies whether images are automatically loaded in web pages") );
    d->webkitUi.kcfg_javascriptEnabled->setToolTip(i18n("Enables the running of JavaScript programs.") );
    d->webkitUi.kcfg_javaEnabled->setToolTip(i18n("Enables Java applets.") );
    d->webkitUi.kcfg_pluginsEnabled->setToolTip(i18n("Enables plugins in web pages.") );
    d->webkitUi.kcfg_javascriptCanOpenWindows->setToolTip(i18n("Allows JavaScript programs to opening new windows.") );
    d->webkitUi.kcfg_javascriptCanAccessClipboard->setToolTip(i18n("Allows JavaScript programs to reading/writing to the clipboard.") );
    d->webkitUi.kcfg_linksIncludedInFocusChain->setToolTip(i18n("Includes hyperlinks in the keyboard focus chain.") );
    d->webkitUi.kcfg_zoomTextOnly->setToolTip(i18n("Applies the zoom factor on a frame to only the text or all content.") );
    d->webkitUi.kcfg_printElementBackgrounds->setToolTip(i18n("Draws also background color and images when the page is printed.") );
    d->webkitUi.kcfg_offlineStorageDatabaseEnabled->setToolTip(i18n("Support for the HTML 5 offline storage feature.") );
    d->webkitUi.kcfg_offlineWebApplicationCacheEnabled->setToolTip(i18n("Support for the HTML 5 web application cache feature.") );
    d->webkitUi.kcfg_localStorageDatabaseEnabled->setToolTip(i18n("Support for the HTML 5 local storage feature.") );
}


// we need this function to UPDATE the config widget data..
void SettingsDialog::readConfig()
{
    // ======= General
    d->generalUi.downloadDirUrlRequester->setMode(KFile::Directory | KFile::ExistingOnly | KFile::LocalOnly);
    d->generalUi.downloadDirUrlRequester->setUrl(ReKonfig::downloadDir());
    connect(d->generalUi.downloadDirUrlRequester, SIGNAL(textChanged(QString)), this, SLOT(saveSettings()));

    // ======= Fonts
    d->fontsUi.kcfg_fixedFont->setOnlyFixed(true);

    // ======= Proxy
    bool proxyEnabled = ReKonfig::isProxyEnabled();
    d->proxyUi.groupBox->setEnabled(proxyEnabled);
    connect(d->proxyUi.kcfg_isProxyEnabled, SIGNAL(clicked(bool)), d->proxyUi.groupBox, SLOT(setEnabled(bool)));
}


// we need this function to SAVE settings in rc file..
void SettingsDialog::saveSettings()
{
    // General
    ReKonfig::setDownloadDir(d->generalUi.downloadDirUrlRequester->url().prettyUrl());

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
        d->generalUi.kcfg_homePage->setText(webView->url().prettyUrl());
    }
}
