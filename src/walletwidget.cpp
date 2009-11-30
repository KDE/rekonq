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


#include "walletwidget.h"
#include "walletwidget.moc"

#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>


WalletWidget::WalletWidget(QObject *parent)
    : QWidget(parent)
{
    QLabel *label = new QLabel( i18n("Do you want rekonq to remember the password for %1 on %2?"), this);
    QPushButton *rememberButton = new QPushButton( i18n("remember"), this);
    QPushButton *neverHereButton = new QPushButton( i18n("never for this site"), this);
    QPushButton *notNowButton = new QPushButton( i18n("not now"), this);

    connect(rememberButton, SIGNAL(clicked()), this, SLOT(rememberData()));
    connect(neverHereButton, SIGNAL(clicked()), this, SLOT(neverRememberData()));
    connect(notNowButton, SIGNAL(clicked()), this, SLOT(notNowRememberData()));
        
    // layout
    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(label);
    layout->addWidget(rememberButton);
    layout->addWidget(neverHereButton);
    layout->addWidget(notNowButton);

    setLayout(layout);
}


WalletWidget::~WalletWidget()
{
}


void WalletWidget::rememberData()
{
    WebView *w = Application::instance()->mainWindow()->currentTab();
    w->page()->wallet()->saveFormData(w->page()->currentFrame());
    hide();
}


void WalletWidget::neverRememberData()
{
    hide();
}


void WalletWidget::notNowRememberData()
{
    hide();
}


