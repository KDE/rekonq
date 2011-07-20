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
#include <ktcpsocket.h>


SslInfoDialog::SslInfoDialog(const QString &host, const WebSslInfo &info, QWidget *parent)
    : KDialog(parent)
    , m_host(host)
    , m_info(info)
{
    setCaption(i18n("Rekonq SSL Information"));
    setAttribute(Qt::WA_DeleteOnClose);

    setMinimumWidth(300);

    setButtons(KDialog::User1 | KDialog::Close);

    setButtonGuiItem(User1, KGuiItem(i18n("Export"), "view-certificate-export"));
    connect(this, SIGNAL(user1Clicked()), this, SLOT(exportCert()));

    ui.setupUi(mainWidget());

    // ------------------------------------------------
    QList<QSslCertificate> caList = m_info.certificateChain();

    Q_FOREACH(const QSslCertificate & cert, caList)
    {
        ui.comboBox->addItem(cert.subjectInfo(QSslCertificate::CommonName));
    }
    connect(ui.comboBox, SIGNAL(activated(int)), this, SLOT(displayFromChain(int)));

    displayFromChain(0);
}


void SslInfoDialog::showCertificateInfo(QSslCertificate subjectCert, const QStringList &certErrors)
{
    QStringList sl = certErrors;
    QString c = sl.takeFirst();
    c += QL1S("<ul>");
    Q_FOREACH(const QString & s, sl)
    {
        c += QL1S("<li>") + s + QL1S("</li>");
    }
    c += QL1S("</ul>");
    ui.certInfoLabel->setText(c);

    // WARNING (Security Issue): set these labels to use PlainText!
    ui.subjectCN->setText(subjectCert.subjectInfo(QSslCertificate::CommonName));
    ui.subjectCN->setTextFormat(Qt::PlainText);
    
    ui.subjectO->setText(subjectCert.subjectInfo(QSslCertificate::Organization));
    ui.subjectO->setTextFormat(Qt::PlainText);

    ui.subjectOU->setText(subjectCert.subjectInfo(QSslCertificate::OrganizationalUnitName));
    ui.subjectOU->setTextFormat(Qt::PlainText);

    ui.subjectSN->setText(subjectCert.serialNumber());
    ui.subjectSN->setTextFormat(Qt::PlainText);

    ui.issuerCN->setText(subjectCert.issuerInfo(QSslCertificate::CommonName));
    ui.issuerCN->setTextFormat(Qt::PlainText);

    ui.issuerO->setText(subjectCert.issuerInfo(QSslCertificate::Organization));
    ui.issuerO->setTextFormat(Qt::PlainText);

    ui.issuerOU->setText(subjectCert.issuerInfo(QSslCertificate::OrganizationalUnitName));
    ui.issuerOU->setTextFormat(Qt::PlainText);

    ui.issuedOn->setText(subjectCert.effectiveDate().date().toString(Qt::SystemLocaleShortDate));
    ui.issuedOn->setTextFormat(Qt::PlainText);

    ui.expiresOn->setText(subjectCert.expiryDate().date().toString(Qt::SystemLocaleShortDate));
    ui.expiresOn->setTextFormat(Qt::PlainText);

    ui.md5->setText(subjectCert.digest(QCryptographicHash::Md5).toHex());
    ui.md5->setTextFormat(Qt::PlainText);

    ui.sha1->setText(subjectCert.digest(QCryptographicHash::Sha1).toHex());
    ui.sha1->setTextFormat(Qt::PlainText);
}


void SslInfoDialog::displayFromChain(int i)
{
    QList<QSslCertificate> caList = m_info.certificateChain();
    QSslCertificate cert = caList.at(i);

    QStringList errors = SslInfoDialog::errorsFromString(m_info.certificateErrors()).at(i);

    if(cert.isValid() && errors.isEmpty())
    {
        QStringList certInfo;
        certInfo << i18n("The Certificate is Valid!");
        showCertificateInfo(cert, certInfo);
    }
    else
    {
        errors.prepend(i18n("The certificate for this site is NOT valid for the following reasons:"));
        showCertificateInfo(cert, errors);
    }
}


void SslInfoDialog::exportCert()
{
    QSslCertificate cert = m_info.certificateChain().at(ui.comboBox->currentIndex());

    QString name = cert.subjectInfo(QSslCertificate::CommonName) + QL1S(".pem");

    QString certPath = KFileDialog::getSaveFileName(name, QString(), this);

    QFile file(certPath);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QTextStream out(&file);
    out << cert.toPem();
}


// static -------------------------------------------------------------------------------------------
QList<QStringList> SslInfoDialog::errorsFromString(const QString &s)
{
    QList<QStringList> resultList;

    QStringList sl1 = s.split('\n', QString::KeepEmptyParts);

    Q_FOREACH(const QString & certErrors, sl1)
    {
        QStringList errors;
        QStringList sl = certErrors.split("\t", QString::SkipEmptyParts);
        Q_FOREACH(const QString & s, sl)
        {
            bool didConvert;
            KSslError::Error error = static_cast<KSslError::Error>(s.trimmed().toInt(&didConvert));
            if(didConvert)
            {
                errors << KSslError(error).errorString();
            }
        }
        resultList << errors;
    }
    return resultList;
}
