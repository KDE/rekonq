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

// Self Includes
#include "networkaccessmanager.h"
#include "networkaccessmanager.moc"

// Local Includes
#include "application.h"
#include "mainwindow.h"

// Auto Includes
#include "rekonq.h"

// Ui Includes
#include "ui_passworddialog.h"
#include "ui_proxy.h"

// KDE Includes
#include <KMessageBox>

// Qt Includes
#include <QDialog>
#include <QStyle>
#include <QTextDocument>
#include <QAuthenticator>
#include <QNetworkProxy>
#include <QNetworkReply>
#include <QSslError>


NetworkAccessManager::NetworkAccessManager(QObject *parent)
    : QNetworkAccessManager(parent)
{
    connect(this, SIGNAL(authenticationRequired(QNetworkReply*, QAuthenticator*)),
            SLOT(authenticationRequired(QNetworkReply*,QAuthenticator*)));
    connect(this, SIGNAL(proxyAuthenticationRequired(const QNetworkProxy&, QAuthenticator*)),
            SLOT(proxyAuthenticationRequired(const QNetworkProxy&, QAuthenticator*)));
#ifndef QT_NO_OPENSSL
    connect(this, SIGNAL(sslErrors(QNetworkReply*, const QList<QSslError>&)),
            SLOT(sslErrors(QNetworkReply*, const QList<QSslError>&)));
#endif
    loadSettings();
}


void NetworkAccessManager::loadSettings()
{
    QNetworkProxy proxy;
    if ( ReKonfig::isProxyEnabled() ) 
    {
        if ( ReKonfig::proxyType() == 0 )
        {
            proxy.setType(QNetworkProxy::Socks5Proxy);
        }
        else
        {
            proxy.setType(QNetworkProxy::HttpProxy);
        }
        proxy.setHostName( ReKonfig::proxyHostName() );
        proxy.setPort( ReKonfig::proxyPort() );
        proxy.setUser( ReKonfig::proxyUserName() );
        proxy.setPassword( ReKonfig::proxyPassword() );
    }
    setProxy(proxy);
}



void NetworkAccessManager::authenticationRequired(QNetworkReply *reply, QAuthenticator *auth)
{
    MainWindow *mainWindow = Application::instance()->mainWindow();

    QDialog dialog(mainWindow);
    dialog.setWindowFlags(Qt::Sheet);

    Ui::PasswordDialog passwordDialog;
    passwordDialog.setupUi(&dialog);

    passwordDialog.iconLabel->setText(QString());
    passwordDialog.iconLabel->setPixmap(mainWindow->style()->standardIcon(QStyle::SP_MessageBoxQuestion, 0, mainWindow).pixmap(32, 32));

    QString introMessage = i18n("<qt>Enter username and password for ") + 
                            Qt::escape(reply->url().toString()) + i18n(" at ") + Qt::escape(reply->url().toString()) + "</qt>";
    passwordDialog.introLabel->setText(introMessage);
    passwordDialog.introLabel->setWordWrap(true);

    if (dialog.exec() == QDialog::Accepted) 
    {
        auth->setUser(passwordDialog.userNameLineEdit->text());
        auth->setPassword(passwordDialog.passwordLineEdit->text());
    }
}

void NetworkAccessManager::proxyAuthenticationRequired(const QNetworkProxy &proxy, QAuthenticator *auth)
{
    MainWindow *mainWindow = Application::instance()->mainWindow();

    QDialog dialog(mainWindow);
    dialog.setWindowFlags(Qt::Sheet);

    Ui::ProxyDialog proxyDialog;
    proxyDialog.setupUi(&dialog);

    proxyDialog.iconLabel->setText(QString());
    proxyDialog.iconLabel->setPixmap(mainWindow->style()->standardIcon(QStyle::SP_MessageBoxQuestion, 0, mainWindow).pixmap(32, 32));

    QString introMessage = i18n("<qt>Connect to proxy ") + Qt::escape(proxy.hostName()) + i18n(" using:</qt>");
    proxyDialog.introLabel->setText(introMessage);
    proxyDialog.introLabel->setWordWrap(true);

    if (dialog.exec() == QDialog::Accepted) 
    {
        auth->setUser(proxyDialog.userNameLineEdit->text());
        auth->setPassword(proxyDialog.passwordLineEdit->text());
    }
}

#ifndef QT_NO_OPENSSL
void NetworkAccessManager::sslErrors(QNetworkReply *reply, const QList<QSslError> &error)
{
    MainWindow *mainWindow = Application::instance()->mainWindow();

    QStringList errorStrings;
    for (int i = 0; i < error.count(); ++i)
        errorStrings += error.at(i).errorString();
    QString errors = errorStrings.join(QLatin1String("\n"));
    int ret = KMessageBox::warningYesNo( mainWindow, i18n("SSL Errors:\n\n") + reply->url().toString() + "\n\n" + QString(errors) + "\n\n");
    if (ret == KMessageBox::Yes)
        reply->ignoreSslErrors();
}
#endif
