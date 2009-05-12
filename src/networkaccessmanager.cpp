/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2007-2008 Trolltech ASA. All rights reserved
* Copyright (C) 2008-2009 by Andrea Diamantini <adjam7 at gmail dot com>
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
#include "networkaccessmanager.h"
#include "networkaccessmanager.moc"

// Local Includes
#include "application.h"
#include "mainwindow.h"

// Auto Includes
#include "rekonq.h"

// Ui Includes
#include "ui_password.h"
#include "ui_proxy.h"

// KDE Includes
#include <KMessageBox>
#include <KStandardDirs>

// Qt Includes
#include <QtCore/QPointer>

#include <QtGui/QDialog>
#include <QtGui/QStyle>
#include <QtGui/QTextDocument>

#include <QtNetwork/QAuthenticator>
#include <QtNetwork/QNetworkProxy>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QSslError>


NetworkAccessManager::NetworkAccessManager(QObject *parent)
        : QNetworkAccessManager(parent)
{
    connect(this, SIGNAL(authenticationRequired(QNetworkReply*, QAuthenticator*)),
            SLOT(authenticationRequired(QNetworkReply*, QAuthenticator*)));
    connect(this, SIGNAL(proxyAuthenticationRequired(const QNetworkProxy&, QAuthenticator*)),
            SLOT(proxyAuthenticationRequired(const QNetworkProxy&, QAuthenticator*)));

#ifndef QT_NO_OPENSSL
    connect(this, SIGNAL(sslErrors(QNetworkReply*, const QList<QSslError>&)),
            SLOT(sslErrors(QNetworkReply*, const QList<QSslError>&)));
#endif

    loadSettings();

    QNetworkDiskCache *diskCache = new QNetworkDiskCache(this);
    QString location = KStandardDirs::locateLocal("cache", "", true);
    diskCache->setCacheDirectory(location);
    setCache(diskCache);
}


void NetworkAccessManager::loadSettings()
{
    QNetworkProxy proxy;
    if (ReKonfig::isProxyEnabled())
    {
        if (ReKonfig::proxyType() == 0)
        {
            proxy.setType(QNetworkProxy::Socks5Proxy);
        }
        else
        {
            proxy.setType(QNetworkProxy::HttpProxy);
        }
        proxy.setHostName(ReKonfig::proxyHostName());
        proxy.setPort(ReKonfig::proxyPort());
        proxy.setUser(ReKonfig::proxyUserName());
        proxy.setPassword(ReKonfig::proxyPassword());
    }
    setProxy(proxy);
}



void NetworkAccessManager::authenticationRequired(QNetworkReply *reply, QAuthenticator *auth)
{
    MainWindow *mainWindow = Application::instance()->mainWindow();

    QPointer<KDialog> dialog = new KDialog(mainWindow, Qt::Sheet);
    dialog->setButtons(KDialog::Ok | KDialog::Cancel);

    Ui::passwordWidget passwordWidget;
    QWidget widget;
    passwordWidget.setupUi(&widget);

    dialog->setMainWidget(&widget);

    passwordWidget.iconLabel->setText(QString());
    passwordWidget.iconLabel->setPixmap(mainWindow->style()->standardIcon(QStyle::SP_MessageBoxQuestion, 0, mainWindow).pixmap(32, 32));

    QString introMessage = i18n("<qt>Enter username and password for %1 at %2</qt>",
                                  Qt::escape(reply->url().toString()),
                                  Qt::escape(reply->url().toString())  );
    passwordWidget.introLabel->setText(introMessage);
    passwordWidget.introLabel->setWordWrap(true);

    if (dialog->exec() == QDialog::Accepted)
    {
        auth->setUser(passwordWidget.userNameLineEdit->text());
        auth->setPassword(passwordWidget.passwordLineEdit->text());
    }
    delete dialog;
}


void NetworkAccessManager::proxyAuthenticationRequired(const QNetworkProxy &proxy, QAuthenticator *auth)
{
    MainWindow *mainWindow = Application::instance()->mainWindow();

    QPointer<KDialog> dialog = new KDialog(mainWindow, Qt::Sheet);
    dialog->setButtons(KDialog::Ok | KDialog::Cancel);

    Ui::proxyWidget proxyWdg;
    QWidget widget;
    proxyWdg.setupUi(&widget);

    dialog->setMainWidget(&widget);

    proxyWdg.iconLabel->setText(QString());
    proxyWdg.iconLabel->setPixmap(mainWindow->style()->standardIcon(QStyle::SP_MessageBoxQuestion, 0, mainWindow).pixmap(32, 32));

    QString introMessage = i18n("<qt>Connect to proxy %1 using:</qt>", Qt::escape(proxy.hostName()) );
    proxyWdg.introLabel->setText(introMessage);
    proxyWdg.introLabel->setWordWrap(true);

    if (dialog->exec() == QDialog::Accepted)
    {
        auth->setUser(proxyWdg.userNameLineEdit->text());
        auth->setPassword(proxyWdg.passwordLineEdit->text());
    }
    delete dialog;
}


#ifndef QT_NO_OPENSSL
void NetworkAccessManager::sslErrors(QNetworkReply *reply, const QList<QSslError> &error)
{
    MainWindow *mainWindow = Application::instance()->mainWindow();

    QStringList errorStrings;
    for (int i = 0; i < error.count(); ++i)
        errorStrings += error.at(i).errorString();
    QString errors = errorStrings.join(QLatin1String("\n"));
    int ret = KMessageBox::warningYesNo(mainWindow, i18n("SSL Errors:\n\n") + reply->url().toString() + "\n\n" + QString(errors) + "\n\n");
    if (ret == KMessageBox::Yes)
        reply->ignoreSslErrors();
}
#endif
