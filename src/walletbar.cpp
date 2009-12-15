/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2009 by Andrea Diamantini <adjam7 at gmail dot com>
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
#include "walletbar.h"
#include "walletbar.moc"

// KDE Includes
#include <klocalizedstring.h>
#include <KIcon>

// Qt Includes
#include <QToolButton>
#include <QPushButton>
#include <QHBoxLayout>
#include <QString>


WalletBar::WalletBar(QWidget *parent)
    : QWidget(parent)
    , m_label( new QLabel(this) )
{
    m_label->setWordWrap(true);

    QToolButton *closeButton = new QToolButton(this);
    closeButton->setAutoRaise(true);
    closeButton->setIcon(KIcon("dialog-close"));

    QPushButton *rememberButton = new QPushButton(KIcon("document-save"), i18n("Remember"), this);
    QPushButton *neverHereButton = new QPushButton(KIcon("process-stop"), i18n("Never for This Site"), this);
    QPushButton *notNowButton = new QPushButton(KIcon("dialog-cancel"), i18n("Not Now"), this);

    connect(closeButton, SIGNAL(clicked()), this, SLOT(notNowRememberData()));
    connect(rememberButton, SIGNAL(clicked()), this, SLOT(rememberData()));
    connect(neverHereButton, SIGNAL(clicked()), this, SLOT(neverRememberData()));
    connect(notNowButton, SIGNAL(clicked()), this, SLOT(notNowRememberData()));
        
    // layout
    QGridLayout *layout = new QGridLayout(this);
    layout->addWidget(closeButton,0,0);
    layout->addWidget(m_label,0,1);
    layout->addWidget(rememberButton,0,2);
    layout->addWidget(neverHereButton,0,3);
    layout->addWidget(notNowButton,0,4);
    layout->setColumnStretch(1,100);

    setLayout(layout);
}


WalletBar::~WalletBar()
{
}


void WalletBar::rememberData()
{
    emit saveFormDataAccepted(m_key);
    destroy();
}


void WalletBar::neverRememberData()
{
    // TODO: store site url (to remember never bother about)
    notNowRememberData();
}


void WalletBar::notNowRememberData()
{
    emit saveFormDataRejected (m_key);
    destroy();
}


void WalletBar::destroy()
{
    if (parentWidget() && parentWidget()->layout())
    {
        parentWidget()->layout()->removeWidget(this);
    }
    this->deleteLater();
}


void WalletBar::onSaveFormData(const QString &key, const QUrl &url)
{
    m_label->setText( i18n("Do you want rekonq to remember the password on %1?", url.host() ) );

    m_key = key;
    m_url = url;
}
