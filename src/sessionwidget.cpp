/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2013 by Andrea Diamantini <adjam7 at gmail dot com>
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
#include "sessionwidget.h"
#include "sessionwidget.moc"

// Auto Includes
#include "rekonq.h"

// Local Includes
#include "sessionmanager.h"
#include "application.h"
#include "rekonqwindow.h"

// KDE Includes
#include <KIcon>

// Qt Includes
#include <QListWidgetItem>


SessionWidget::SessionWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUi(this);

    QStringList ses = ReKonfig::savedSessions();

    Q_FOREACH(const QString & s, ses)
    {
        QListWidgetItem *item = new QListWidgetItem(s, listWidget, 0);
        item->setFlags (item->flags () | Qt::ItemIsEditable);
        listWidget->addItem(item);
    }
    
    saveButton->setIcon(KIcon("document-save"));
    connect(saveButton, SIGNAL(clicked()), this, SLOT(saveSession()));
        
    deleteButton->setIcon(KIcon("edit-delete"));
    connect(deleteButton, SIGNAL(clicked()), this, SLOT(deleteSession()));
    
    connect(listWidget, SIGNAL(currentRowChanged(int)), this, SLOT(updateButtons(int)));
    connect(listWidget, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(save()));
    
    updateButtons(-1);
    
    if (rApp->rekonqWindowList().isEmpty())
        saveButton->setEnabled(false);
}


void SessionWidget::loadSession()
{
    int cc = listWidget->currentRow();
    if (cc < 0)
    {
        rApp->loadUrl(KUrl("rekonq:home"));
        return;
    }
    SessionManager::self()->restoreYourSession(cc);
}


void SessionWidget::saveSession()
{
    int cc = listWidget->count();
    SessionManager::self()->saveYourSession(cc);
    
    QListWidgetItem *item = new QListWidgetItem(i18n("untitled"), listWidget, 0);
    item->setFlags (item->flags () | Qt::ItemIsEditable);
    listWidget->addItem(item);
}


void SessionWidget::deleteSession()
{
    listWidget->takeItem(listWidget->currentRow());
    save();
}


void SessionWidget::updateButtons(int index)
{
    kDebug() << "UPDATE INDEX: " << index;
    if (index < 0)
    {
        deleteButton->setEnabled(false);
        return;
    }
    
    deleteButton->setEnabled(true);
}


void SessionWidget::save()
{
    kDebug() << " ------------------------ SAVE --------------------------";
    
    QStringList ses;
    
    int c = listWidget->count();
    for (int i = 0; i < c; ++i)
    {        
        QListWidgetItem *item = listWidget->item(i);
        ses << item->text();
    }

    ReKonfig::setSavedSessions(ses);
}
