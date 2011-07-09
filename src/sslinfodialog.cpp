/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2011 by Andrea Diamantini <adjam7 at gmail dot com>
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
#include "sslinfodialog.h"
#include "sslinfodialog.moc"

#include <KFileDialog>

#include <QtGui/QFrame>
#include <QtCore/QDate>
#include <QtCore/QFile>
#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtCore/Q_PID>
#include <QtNetwork/QSslCertificate>

#include <QFormLayout>

#include <kglobal.h>
#include <klocale.h>


SslInfoDialog::SslInfoDialog(const QString &host, const WebSslInfo &info, QWidget *parent)
    : KDialog(parent)
    , m_host(host)
    , m_info(info)
{
    setCaption(i18n("Rekonq SSL Information"));
    setAttribute(Qt::WA_DeleteOnClose);

    setMinimumWidth(300);
    
    setButtons(KDialog::User1 | KDialog::Close);
    
    setButtonGuiItem(User1, KGuiItem(i18n("Export"), "view-certificate-export") );
    connect(this, SIGNAL(user1Clicked()), this, SLOT(exportCert()));
    
    ui.setupUi(mainWidget());

    // ------------------------------------------------
    QList<QSslCertificate> caList = m_info.certificateChain();
    
    Q_FOREACH(const QSslCertificate &cert, caList)
    {
        ui.comboBox->addItem( cert.subjectInfo(QSslCertificate::CommonName) );
    }
    connect(ui.comboBox, SIGNAL(activated(int)), this, SLOT(displayFromChain(int)));
        
    QSslCertificate subjectCert = caList.first();
    
    showCertificateInfo(subjectCert);
}


void SslInfoDialog::showCertificateInfo(QSslCertificate subjectCert)
{
    ui.subjectCN->setText( subjectCert.subjectInfo(QSslCertificate::CommonName) );
    ui.subjectO->setText( subjectCert.subjectInfo(QSslCertificate::Organization) );
    ui.subjectOU->setText( subjectCert.subjectInfo(QSslCertificate::OrganizationalUnitName) );
    ui.subjectSN->setText( subjectCert.serialNumber().toHex() );
    
    ui.issuerCN->setText( subjectCert.issuerInfo(QSslCertificate::CommonName) );
    ui.issuerO->setText( subjectCert.issuerInfo(QSslCertificate::Organization) );
    ui.issuerOU->setText( subjectCert.issuerInfo(QSslCertificate::OrganizationalUnitName) );
    
    ui.issuedOn->setText( subjectCert.effectiveDate().date().toString(Qt::SystemLocaleShortDate) );
    ui.expiresOn->setText( subjectCert.expiryDate().date().toString(Qt::SystemLocaleShortDate) );
    ui.sha256->setText( subjectCert.digest(QCryptographicHash::Md5).toHex() );
    ui.sha1->setText( subjectCert.digest(QCryptographicHash::Sha1).toHex() );
    
}


void SslInfoDialog::displayFromChain(int i)
{
    QList<QSslCertificate> caList = m_info.certificateChain();
    QSslCertificate cert = caList.at(i);
    showCertificateInfo(cert);
}


void SslInfoDialog::exportCert()
{
    QSslCertificate cert = m_info.certificateChain().at( ui.comboBox->currentIndex() );

    QString name = cert.subjectInfo(QSslCertificate::CommonName) + QL1S(".pem");
    
    QString certPath = KFileDialog::getSaveFileName(name, QString(), this);

    kDebug() << certPath;
    
    QFile file(certPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

     QTextStream out(&file);
     out << cert.toPem();
}
