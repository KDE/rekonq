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

//Ui Includes
#include "ui_settings_general.h"
#include "ui_settings_appearance.h"
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
    Ui::appearance appearanceUi;
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
    generalUi.kurlrequester->setMode( KFile::Directory );

    widget = new QWidget;
    appearanceUi.setupUi( widget );
    widget->layout()->setMargin(0);
    pageItem = parent->addPage( widget , i18n("Appearance") );
    pageItem->setIcon( KIcon("kfontview") );

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
    : KConfigDialog(parent, "Settings", new KConfigSkeleton("rekonqrc") )
    , d(new Private(this) ) 
{
    setFaceType(KPageDialog::List);
    setButtons(KDialog::Ok | KDialog::Cancel | KDialog::Apply);
    showButtonSeparator(true);

    setWindowTitle( i18n("Setting rekonq..") );
    setModal(true);

    connect(this, SIGNAL( okClicked() ), this, SLOT( slotOk() ) );
    connect(this, SIGNAL( closeClicked() ), this, SLOT( close() ) );
    connect(this, SIGNAL( applyClicked() ), this, SLOT( slotApply() ) );

    connect( d->generalUi.setHomeToCurrentPageButton, SIGNAL(clicked()), this, SLOT( setHomeToCurrentPage() ) );
    connect( d->privacyUi.exceptionsButton, SIGNAL(clicked()), this, SLOT( showExceptions() ) );
    connect( d->privacyUi.cookiesButton, SIGNAL(clicked()), this, SLOT( showCookies() ) );
    connect( d->appearanceUi.standardFontButton, SIGNAL(clicked()), this, SLOT( chooseFont() ) );
    connect( d->appearanceUi.fixedFontButton, SIGNAL(clicked()), this, SLOT( chooseFixedFont() ) );

    loadDefaults();
    loadFromSettings();
}


SettingsDialog::~SettingsDialog()
{
    delete d;
}


void SettingsDialog::loadDefaults()
{
    QWebSettings *defaultSettings = QWebSettings::globalSettings();
    QString standardFontFamily = defaultSettings->fontFamily(QWebSettings::StandardFont);
    int standardFontSize = defaultSettings->fontSize(QWebSettings::DefaultFontSize);
    m_standardFont = QFont(standardFontFamily, standardFontSize);
    d->appearanceUi.standardLabel->setText(QString(QLatin1String("%1 %2")).arg( m_standardFont.family() ).arg( m_standardFont.pointSize() ));

    QString fixedFontFamily = defaultSettings->fontFamily(QWebSettings::FixedFont);
    int fixedFontSize = defaultSettings->fontSize(QWebSettings::DefaultFixedFontSize);
    m_fixedFont = QFont(fixedFontFamily, fixedFontSize);
    d->appearanceUi.fixedLabel->setText(QString(QLatin1String("%1 %2")).arg( m_fixedFont.family() ).arg( m_fixedFont.pointSize() ));

    d->generalUi.kurlrequester->setUrl( KUrl( "~" ) ); // QDesktopServices::storageLocation(QDesktopServices::DesktopLocation) );

    d->privacyUi.enableJavascript->setChecked(defaultSettings->testAttribute(QWebSettings::JavascriptEnabled));
    d->privacyUi.enablePlugins->setChecked(defaultSettings->testAttribute(QWebSettings::PluginsEnabled));
}


void SettingsDialog::loadFromSettings()
{
    KConfig config("rekonqrc");
    KConfigGroup group1 = config.group("Global Settings");
    
    QString defaultHome = QString("http://www.kde.org");
    d->generalUi.homeLineEdit->setText( group1.readEntry(QString("home"), defaultHome) );

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
    d->generalUi.expireHistory->setCurrentIndex(idx);

    QString downloadDirectory = group1.readEntry( QString("downloadDirectory") , QString() );
    d->generalUi.kurlrequester->setUrl( KUrl(downloadDirectory) );

    d->generalUi.openLinksIn->setCurrentIndex( group1.readEntry( QString("openLinksIn"), d->generalUi.openLinksIn->currentIndex() ) );


    // Appearance
    KConfigGroup group2 = config.group("Appearance Settings");

    m_fixedFont = group2.readEntry( QString("fixedFont"), m_fixedFont );
    m_standardFont = group2.readEntry( QString("standardFont"), m_standardFont );

    d->appearanceUi.standardLabel->setText(QString(QLatin1String("%1 %2")).arg( m_standardFont.family() ).arg( m_standardFont.pointSize() ) );
    d->appearanceUi.fixedLabel->setText(QString(QLatin1String("%1 %2")).arg( m_fixedFont.family() ).arg( m_fixedFont.pointSize() ) );

    // Privacy
    KConfigGroup group3 = config.group("Privacy Settings");

    d->privacyUi.enableJavascript->setChecked( group3.readEntry( QString("enableJavascript"), d->privacyUi.enableJavascript->isChecked() ) );
    d->privacyUi.enablePlugins->setChecked( group3.readEntry( QString("enablePlugins"), d->privacyUi.enablePlugins->isChecked() ) );

    CookieJar *jar = BrowserApplication::cookieJar();
    QString value = group3.readEntry( QString("acceptCookies"), QString("AcceptOnlyFromSitesNavigatedTo") ) ;
    QMetaEnum acceptPolicyEnum = jar->staticMetaObject.enumerator(jar->staticMetaObject.indexOfEnumerator("AcceptPolicy"));
    CookieJar::AcceptPolicy acceptCookies = acceptPolicyEnum.keyToValue( value.toLocal8Bit() ) == -1 ?
                        CookieJar::AcceptOnlyFromSitesNavigatedTo :
                        static_cast<CookieJar::AcceptPolicy>(acceptPolicyEnum.keyToValue( value.toLocal8Bit() ) );
    switch(acceptCookies) 
    {
    case CookieJar::AcceptAlways:
        d->privacyUi.acceptCombo->setCurrentIndex(0);
        break;
    case CookieJar::AcceptNever:
        d->privacyUi.acceptCombo->setCurrentIndex(1);
        break;
    case CookieJar::AcceptOnlyFromSitesNavigatedTo:
        d->privacyUi.acceptCombo->setCurrentIndex(2);
        break;
    }

    value = group3.readEntry( QString("keepCookiesUntil"), QString("Expire") ); 
    QMetaEnum keepPolicyEnum = jar->staticMetaObject.enumerator(jar->staticMetaObject.indexOfEnumerator("KeepPolicy"));
    CookieJar::KeepPolicy keepCookies = keepPolicyEnum.keyToValue( value.toLocal8Bit() ) == -1 ?
                        CookieJar::KeepUntilExpire :
                        static_cast<CookieJar::KeepPolicy>(keepPolicyEnum.keyToValue( value.toLocal8Bit() ) );
    switch(keepCookies) 
    {
    case CookieJar::KeepUntilExpire:
        d->privacyUi.keepUntilCombo->setCurrentIndex(0);
        break;
    case CookieJar::KeepUntilExit:
        d->privacyUi.keepUntilCombo->setCurrentIndex(1);
        break;
    case CookieJar::KeepUntilTimeLimit:
        d->privacyUi.keepUntilCombo->setCurrentIndex(2);
        break;
    }

    // Proxy
    KConfigGroup group4 = config.group("Proxy Settings");

    d->proxyUi.proxySupport->setChecked( group4.readEntry( QString("enabled"), false ) );
    d->proxyUi.proxyType->setCurrentIndex( group4.readEntry( QString("type"), 0) );
    d->proxyUi.proxyHostName->setText( group4.readEntry( QString("hostName"), QString() ) );
    d->proxyUi.proxyPort->setValue( group4.readEntry( QString("port"), QString().toInt() ) );
    d->proxyUi.proxyUserName->setText( group4.readEntry( QString("userName") , QString() ) );
    d->proxyUi.proxyPassword->setText( group4.readEntry( QString("password") , QString() ) );

}


void SettingsDialog::saveToSettings()
{
    KConfig config("rekonqrc");
    KConfigGroup group1 = config.group("Global Settings");
    
    group1.writeEntry(QString("home"), d->generalUi.homeLineEdit->text() );
    group1.writeEntry(QString("openLinksIn"), d->generalUi.openLinksIn->currentIndex() );
    group1.writeEntry(QString("downloadDirectory"), d->generalUi.kurlrequester->url().path() );

    int historyExpire = d->generalUi.expireHistory->currentIndex();
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
    group2.writeEntry(QString("fixedFont"), m_fixedFont);
    group2.writeEntry(QString("standardFont"), m_standardFont);

    KConfigGroup group3 = config.group("Privacy Settings");
    group3.writeEntry(QString("enableJavascript"), d->privacyUi.enableJavascript->isChecked() );
    group3.writeEntry(QString("enablePlugins"), d->privacyUi.enablePlugins->isChecked() );

    CookieJar::KeepPolicy keepCookies;
    switch( d->privacyUi.acceptCombo->currentIndex() ) 
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
    switch( d->privacyUi.keepUntilCombo->currentIndex() ) 
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
    group4.writeEntry(QString("enabled"), d->proxyUi.proxySupport->isChecked() );
    group4.writeEntry(QString("type"), d->proxyUi.proxyType->currentIndex() );
    group4.writeEntry(QString("hostName"), d->proxyUi.proxyHostName->text() );
    group4.writeEntry(QString("port"), d->proxyUi.proxyPort->text() );
    group4.writeEntry(QString("userName"), d->proxyUi.proxyUserName->text() );
    group4.writeEntry(QString("password"), d->proxyUi.proxyPassword->text() );

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
    QFont myFont( m_standardFont );
    int result = KFontDialog::getFont( myFont );
    if ( result == KFontDialog::Accepted  ) 
    {
        m_standardFont = myFont;
        d->appearanceUi.standardLabel->setText(QString(QLatin1String("%1 %2")).arg( m_standardFont.family() ).arg( m_standardFont.pointSize() ) );
    }
}


void SettingsDialog::chooseFixedFont()
{
   
    QFont myFont( m_fixedFont );
    int result = KFontDialog::getFont( myFont , KFontChooser::FixedFontsOnly );
    if ( result == KFontDialog::Accepted  ) 
    {
        m_fixedFont = myFont;
        d->appearanceUi.fixedLabel->setText(QString(QLatin1String("%1 %2")).arg( m_fixedFont.family() ).arg( m_fixedFont.pointSize() ) );
    }
}


void SettingsDialog::setHomeToCurrentPage()
{
    BrowserMainWindow *mw = static_cast<BrowserMainWindow*>(parent());
    WebView *webView = mw->currentTab();
    if (webView)
    {
        d->generalUi.homeLineEdit->setText( webView->url().prettyUrl() );
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
