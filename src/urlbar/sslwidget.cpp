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

// Qt Includes
#include <QtGui/QDialogButtonBox>
#include <QtGui/QFormLayout>
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
    
    QFormLayout *layout = new QFormLayout(this);

    QLabel *label;
    
    // ------------------------------------------------------------------------------------------------------------------
    label = new QLabel(this);
    label->setWordWrap(true);
    label->setText( i18n("<h4>Identity</h4>") );
    layout->addRow(label);

    if (cert.isNull())
    {
        label = new QLabel(this);
        label->setWordWrap(true);
        label->setText( i18n("Warning: this site is carrying a NULL certificate") );
        layout->addRow(label);
    }
    else
    {
        if (cert.isValid())
        {
            label = new QLabel(this);
            label->setWordWrap(true);
            label->setText( i18n("This certificate for this site is Valid and has been verified by ") 
                            + cert.issuerInfo(QSslCertificate::CommonName) );
            layout->addRow(label);
        }
        else
        {
            label = new QLabel(this);
            label->setWordWrap(true);
            label->setText( i18n("Warning: The certificate for this site is NOT valid!") );
            layout->addRow(label);
        }
    }
    
    label = new QLabel(this);
    label->setWordWrap(true);
    label->setText("<a href=\"moresslinfos\">Certificate Information</a>");
    connect(label, SIGNAL(linkActivated(const QString &)), this, SLOT(showMoreSslInfos(const QString &)));
    layout->addRow(label);
    
    // ------------------------------------------------------------------------------------------------------------------
    label = new QLabel(this);
    label->setWordWrap(true);
    label->setText( QL1S("<hr /><h4>Encryption</h4>") ); // ----------------------------------------------- //
    layout->addRow(label);

    label = new QLabel(this);
    label->setWordWrap(true);
    label->setText( i18n("Your connection to %1 is encrypted with %2 encryption\n\n", m_url.host(), m_info.supportedChiperBits()) );
    layout->addRow(label);
    
    QString sslVersion = QL1S("SSLv") + cert.version();
    label = new QLabel(this);
    label->setWordWrap(true);
    label->setText( i18n("The connection uses %1\n\n", sslVersion) );
    layout->addRow(label);
    
    const QStringList cipherInfo = m_info.ciphers().split('\n', QString::SkipEmptyParts);
    label = new QLabel(this);
    label->setWordWrap(true);
    label->setText( i18n("The connection is encrypted using %1 at %2 bits with %3 for message authentication and %4 as the key exchange mechanism.\n\n",
        cipherInfo[0],
        m_info.usedChiperBits(),
        cipherInfo[3],
        cipherInfo[1])
                         
    );
    layout->addRow(label);
    
    // ------------------------------------------------------------------------------------------------------------------
    label = new QLabel(this);
    label->setWordWrap(true);
    label->setText( i18n("<hr /><h4>Site Information</h4>") );
    layout->addRow(label);

    label = new QLabel(this);
    label->setWordWrap(true);

    if (rApp->historyManager()->historyContains(url.toString())) //FIXME change with visit count > 1
    {
        label->setText( i18n("You just visited this site") );
    }
    else
    {
        label->setText( i18n("It's your first time visiting this site") );            
    }
    layout->addRow(label);

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
