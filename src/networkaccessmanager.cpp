/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the demonstration applications of the Qt Toolkit.
**
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License versions 2.0 or 3.0 as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file.  Please review the following information
** to ensure GNU General Public Licensing requirements will be met:
** http://www.fsf.org/licensing/licenses/info/GPLv2.html and
** http://www.gnu.org/copyleft/gpl.html.  In addition, as a special
** exception, Nokia gives you certain additional rights. These rights
** are described in the Nokia Qt GPL Exception version 1.3, included in
** the file GPL_EXCEPTION.txt in this package.
**
** Qt for Windows(R) Licensees
** As a special exception, Nokia, as the sole copyright holder for Qt
** Designer, grants users of the Qt/Eclipse Integration plug-in the
** right for the Qt/Eclipse Integration to link to functionality
** provided by Qt Designer and its related libraries.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
**
****************************************************************************/

#include "networkaccessmanager.h"

#include "browserapplication.h"
#include "browsermainwindow.h"
#include "ui_passworddialog.h"
#include "ui_proxy.h"

#include <QtCore/QSettings>

#include <QtGui/QDialog>
#include <QtGui/QMessageBox>
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
    QSettings settings;
    settings.beginGroup(QLatin1String("proxy"));
    QNetworkProxy proxy;
    if (settings.value(QLatin1String("enabled"), false).toBool()) {
        if (settings.value(QLatin1String("type"), 0).toInt() == 0)
            proxy.setType(QNetworkProxy::Socks5Proxy);
        else
            proxy.setType(QNetworkProxy::HttpProxy);
        proxy.setHostName(settings.value(QLatin1String("hostName")).toString());
        proxy.setPort(settings.value(QLatin1String("port"), 1080).toInt());
        proxy.setUser(settings.value(QLatin1String("userName")).toString());
        proxy.setPassword(settings.value(QLatin1String("password")).toString());
    }
    setProxy(proxy);
}

void NetworkAccessManager::authenticationRequired(QNetworkReply *reply, QAuthenticator *auth)
{
    BrowserMainWindow *mainWindow = BrowserApplication::instance()->mainWindow();

    QDialog dialog(mainWindow);
    dialog.setWindowFlags(Qt::Sheet);

    Ui::PasswordDialog passwordDialog;
    passwordDialog.setupUi(&dialog);

    passwordDialog.iconLabel->setText(QString());
    passwordDialog.iconLabel->setPixmap(mainWindow->style()->standardIcon(QStyle::SP_MessageBoxQuestion, 0, mainWindow).pixmap(32, 32));

    QString introMessage = tr("<qt>Enter username and password for \"%1\" at %2</qt>");
    introMessage = introMessage.arg(Qt::escape(reply->url().toString())).arg(Qt::escape(reply->url().toString()));
    passwordDialog.introLabel->setText(introMessage);
    passwordDialog.introLabel->setWordWrap(true);

    if (dialog.exec() == QDialog::Accepted) {
        auth->setUser(passwordDialog.userNameLineEdit->text());
        auth->setPassword(passwordDialog.passwordLineEdit->text());
    }
}

void NetworkAccessManager::proxyAuthenticationRequired(const QNetworkProxy &proxy, QAuthenticator *auth)
{
    BrowserMainWindow *mainWindow = BrowserApplication::instance()->mainWindow();

    QDialog dialog(mainWindow);
    dialog.setWindowFlags(Qt::Sheet);

    Ui::ProxyDialog proxyDialog;
    proxyDialog.setupUi(&dialog);

    proxyDialog.iconLabel->setText(QString());
    proxyDialog.iconLabel->setPixmap(mainWindow->style()->standardIcon(QStyle::SP_MessageBoxQuestion, 0, mainWindow).pixmap(32, 32));

    QString introMessage = tr("<qt>Connect to proxy \"%1\" using:</qt>");
    introMessage = introMessage.arg(Qt::escape(proxy.hostName()));
    proxyDialog.introLabel->setText(introMessage);
    proxyDialog.introLabel->setWordWrap(true);

    if (dialog.exec() == QDialog::Accepted) {
        auth->setUser(proxyDialog.userNameLineEdit->text());
        auth->setPassword(proxyDialog.passwordLineEdit->text());
    }
}

#ifndef QT_NO_OPENSSL
void NetworkAccessManager::sslErrors(QNetworkReply *reply, const QList<QSslError> &error)
{
    BrowserMainWindow *mainWindow = BrowserApplication::instance()->mainWindow();

    QStringList errorStrings;
    for (int i = 0; i < error.count(); ++i)
        errorStrings += error.at(i).errorString();
    QString errors = errorStrings.join(QLatin1String("\n"));
    int ret = QMessageBox::warning(mainWindow, QCoreApplication::applicationName(),
                           tr("SSL Errors:\n\n%1\n\n%2\n\n"
                              "Do you want to ignore these errors?").arg(reply->url().toString()).arg(errors),
                           QMessageBox::Yes | QMessageBox::No,
                           QMessageBox::No);
    if (ret == QMessageBox::Yes)
        reply->ignoreSslErrors();
}
#endif
