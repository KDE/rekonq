/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2007-2008 Trolltech ASA. All rights reserved
* Copyright (C) 2008-2009 by Andrea Diamantini <adjam7 at gmail dot com>*
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
#include <QtCore/QIODevice>

#include <QtGui/QStyle>
#include <QtGui/QTextDocument>

#include <QtNetwork/QAuthenticator>
#include <QtNetwork/QNetworkDiskCache>



NetworkAccessManager::NetworkAccessManager(QObject *parent)
        : AccessManager(parent)
        , m_diskCache(0)
{
    connect(this, SIGNAL(authenticationRequired(QNetworkReply*, QAuthenticator*)),
            SLOT(authenticationRequired(QNetworkReply*, QAuthenticator*)));
    connect(this, SIGNAL(proxyAuthenticationRequired(const QNetworkProxy&, QAuthenticator*)),
            SLOT(proxyAuthenticationRequired(const QNetworkProxy&, QAuthenticator*)));

#ifndef QT_NO_OPENSSL
    connect(this, SIGNAL(sslErrors(QNetworkReply*, const QList<QSslError>&)),
            SLOT(slotSSLErrors(QNetworkReply*, const QList<QSslError>&)));
#endif

    // load AccessManager Settings
    loadSettings();
    
    // resetting disk cache
    resetDiskCache();
}


void NetworkAccessManager::loadSettings()
{
    if (ReKonfig::isProxyEnabled())
    {
        QNetworkProxy proxy;
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
 
       setProxy(proxy);
    }
}


void NetworkAccessManager::resetDiskCache()
{
    if(!m_diskCache)
    {
        m_diskCache = new QNetworkDiskCache(this);
        QString location = KStandardDirs::locateLocal("cache", "", true);
        kDebug() << location;
        
        m_diskCache->setCacheDirectory(location);
        setCache(m_diskCache);
    }
    else
    {
        QString location = m_diskCache->cacheDirectory();
            
        QDir cacheDir(location + QString("/http") );
        QStringList fileList = cacheDir.entryList();
        foreach(QString str, fileList)
        {
            QFile file(str);
            file.remove();
        }
    }
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
    //FIXME Replace the text below with an explanation of what exactly %1 and %2 stand for
    QString introMessage = i18nc("%1=stuff %2=stuff2", "<qt>Enter username and password for %1 at %2</qt>",
                                  Qt::escape(reply->url().toString()),
                                  Qt::escape(reply->url().toString())  );
    passwordWidget.introLabel->setText(introMessage);
    passwordWidget.introLabel->setWordWrap(true);

    if (dialog->exec() == KDialog::Ok)
    {
        auth->setUser(passwordWidget.userNameLineEdit->text());
        auth->setPassword(passwordWidget.passwordLineEdit->text());
    }
    dialog->deleteLater();
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
    //FIXME Connect to proxy %1 using what? Best solution would be adding a %2 after the "using:" part and explain %1 and %2 in an i18nc call
    QString introMessage = i18n("<qt>Connect to proxy %1 using:</qt>", Qt::escape(proxy.hostName()) );
    proxyWdg.introLabel->setText(introMessage);
    proxyWdg.introLabel->setWordWrap(true);

    if (dialog->exec() == KDialog::Ok)
    {
        auth->setUser(proxyWdg.userNameLineEdit->text());
        auth->setPassword(proxyWdg.passwordLineEdit->text());
    }
    dialog->deleteLater();
}


#ifndef QT_NO_OPENSSL
void NetworkAccessManager::slotSSLErrors(QNetworkReply *reply, const QList<QSslError> &error)
{
    MainWindow *mainWindow = Application::instance()->mainWindow();

    QStringList errorStrings;
    for (int i = 0; i < error.count(); ++i)
        errorStrings += error.at(i).errorString();
    QString errors = errorStrings.join(QLatin1String("\n"));
    int ret = KMessageBox::warningContinueCancel(mainWindow, 
                    i18n("SSL Errors:\n\n") + reply->url().toString() + "\n\n" + QString(errors) + "\n\n");

    if (ret == KMessageBox::Yes)
        reply->ignoreSslErrors();
}
#endif


KIO::MetaData& NetworkAccessManager::metaData()
{
    return m_metaData;
};


QNetworkReply *NetworkAccessManager::createRequest(Operation op, const QNetworkRequest &req, QIODevice *outgoingData)
{
    // FIXME: rude hack. Waiting for a real POST behaviour fix ;)
    if(op == PostOperation)
    {
        return QNetworkAccessManager::createRequest(op,req,outgoingData);
    }
    return AccessManager::createRequest(op,req,outgoingData);
}
