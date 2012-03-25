/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2012 by Andrea Diamantini <adjam7 at gmail dot com>
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
#include "passexceptionswidget.h"
#include "passexceptionswidget.moc"

// Auto Includes
#include "rekonq.h"


PassExWidget::PassExWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUi(this);

    setAttribute(Qt::WA_DeleteOnClose, true);
    
    connect(removeOneButton, SIGNAL(clicked()), this, SLOT(removeOne()));
    connect(removeAllButton, SIGNAL(clicked()), this, SLOT(removeAll()));

    QStringList exList = ReKonfig::walletBlackList();
    Q_FOREACH(const QString &str, exList)
    {
        QListWidgetItem *item = new QListWidgetItem(str, listWidget);
        listWidget->addItem(item);
    }
}


void PassExWidget::removeOne()
{
    QString item = listWidget->takeItem(listWidget->currentRow())->text();
    
    QStringList exList = ReKonfig::walletBlackList();
    exList.removeOne(item);
    ReKonfig::setWalletBlackList(exList);
}


void PassExWidget::removeAll()
{
    listWidget->clear();
    
    QStringList clearList;
    ReKonfig::setWalletBlackList(clearList);
}
