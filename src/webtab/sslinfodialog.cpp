/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2011-2014 by Andrea Diamantini <adjam7 at gmail dot com>
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

// KDE Includes
#include <KLocalizedString>
#include <ktcpsocket.h>

// Qt Includes
#include <QDate>
#include <QFile>
#include <QFileDialog>
#include <QLabel>
#include <QPushButton>
#include <QSslCertificate>


SslInfoDialog::SslInfoDialog(const QString &host, const WebSslInfo &info, QWidget *parent)
    : QDialog(parent)
    , _host(host)
    , _info(info)
{
    setWindowTitle(i18n("Rekonq SSL Information"));
    setAttribute(Qt::WA_DeleteOnClose);

    setMinimumWidth(300);

    _buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Close); 
    
    _buttonBox->button(QDialogButtonBox::Ok)->setIcon(QIcon::fromTheme(QL1S("view-certificate-export")));
    _buttonBox->button(QDialogButtonBox::Ok)->setText(i18n("Export"));

    connect(_buttonBox, SIGNAL(accepted()), this, SLOT(exportCert()));
    connect(_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    // the session widget
    QWidget *widget = new QWidget(this);
    _ui.setupUi(widget);
    
    // insert everything inside the dialog...
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(widget);
    mainLayout->addWidget(_buttonBox);
    setLayout(mainLayout);
    
    // ------------------------------------------------
    QList<QSslCertificate> caList = _info.certificateChain();

    Q_FOREACH(const QSslCertificate & cert, caList)
    {
        QString name = cert.subjectInfo(QSslCertificate::CommonName).at(0);
        if (name.isEmpty())
            name = cert.subjectInfo(QSslCertificate::Organization).at(0);
        if (name.isEmpty())
            name = QL1S(cert.serialNumber());
        _ui.comboBox->addItem(name);
    }
    connect(_ui.comboBox, SIGNAL(activated(int)), this, SLOT(displayFromChain(int)));

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
    _ui.certInfoLabel->setText(c);

    _ui.subjectCN->setText( subjectCert.subjectInfo(QSslCertificate::CommonName).at(0).toHtmlEscaped() );
    _ui.subjectO->setText( subjectCert.subjectInfo(QSslCertificate::Organization).at(0).toHtmlEscaped() );
    _ui.subjectOU->setText( subjectCert.subjectInfo(QSslCertificate::OrganizationalUnitName).at(0).toHtmlEscaped() );
    _ui.subjectSN->setText( QString::fromLatin1(subjectCert.serialNumber()) );

    _ui.issuerCN->setText( subjectCert.issuerInfo(QSslCertificate::CommonName).at(0).toHtmlEscaped() );
    _ui.issuerO->setText( subjectCert.issuerInfo(QSslCertificate::Organization).at(0).toHtmlEscaped() );
    _ui.issuerOU->setText( subjectCert.issuerInfo(QSslCertificate::OrganizationalUnitName).at(0).toHtmlEscaped() );

    _ui.issuedOn->setText( subjectCert.effectiveDate().date().toString(Qt::SystemLocaleShortDate).toHtmlEscaped().at(0) );
    _ui.expiresOn->setText( subjectCert.expiryDate().date().toString(Qt::SystemLocaleShortDate).toHtmlEscaped().at(0) );

    _ui.md5->setText(QString::fromLatin1(subjectCert.digest(QCryptographicHash::Md5).toHex()));
    _ui.sha1->setText(QString::fromLatin1(subjectCert.digest(QCryptographicHash::Sha1).toHex()));
}


void SslInfoDialog::displayFromChain(int i)
{
    QList<QSslCertificate> caList = _info.certificateChain();
    QSslCertificate cert = caList.at(i);

    QStringList errors = SslInfoDialog::errorsFromString(_info.certificateErrors()).at(i);

    if (errors.isEmpty())
    {
        QStringList certInfo;
        certInfo << i18n("The certificate is valid");
        showCertificateInfo(cert, certInfo);
    }
    else
    {
        errors.prepend(i18n("The certificate for this site is not valid for the following reasons:"));
        showCertificateInfo(cert, errors);
    }
}


void SslInfoDialog::exportCert()
{
    QSslCertificate cert = _info.certificateChain().at(_ui.comboBox->currentIndex());

    if (cert.isNull())
        return;

    QString name = _host + QL1S(".pem");

    QString certPath = QFileDialog::getSaveFileName(this, name, QString());
 
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

    QStringList sl1 = s.split(QL1C('\n'), QString::KeepEmptyParts);

    Q_FOREACH(const QString & certErrors, sl1)
    {
        QStringList errors;
        QStringList sl = certErrors.split(QL1C('\t'), QString::SkipEmptyParts);
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
