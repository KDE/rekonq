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
#include "walletwidget.h"
#include "walletwidget.moc"

// KDE Includes
#include <klocalizedstring.h>

// Qt Includes
#include <QPushButton>
#include <QHBoxLayout>


WalletWidget::WalletWidget(QWidget *parent)
    : QWidget(parent)
    , m_label( new QLabel(this) )
{
    QPushButton *rememberButton = new QPushButton( i18n("Remember"), this);
    QPushButton *neverHereButton = new QPushButton( i18n("Never for This Site"), this);
    QPushButton *notNowButton = new QPushButton( i18n("Not Now"), this);

    connect(rememberButton, SIGNAL(clicked()), this, SLOT(rememberData()));
    connect(neverHereButton, SIGNAL(clicked()), this, SLOT(neverRememberData()));
    connect(notNowButton, SIGNAL(clicked()), this, SLOT(notNowRememberData()));
        
    // layout
    QHBoxLayout *layout = new QHBoxLayout;
    layout->addWidget(m_label);
    layout->addWidget(rememberButton);
    layout->addWidget(neverHereButton);
    layout->addWidget(notNowButton);

    setLayout(layout);
    
    // we start off hidden
    hide();
}


WalletWidget::~WalletWidget()
{
}


void WalletWidget::rememberData()
{
    hide();
    emit saveFormDataAccepted(m_key);
}


void WalletWidget::neverRememberData()
{
    // TODO: store site url (to remember never bother about)
    notNowRememberData();
}


void WalletWidget::notNowRememberData()
{
    hide();
    emit saveFormDataRejected (m_key);
}


void WalletWidget::onSaveFormData(const QString &key, const QUrl &url)
{
    m_label->setText( i18n("Do you want rekonq to remember the password for \"%1\" on \"%2\"?")
                        .arg(key)
                        .arg(url.host())
                    );
    m_key = key;
    m_url = url;
    
    // TODO: check if url is stored somewhere to not remember pass..
    if(true)
        show();
    else
        notNowRememberData();
    
}
