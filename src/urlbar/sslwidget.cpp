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
#include "iconmanager.h"
#include "mainwindow.h"
#include "webtab.h"

// KDE Includes

// Qt Includes
#include <QtGui/QDialogButtonBox>
#include <QtGui/QFormLayout>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>


SSLWidget::SSLWidget(const QUrl &url, const WebSslInfo &info, QWidget *parent)
    : QMenu(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setMinimumWidth(500);

    QFormLayout *layout = new QFormLayout(this);

    QLabel *l;
    
    // Title
    QLabel *title = new QLabel(this);
    title->setText( QL1S("<h4>") + url.toString() + QL1S("</h4>") );
    title->setAlignment(Qt::AlignCenter);
    layout->addRow(title);

    QLabel *u = new QLabel(this);
    u->setText( info.url().toString() );
    layout->addRow(u);
    
    QLabel *hAdd = new QLabel(this);
    hAdd->setText( QL1S("Peer Address: ") + info.peerAddress().toString() );
    layout->addRow(hAdd);

    QLabel *pAdd = new QLabel(this);
    pAdd->setText( QL1S("Parent Address: ") + info.parentAddress().toString() );
    layout->addRow(pAdd);

    QLabel *cip = new QLabel(this);
    cip->setText( QL1S("Cipher: ") + info.ciphers() );
    layout->addRow(cip);

    QLabel *pr = new QLabel(this);
    pr->setText( QL1S("Protocol: ") + info.protocol() );
    layout->addRow(pr);

    QLabel *epr = new QLabel(this);
    epr->setText( QL1S("CA errors: ") + info.certificateErrors() );
    layout->addRow(epr);

    QList<QSslCertificate> caList = info.certificateChain();
    Q_FOREACH(const QSslCertificate &cert, caList)
    {
        QLabel *a = new QLabel(this);
        a->setText( QL1S("ORGANIZATION: ") + cert.issuerInfo(QSslCertificate::Organization) );
        layout->addRow(a);
        
        QLabel *b = new QLabel(this);
        b->setText( QL1S("CN: ") + cert.issuerInfo(QSslCertificate::CommonName) );
        layout->addRow(b);
        
        QLabel *c = new QLabel(this);
        c->setText( QL1S("LOCALITY NAME: ") + cert.issuerInfo(QSslCertificate::LocalityName) );
        layout->addRow(c);
        
        QLabel *d = new QLabel(this);
        d->setText( QL1S("ORGANIZATION UNIT NAME: ") + cert.issuerInfo(QSslCertificate::OrganizationalUnitName) );
        layout->addRow(d);
        
        QLabel *e = new QLabel(this);
        e->setText( QL1S("COUNTRY NAME: ") + cert.issuerInfo(QSslCertificate::CountryName) );
        layout->addRow(e);
        
        QLabel *f = new QLabel(this);
        f->setText( QL1S("STATE OR PROVINCE NAME: ") + cert.issuerInfo(QSslCertificate::StateOrProvinceName) );
        layout->addRow(f);
    }

// ----------------------------------------------------------------
    l = new QLabel(this);
    l->setText( QL1S("<h4>") + i18n("Site Information") + QL1S("</h4>") );
    layout->addRow(l);
    
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
