/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2013-2014 by Andrea Diamantini <adjam7 at gmail dot com>
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
#include "sessiondialog.h"

// Auto Includes
#include "rekonq.h"

// Local Includes
#include "sessionmanager.h"
#include "application.h"
#include "rekonqwindow.h"

// KDE Includes
#include <KLocalizedString>

// Qt Includes
#include <QIcon>
#include <QListWidgetItem>


SessionDialog::SessionDialog(QWidget *parent)
    : QDialog(parent)
{
    // the title
    setWindowTitle(i18nc("@title:window", "Manage Session"));
    
    // the button box
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Close);
    
    buttonBox->button(QDialogButtonBox::Ok)->setIcon(QIcon::fromTheme(QL1S("system-run")));
    buttonBox->button(QDialogButtonBox::Ok)->setText(i18n("Load"));

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(loadSession()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    // the session widget
    QWidget *widget = new QWidget(this);
    sessionWidget.setupUi(widget);
    
    // insert everything inside the dialog...
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(widget);
    mainLayout->addWidget(buttonBox);
    setLayout(mainLayout);
    
    // 
    QStringList ses = ReKonfig::savedSessions();

    Q_FOREACH(const QString & s, ses)
    {
        QListWidgetItem *item = new QListWidgetItem(s, sessionWidget.listWidget, 0);
        item->setFlags (item->flags () | Qt::ItemIsEditable);
        sessionWidget.listWidget->addItem(item);
    }
    
    sessionWidget.saveButton->setIcon(QIcon::fromTheme( QL1S("document-save") ));
    connect(sessionWidget.saveButton, SIGNAL(clicked()), this, SLOT(saveSession()));
        
    sessionWidget.deleteButton->setIcon(QIcon::fromTheme( QL1S("edit-delete") ));
    connect(sessionWidget.deleteButton, SIGNAL(clicked()), this, SLOT(deleteSession()));
    
    connect(sessionWidget.listWidget, SIGNAL(currentRowChanged(int)), this, SLOT(updateButtons(int)));
    connect(sessionWidget.listWidget, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(save()));
    
    updateButtons(-1);
    
    if (rApp->rekonqWindowList().isEmpty())
        sessionWidget.saveButton->setEnabled(false);
}


void SessionDialog::loadSession()
{
    // I'm not sure this is truly the right way to proceed;
    // But it works as expected. We'll see...
    accept();
    qDebug() << "Ok, loading your session...";
    
    int cc = sessionWidget.listWidget->currentRow();
    if (cc < 0)
    {
        rApp->loadUrl(QUrl( QL1S("rekonq:home") ));
        return;
    }
    
    SessionManager::self()->restoreYourSession(cc);
    
}


void SessionDialog::saveSession()
{
    int cc = sessionWidget.listWidget->count();
    SessionManager::self()->saveYourSession(cc);
    
    QListWidgetItem *item = new QListWidgetItem(i18n("untitled"), sessionWidget.listWidget, 0);
    item->setFlags (item->flags () | Qt::ItemIsEditable);
    sessionWidget.listWidget->addItem(item);
}


void SessionDialog::deleteSession()
{
    sessionWidget.listWidget->takeItem(sessionWidget.listWidget->currentRow());
    save();
}


void SessionDialog::updateButtons(int index)
{
    qDebug() << "UPDATE INDEX: " << index;
    if (index < 0)
    {
        sessionWidget.deleteButton->setEnabled(false);
        return;
    }
    
    sessionWidget.deleteButton->setEnabled(true);
}


void SessionDialog::save()
{
    qDebug() << " ------------------------ SAVE --------------------------";
    
    QStringList ses;
    
    int c = sessionWidget.listWidget->count();
    for (int i = 0; i < c; ++i)
    {        
        QListWidgetItem *item = sessionWidget.listWidget->item(i);
        ses << item->text();
    }

    ReKonfig::setSavedSessions(ses);
}
