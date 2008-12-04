/* ============================================================
 *
 * This file is a part of the reKonq project
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */


// Local Includes
#include "settings.h"
#include "settings.moc"

#include "browserapplication.h"
#include "browsermainwindow.h"
#include "cookiejar.h"
#include "history.h"
#include "networkaccessmanager.h"
#include "webview.h"

// KDE Includes
#include <KConfig>
#include <KFontDialog>

// Qt Includes
#include <QtGui>
#include <QtWebKit>

SettingsDialog::SettingsDialog(QWidget *parent)
    : KDialog(parent),
    widget(0)
{
    widget = new QWidget;

    setupUi(widget);

    setMainWidget(widget);

    setWindowTitle( i18n("Setting reKonq..") );
    setButtons( KDialog::Ok | KDialog::Close | KDialog::Apply );
    setModal(true);

    connect(this, SIGNAL( okClicked() ), this, SLOT( slotOk() ) );
    connect(this, SIGNAL( closeClicked() ), this, SLOT( close() ) );
    connect(this, SIGNAL( applyClicked() ), this, SLOT( slotApply() ) );

    connect(exceptionsButton, SIGNAL(clicked()), this, SLOT( showExceptions() ) );
    connect(setHomeToCurrentPageButton, SIGNAL(clicked()), this, SLOT( setHomeToCurrentPage() ) );
    connect(cookiesButton, SIGNAL(clicked()), this, SLOT( showCookies() ) );
    connect(standardFontButton, SIGNAL(clicked()), this, SLOT( chooseFont() ) );
    connect(fixedFontButton, SIGNAL(clicked()), this, SLOT( chooseFixedFont() ) );

    loadDefaults();
    loadFromSettings();
}


void SettingsDialog::loadDefaults()
{
    QWebSettings *defaultSettings = QWebSettings::globalSettings();
    QString standardFontFamily = defaultSettings->fontFamily(QWebSettings::StandardFont);
    int standardFontSize = defaultSettings->fontSize(QWebSettings::DefaultFontSize);
    standardFont = QFont(standardFontFamily, standardFontSize);
    standardLabel->setText(QString(QLatin1String("%1 %2")).arg(standardFont.family()).arg(standardFont.pointSize()));

    QString fixedFontFamily = defaultSettings->fontFamily(QWebSettings::FixedFont);
    int fixedFontSize = defaultSettings->fontSize(QWebSettings::DefaultFixedFontSize);
    fixedFont = QFont(fixedFontFamily, fixedFontSize);
    fixedLabel->setText(QString(QLatin1String("%1 %2")).arg(fixedFont.family()).arg(fixedFont.pointSize()));

    downloadsLocation->setText(QDesktopServices::storageLocation(QDesktopServices::DesktopLocation));

    enableJavascript->setChecked(defaultSettings->testAttribute(QWebSettings::JavascriptEnabled));
    enablePlugins->setChecked(defaultSettings->testAttribute(QWebSettings::PluginsEnabled));
}


void SettingsDialog::loadFromSettings()
{
    KConfig config("rekonqrc");
    KConfigGroup group1 = config.group("Global Settings");
    
    QString defaultHome = QString("http://www.kde.org");
    homeLineEdit->setText( group1.readEntry(QString("home"), defaultHome) );

    int historyExpire = group1.readEntry( QString("historyExpire"), QString().toInt() );
    int idx = 0;    
    switch (historyExpire) 
    {
        case 1: idx = 0; break;
        case 7: idx = 1; break;
        case 14: idx = 2; break;
        case 30: idx = 3; break;
        case 365: idx = 4; break;
        case -1: idx = 5; break;
        default: idx = 5;
    }
    expireHistory->setCurrentIndex(idx);

    QString downloadDirectory = group1.readEntry( QString("downloadDirectory") , QString() );
    downloadsLocation->setText(downloadDirectory);

    openLinksIn->setCurrentIndex( group1.readEntry( QString("openLinksIn"), openLinksIn->currentIndex() ) );


    // Appearance
    KConfigGroup group2 = config.group("Appearance Settings");

    fixedFont = group2.readEntry( QString("fixedFont"), fixedFont );
    standardFont = group2.readEntry( QString("standardFont"), standardFont );

    standardLabel->setText(QString(QLatin1String("%1 %2")).arg(standardFont.family()).arg(standardFont.pointSize()));
    fixedLabel->setText(QString(QLatin1String("%1 %2")).arg(fixedFont.family()).arg(fixedFont.pointSize()));

    enableJavascript->setChecked( group2.readEntry( QString("enableJavascript"), enableJavascript->isChecked() ) );
    enablePlugins->setChecked( group2.readEntry( QString("enablePlugins"), enablePlugins->isChecked() ) );

    // Privacy
    KConfigGroup group3 = config.group("Privacy Settings");

    CookieJar *jar = BrowserApplication::cookieJar();
    QString value = group3.readEntry( QString("acceptCookies"), QString("AcceptOnlyFromSitesNavigatedTo") ) ;
    QMetaEnum acceptPolicyEnum = jar->staticMetaObject.enumerator(jar->staticMetaObject.indexOfEnumerator("AcceptPolicy"));
    CookieJar::AcceptPolicy acceptCookies = acceptPolicyEnum.keyToValue( value.toLocal8Bit() ) == -1 ?
                        CookieJar::AcceptOnlyFromSitesNavigatedTo :
                        static_cast<CookieJar::AcceptPolicy>(acceptPolicyEnum.keyToValue( value.toLocal8Bit() ) );
    switch(acceptCookies) 
    {
    case CookieJar::AcceptAlways:
        acceptCombo->setCurrentIndex(0);
        break;
    case CookieJar::AcceptNever:
        acceptCombo->setCurrentIndex(1);
        break;
    case CookieJar::AcceptOnlyFromSitesNavigatedTo:
        acceptCombo->setCurrentIndex(2);
        break;
    }

    value = group3.readEntry( QString("keepCookiesUntil"), QString("Expire") ); 
    QMetaEnum keepPolicyEnum = jar->staticMetaObject.enumerator(jar->staticMetaObject.indexOfEnumerator("KeepPolicy"));
    CookieJar::KeepPolicy keepCookies = keepPolicyEnum.keyToValue( value.toLocal8Bit() ) == -1 ?
                        CookieJar::KeepUntilExpire :
                        static_cast<CookieJar::KeepPolicy>(keepPolicyEnum.keyToValue( value.toLocal8Bit() ) );
    switch(keepCookies) {
    case CookieJar::KeepUntilExpire:
        keepUntilCombo->setCurrentIndex(0);
        break;
    case CookieJar::KeepUntilExit:
        keepUntilCombo->setCurrentIndex(1);
        break;
    case CookieJar::KeepUntilTimeLimit:
        keepUntilCombo->setCurrentIndex(2);
        break;
    }

    // Proxy
    KConfigGroup group4 = config.group("Proxy Settings");

    proxySupport->setChecked( group4.readEntry( QString("enabled"), false ) );
    proxyType->setCurrentIndex( group4.readEntry( QString("type"), 0) );
    proxyHostName->setText( group4.readEntry( QString("hostName"), QString() ) );
    proxyPort->setValue( group4.readEntry( QString("port"), QString().toInt() ) );
    proxyUserName->setText( group4.readEntry( QString("userName") , QString() ) );
    proxyPassword->setText( group4.readEntry( QString("password") , QString() ) );

}


void SettingsDialog::saveToSettings()
{
    KConfig config("rekonqrc");
    KConfigGroup group1 = config.group("Global Settings");
    
    group1.writeEntry(QString("home"), homeLineEdit->text() );
    group1.writeEntry(QString("openLinksIn"), openLinksIn->currentIndex() );
    group1.writeEntry(QString("downloadDirectory"), downloadsLocation->text() );

    int historyExpire = expireHistory->currentIndex();
    int idx = -1;
    switch (historyExpire) 
    {
    case 0: idx = 1; break;
    case 1: idx = 7; break;
    case 2: idx = 14; break;
    case 3: idx = 30; break;
    case 4: idx = 365; break;
    case 5: idx = -1; break;
    }
    group1.writeEntry(QString("historyExpire"), idx );

    KConfigGroup group2 = config.group("Appearance Settings");
    group2.writeEntry(QString("fixedFont"),fixedFont);
    group2.writeEntry(QString("standardFont"), standardFont);

    KConfigGroup group3 = config.group("Privacy Settings");
    group3.writeEntry(QString("enableJavascript"), enableJavascript->isChecked() );
    group3.writeEntry(QString("enablePlugins"), enablePlugins->isChecked() );

    CookieJar::KeepPolicy keepCookies;
    switch(acceptCombo->currentIndex()) 
    {
    default:
    case 0:
        keepCookies = CookieJar::KeepUntilExpire;
        break;
    case 1:
        keepCookies = CookieJar::KeepUntilExit;
        break;
    case 2:
        keepCookies = CookieJar::KeepUntilTimeLimit;
        break;
    }
    CookieJar *jar = BrowserApplication::cookieJar();
    QMetaEnum acceptPolicyEnum = jar->staticMetaObject.enumerator(jar->staticMetaObject.indexOfEnumerator("AcceptPolicy"));
    group3.writeEntry(QString("acceptCookies"), QString(acceptPolicyEnum.valueToKey(keepCookies) ) );

    CookieJar::KeepPolicy keepPolicy;
    switch(keepUntilCombo->currentIndex()) 
    {
        default:
    case 0:
        keepPolicy = CookieJar::KeepUntilExpire;
        break;
    case 1:
        keepPolicy = CookieJar::KeepUntilExit;
        break;
    case 2:
        keepPolicy = CookieJar::KeepUntilTimeLimit;
        break;
    }

    QMetaEnum keepPolicyEnum = jar->staticMetaObject.enumerator(jar->staticMetaObject.indexOfEnumerator("KeepPolicy"));
    group3.writeEntry(QString("keepCookiesUntil"), QString(keepPolicyEnum.valueToKey(keepPolicy) ) );

    
    KConfigGroup group4 = config.group("Proxy Settings");
    group4.writeEntry(QString("enabled"), proxySupport->isChecked() );
    group4.writeEntry(QString("type"), proxyType->currentIndex() );
    group4.writeEntry(QString("hostName"), proxyHostName->text() );
    group4.writeEntry(QString("port"), proxyPort->text() );
    group4.writeEntry(QString("userName"), proxyUserName->text() );
    group4.writeEntry(QString("password"), proxyPassword->text() );

    config.sync();

    // ---
    BrowserApplication::instance()->loadSettings();
    BrowserApplication::networkAccessManager()->loadSettings();
    BrowserApplication::cookieJar()->loadSettings();
    BrowserApplication::historyManager()->loadSettings();
}


void SettingsDialog::showCookies()
{
    CookiesDialog *dialog = new CookiesDialog(BrowserApplication::cookieJar(), this);
    dialog->exec();
}


void SettingsDialog::showExceptions()
{
    CookiesExceptionsDialog *dialog = new CookiesExceptionsDialog(BrowserApplication::cookieJar(), this);
    dialog->exec();
}


void SettingsDialog::chooseFont()
{
    QFont myFont;
    int result = KFontDialog::getFont( myFont );
    if ( result == KFontDialog::Accepted  ) 
    {
        standardFont = myFont;
        standardLabel->setText(QString(QLatin1String("%1 %2")).arg(standardFont.family()).arg(standardFont.pointSize()));
    }
}


void SettingsDialog::chooseFixedFont()
{
   
    QFont myFont;
    int result = KFontDialog::getFont( myFont , KFontChooser::FixedFontsOnly );
    if ( result == KFontDialog::Accepted  ) 
    {
        fixedFont = myFont;
        fixedLabel->setText(QString(QLatin1String("%1 %2")).arg(fixedFont.family()).arg(fixedFont.pointSize()));
    }
}


void SettingsDialog::setHomeToCurrentPage()
{
    BrowserMainWindow *mw = static_cast<BrowserMainWindow*>(parent());
    WebView *webView = mw->currentTab();
    if (webView)
    {
        homeLineEdit->setText( webView->url().prettyUrl() );
    }
}


void SettingsDialog::slotOk()
{
    slotApply();
    close();
}

void SettingsDialog::slotApply()
{
    saveToSettings();
}
