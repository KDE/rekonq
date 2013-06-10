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


// Auto Includes
#include "sslwidget.h"
#include "sslwidget.moc"

// Local includes
#include "historymanager.h"
#include "sslinfodialog.h"

// Qt Includes
#include <QPointer>

#include <QDialogButtonBox>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QTextDocument>


const int c_dim = 16;


SSLWidget::SSLWidget(const QUrl &url, const WebSslInfo &info, QWidget *parent)
    : QMenu(parent)
    , m_url(url)
    , m_info(info)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setMinimumWidth(400);

    QList<QSslCertificate> certList = m_info.certificateChain();
    QSslCertificate cert;
    if (!certList.isEmpty())
        cert = certList.first();

    QList<QStringList> certErrorList = SslInfoDialog::errorsFromString(m_info.certificateErrors());
    QStringList firstCertErrorList;
    if (!certErrorList.isEmpty())
        firstCertErrorList = certErrorList.first();

    QGridLayout *layout = new QGridLayout(this);

    QLabel *label;
    QLabel *imageLabel;

    int rows = 0;
    // ------------------------------------------------------------------------------------------------------
    imageLabel = new QLabel(this);
    layout->addWidget(imageLabel, rows , 0, Qt::AlignCenter);

    label = new QLabel(this);
    label->setWordWrap(true);
    label->setText(i18n("Identity"));
    QFont f1 = label->font();
    f1.setBold(true);
    label->setFont(f1);
    layout->addWidget(label, rows++, 1);

    label = new QLabel(this);
    label->setWordWrap(true);
    if (cert.isNull())
    {
        label->setText(i18n("Warning: this site is not carrying a certificate."));
        imageLabel->setPixmap(KIcon("security-low").pixmap(c_dim));
        layout->addWidget(label, rows++, 1);
    }
    else
    {
        if (cert.isValid() && firstCertErrorList.isEmpty())
        {
            label->setText(i18n("The certificate for this site is valid and has been verified by:\n%1.",
                                Qt::escape(cert.issuerInfo(QSslCertificate::CommonName))));

            imageLabel->setPixmap(KIcon("security-high").pixmap(c_dim));
        }
        else
        {
            QString c = QL1S("<ul>");
            Q_FOREACH(const QString & s, firstCertErrorList)
            {
                c += QL1S("<li>") + s + QL1S("</li>");
            }
            c += QL1S("</ul>");

            label->setText(i18n("The certificate for this site is not valid, for the following reasons:\n%1.", c));
            label->setTextFormat(Qt::RichText);
            imageLabel->setPixmap(KIcon("security-low").pixmap(c_dim));
        }

        layout->addWidget(label, rows++, 1);

        label = new QLabel(this);
        label->setWordWrap(true);
        label->setText(QL1S("<a href=\"moresslinfos\">") + i18n("Certificate Information") + QL1S("</a>"));
        connect(label, SIGNAL(linkActivated(QString)), this, SLOT(showMoreSslInfos(QString)));
        layout->addWidget(label, rows++, 1);
    }

    // -------------------------------------------------------------------------------------------------------------
    label = new QLabel(this);
    label->setWordWrap(true);
    label->setText(i18n("Encryption"));
    QFont f2 = label->font();
    f2.setBold(true);
    label->setFont(f2);
    layout->addWidget(label, rows, 1);

    imageLabel = new QLabel(this);
    layout->addWidget(imageLabel, rows++ , 0, Qt::AlignCenter);

    QString ciph = m_info.ciphers();
    if (ciph.isEmpty())   // NOTE: Can I verify encryption in another (better?) way?
    {
        label = new QLabel(this);
        label->setWordWrap(true);
        label->setText(i18n("Your connection to %1 is not encrypted.\n", m_url.host()));
        layout->addWidget(label, rows++ , 1);

        imageLabel->setPixmap(KIcon("security-low").pixmap(c_dim));
    }
    else
    {
        label = new QLabel(this);
        label->setWordWrap(true);
        label->setText(i18n("Your connection to \"%1\" is encrypted.\n", m_url.host()));
        layout->addWidget(label, rows++, 1);

        QString vers = m_info.protocol();
        QString sslVersion;
        if (vers == QL1S("SSLv3"))
        {
            sslVersion = QL1S("SSL 3.0");
            imageLabel->setPixmap(KIcon("security-high").pixmap(c_dim));
        }
        else if (vers == QL1S("SSLv2"))
        {
            sslVersion = QL1S("SSL 2.0");
            imageLabel->setPixmap(KIcon("security-low").pixmap(c_dim));
        }
        else if (vers == QL1S("TLSv1"))
        {
            sslVersion = QL1S("TLS 1.0");
            imageLabel->setPixmap(KIcon("security-high").pixmap(c_dim));
        }
        else if (vers == QL1S("TLSv1.1"))
        {
            sslVersion = QL1S("TLS 1.1");
            imageLabel->setPixmap(KIcon("security-high").pixmap(c_dim));
        }
        else if (vers == QL1S("TLSv1.2"))
        {
            sslVersion = QL1S("TLS 1.2");
            imageLabel->setPixmap(KIcon("security-high").pixmap(c_dim));
        }
        else
        {
            sslVersion = i18n("Unknown");
            imageLabel->setPixmap(KIcon("security-low").pixmap(c_dim));
        }

        label = new QLabel(this);
        label->setWordWrap(true);
        label->setText(i18n("It uses protocol: %1.\n", sslVersion));
        layout->addWidget(label, rows++, 1);

        const QStringList cipherInfo = m_info.ciphers().split('\n', QString::KeepEmptyParts);
        label = new QLabel(this);
        label->setWordWrap(true);
        label->setText(
            i18n("It is encrypted using %1 at %2 bits, with %3 for message authentication and %4 with Auth %5 as key exchange mechanism.\n\n",
                 cipherInfo[0],
                 m_info.usedChiperBits(),
                 cipherInfo[3],
                 cipherInfo[2],
                 cipherInfo[1])
        );
        layout->addWidget(label, rows++, 1);

    }

    // ------------------------------------------------------------------------------------------------------------------
    imageLabel = new QLabel(this);
    layout->addWidget(imageLabel, rows , 0, Qt::AlignCenter);

    label = new QLabel(this);
    label->setWordWrap(true);
    label->setText(i18n("Site Information"));
    QFont f3 = label->font();
    f3.setBold(true);
    label->setFont(f3);
    layout->addWidget(label, rows++, 1);

    label = new QLabel(this);
    label->setWordWrap(true);

    QList<HistoryItem> hList = HistoryManager::self()->find(url.toString());
    HistoryItem firstVisit = hList.isEmpty() ?
                             HistoryItem() :
                             hList.first() ;

    imageLabel->setPixmap(KIcon("dialog-information").pixmap(c_dim));
    if (firstVisit.visitCount == 1)
    {
        label->setText(i18n("It is your first time visiting this site."));
    }
    else
    {
        label->setText(i18n("You just visited this site.\nYour first visit was on %1.\n", firstVisit.firstDateTimeVisit.toString()));
    }
    layout->addWidget(label, rows++, 1);

}


void SSLWidget::showAt(const QPoint &pos)
{
    adjustSize();

    QPoint p(pos.x(), pos.y() + 10);
    move(p);
    show();
}


void SSLWidget::accept()
{
    close();
}


void SSLWidget::showMoreSslInfos(const QString &)
{
    QPointer<SslInfoDialog> dlg = new SslInfoDialog(m_url.host(), m_info, this);
    dlg->exec();
    delete dlg;

    return;
}
