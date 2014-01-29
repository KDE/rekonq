/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2009-2014 by Andrea Diamantini <adjam7 at gmail dot com>
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

// Auto Includes
#include "rekonq.h"

// KDE Includes
#include <KLocalizedString>

// Qt Includes
#include <QAction>
#include <QIcon>


WalletBar::WalletBar(QWidget *parent)
    : KMessageWidget(parent)
{
    setMessageType(KMessageWidget::Warning);

    QSize sz = size();
    sz.setWidth(qobject_cast<QWidget *>(parent)->size().width());
    resize(sz);

    setCloseButtonVisible(false);

    QAction *rememberAction = new QAction(QIcon::fromTheme("document-save"), i18n("Remember"), this);
    connect(rememberAction, SIGNAL(triggered(bool)), this, SLOT(rememberData()));
    addAction(rememberAction);

    QAction *neverHereAction = new QAction(QIcon::fromTheme("process-stop"), i18n("Never for This Site"), this);
    connect(neverHereAction, SIGNAL(triggered(bool)), this, SLOT(neverRememberData()));
    addAction(neverHereAction);

    QAction *notNowAction = new QAction(QIcon::fromTheme("dialog-cancel"), i18n("Not Now"), this);
    connect(notNowAction, SIGNAL(triggered(bool)), this, SLOT(notNowRememberData()));
    addAction(notNowAction);
}


void WalletBar::rememberData()
{
    emit saveFormDataAccepted(m_key);

    animatedHide();
    deleteLater();
}


void WalletBar::neverRememberData()
{
    // add url to the blacklist
    QStringList list = ReKonfig::walletBlackList();
    list << m_url.toString();
    ReKonfig::setWalletBlackList(list);

    notNowRememberData();
}


void WalletBar::notNowRememberData()
{
    emit saveFormDataRejected(m_key);

    animatedHide();
    deleteLater();
}



void WalletBar::onSaveFormData(const QString &key, const QUrl &url)
{
    setText(i18n("Do you want rekonq to remember the password on %1?", url.host()));

    m_key = key;
    m_url = url;
}
