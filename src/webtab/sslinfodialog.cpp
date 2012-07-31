/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2011-2012 by Andrea Diamantini <adjam7 at gmail dot com>
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

// KDE Includes
#include <KFileDialog>

#include <kglobal.h>
#include <klocale.h>
#include <ktcpsocket.h>

// Qt Includes
#include <QDate>
#include <QFile>

#include <QLabel>
#include <QTextDocument>

#include <QSslCertificate>


SslInfoDialog::SslInfoDialog(const QString &host, const WebSslInfo &info, QWidget *parent)
    : KDialog(parent)
    , m_host(host)
    , m_info(info)
{
    setCaption(i18n("Rekonq SSL Information"));
    setAttribute(Qt::WA_DeleteOnClose);

    setMinimumWidth(300);

    setButtons(KDialog::User1 | KDialog::Close);

    setButtonGuiItem(User1, KGuiItem(i18n("Export"), QL1S("view-certificate-export")));
    connect(this, SIGNAL(user1Clicked()), this, SLOT(exportCert()));

    ui.setupUi(mainWidget());

    // ------------------------------------------------
    QList<QSslCertificate> caList = m_info.certificateChain();

    Q_FOREACH(const QSslCertificate & cert, caList)
    {
        QString name = cert.subjectInfo(QSslCertificate::CommonName);
        if (name.isEmpty())
            name = cert.subjectInfo(QSslCertificate::Organization);
        if (name.isEmpty())
            name = cert.serialNumber();
        ui.comboBox->addItem(name);
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

    ui.subjectCN->setText(Qt::escape(subjectCert.subjectInfo(QSslCertificate::CommonName)));
    ui.subjectO->setText(Qt::escape(subjectCert.subjectInfo(QSslCertificate::Organization)));
    ui.subjectOU->setText(Qt::escape(subjectCert.subjectInfo(QSslCertificate::OrganizationalUnitName)));
    ui.subjectSN->setText(Qt::escape(subjectCert.serialNumber()));

    ui.issuerCN->setText(Qt::escape(subjectCert.issuerInfo(QSslCertificate::CommonName)));
    ui.issuerO->setText(Qt::escape(subjectCert.issuerInfo(QSslCertificate::Organization)));
    ui.issuerOU->setText(Qt::escape(subjectCert.issuerInfo(QSslCertificate::OrganizationalUnitName)));

    ui.issuedOn->setText(Qt::escape(subjectCert.effectiveDate().date().toString(Qt::SystemLocaleShortDate)));
    ui.expiresOn->setText(Qt::escape(subjectCert.expiryDate().date().toString(Qt::SystemLocaleShortDate)));

    ui.md5->setText(Qt::escape(subjectCert.digest(QCryptographicHash::Md5).toHex()));
    ui.sha1->setText(Qt::escape(subjectCert.digest(QCryptographicHash::Sha1).toHex()));
}


void SslInfoDialog::displayFromChain(int i)
{
    QList<QSslCertificate> caList = m_info.certificateChain();
    QSslCertificate cert = caList.at(i);

    QStringList errors = SslInfoDialog::errorsFromString(m_info.certificateErrors()).at(i);

    if (cert.isValid() && errors.isEmpty())
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

    if (cert.isNull())
        return;

    QString name = m_host + QL1S(".pem");

    QString certPath = KFileDialog::getSaveFileName(name, QString(), this);

    QFile file(certPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
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
        QStringList sl = certErrors.split('\t', QString::SkipEmptyParts);
        Q_FOREACH(const QString & s, sl)
        {
            bool didConvert;
            KSslError::Error error = static_cast<KSslError::Error>(s.trimmed().toInt(&didConvert));
            if (didConvert)
            {
                errors << KSslError(error).errorString();
            }
        }
        resultList << errors;
    }
    return resultList;
}
