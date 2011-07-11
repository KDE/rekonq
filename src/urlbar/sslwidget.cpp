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


// Auto Includes
#include "sslwidget.h"
#include "sslwidget.moc"

// Local includes
#include "application.h"
#include "historymanager.h"
#include "sslinfodialog.h"

// KDE Includes
#include <QSslError>

// Qt Includes
#include <QtGui/QDialogButtonBox>
#include <QtGui/QGridLayout>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>


SSLWidget::SSLWidget(const QUrl &url, const WebSslInfo &info, QWidget *parent)
    : QMenu(parent)
    , m_url(url)
    , m_info(info)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setMinimumWidth(400);

    QSslCertificate cert = info.certificateChain().first();
    
    QGridLayout *layout = new QGridLayout(this);

    QLabel *label;
    QLabel *imageLabel;

    int rows = 0;
    // ------------------------------------------------------------------------------------------------------------------
    imageLabel = new QLabel(this);
    layout->addWidget(imageLabel, rows , 0, Qt::AlignCenter);

    label = new QLabel(this);
    label->setWordWrap(true);
    label->setText( i18n("<h4>Identity</h4>") );
    layout->addWidget(label, rows++, 1);
    
    if (cert.isNull())
    {
        label = new QLabel(this);
        label->setWordWrap(true);
        label->setText( i18n("Warning: this site is carrying a NULL certificate!") );
        
        imageLabel->setPixmap(KIcon("security-low").pixmap(32));
    }
    else
    {
        if (cert.isValid())
        {
            label = new QLabel(this);
            label->setWordWrap(true);
            label->setText( i18n("This certificate for this site is valid and has been verified by:\n%1.", 
                                 cert.issuerInfo(QSslCertificate::CommonName)) );
            
            
            imageLabel->setPixmap(KIcon("security-high").pixmap(32));
        }
        else
        {
            QString errors;
            QStringList sl = m_info.certificateErrors().split("\t", QString::SkipEmptyParts);
            Q_FOREACH(const QString &s, sl)
            {
                bool didConvert;
                QSslError::SslError error = static_cast<QSslError::SslError>(s.trimmed().toInt(&didConvert));
                if (didConvert) 
                {
                    errors += QSslError(error).errorString() + QL1S("\n");
                }
            }
            label = new QLabel(this);
            label->setWordWrap(true);
            label->setText( i18n("The certificate for this site is NOT valid for the following reasons:\n%1", errors) );
            
            imageLabel->setPixmap(KIcon("security-medium").pixmap(32));
        }
    }

    layout->addWidget(label, rows++, 1);

    label = new QLabel(this);
    label->setWordWrap(true);
    label->setText("<a href=\"moresslinfos\">Certificate Information</a>");
    connect(label, SIGNAL(linkActivated(const QString &)), this, SLOT(showMoreSslInfos(const QString &)));
    layout->addWidget(label, rows++, 1);
    
    // ------------------------------------------------------------------------------------------------------------------
    label = new QLabel(this);
    label->setWordWrap(true);
    label->setText( QL1S("<h4>Encryption</h4>") ); // ----------------------------------------------- //
    layout->addWidget(label, rows, 1);

    imageLabel = new QLabel(this);
    layout->addWidget(imageLabel, rows++ , 0, Qt::AlignCenter);

    if (cert.isNull())
    {
        label = new QLabel(this);
        label->setWordWrap(true);
        label->setText( i18n("Your connection to %1 is NOT encrypted!!\n\n", m_url.host()) );
        layout->addWidget(label, rows++ , 1);
        
        imageLabel->setPixmap(KIcon("security-low").pixmap(32));
    }
    else
    {
        label = new QLabel(this);
        label->setWordWrap(true);
        label->setText( i18n("Your connection to %1 is encrypted with %2-bit encryption.\n\n", m_url.host(), m_info.supportedChiperBits()) );
        layout->addWidget(label, rows++, 1);
        
        int vers = cert.version().toInt();
        QString sslVersion;
        switch(vers)
        {
        case 0:
            sslVersion = QL1S("SSL 3.0");
            imageLabel->setPixmap(KIcon("security-high").pixmap(32));
            break;
        case 1:
            sslVersion = QL1S("SSL 2.0");
            imageLabel->setPixmap(KIcon("security-medium").pixmap(32));
            break;
        case 2:
        case 3:
            sslVersion = QL1S("TLS 1.0");
            imageLabel->setPixmap(KIcon("security-high").pixmap(32));
            break;
        default:
            sslVersion = QL1S("Unknown");
            imageLabel->setPixmap(KIcon("security-medium").pixmap(32));
        }

        label = new QLabel(this);
        label->setWordWrap(true);
        label->setText( i18n("The connection uses %1.\n\n", sslVersion) );
        layout->addWidget(label, rows++, 1);
        
        const QStringList cipherInfo = m_info.ciphers().split('\n', QString::SkipEmptyParts);
        label = new QLabel(this);
        label->setWordWrap(true);
        label->setText( 
            i18n("The connection is encrypted using %1 at %2 bits with %3 for message authentication and %4 as the key exchange mechanism.\n\n",
            cipherInfo[0],
            m_info.usedChiperBits(),
            cipherInfo[3],
            cipherInfo[1]) );
        layout->addWidget(label, rows++, 1);
        
    }
    
    // ------------------------------------------------------------------------------------------------------------------
    imageLabel = new QLabel(this);
    layout->addWidget(imageLabel, rows , 0, Qt::AlignCenter);

    label = new QLabel(this);
    label->setWordWrap(true);
    label->setText( i18n("<h4>Site Information</h4>") );
    layout->addWidget(label, rows++, 1);

    label = new QLabel(this);
    label->setWordWrap(true);

    HistoryItem firstVisit = rApp->historyManager()->find(url.toString()).first();

    if (firstVisit.visitCount == 1)
    {
        label->setText( i18n("It's your first time visiting this site!") );            
        imageLabel->setPixmap(KIcon("security-medium").pixmap(32));
    }
    else
    {
        label->setText( i18n("You just visited this site!\nYour first visit was on %1.\n", firstVisit.firstDateTimeVisit.toString()) );
        imageLabel->setPixmap(KIcon("security-high").pixmap(32));
    }
    layout->addWidget(label, rows++, 1);

    // -----------------------------------------------------------------------------------
    setLayout(layout);
}


void SSLWidget::showAt(const QPoint &pos)
{
    adjustSize();

    QPoint p(pos.x() - width(), pos.y() + 10);
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
