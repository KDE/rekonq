/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2007-2008 Trolltech ASA. All rights reserved
* Copyright (C) 2008-2009 by Andrea Diamantini <adjam7 at gmail dot com>
*
*
* This program is free software; you can redistribute it
* and/or modify it under the terms of the GNU General
* Public License as published by the Free Software Foundation;
* either version 3, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* ============================================================ */


// Auto Includes
#include "historymenu.h"
#include "historymenu.moc"

// Local Includes
#include "application.h"
#include "mainwindow.h"

// Qt Includes
#include <QtGui/QWidget>
#include <QtCore/QModelIndex>

// KDE Includes
#include <KUrl>
#include <KMessageBox>


HistoryMenu::HistoryMenu(QWidget *parent)
        : ModelMenu(parent)
        , m_history(0)
{
    connect(this, SIGNAL(activated(const QModelIndex &)), this, SLOT(activated(const QModelIndex &)));
    setHoverRole(HistoryModel::UrlStringRole);
}


void HistoryMenu::activated(const QModelIndex &index)
{
    emit openUrl(index.data(HistoryModel::UrlRole).toUrl());
}


bool HistoryMenu::prePopulated()
{
    if (!m_history)
    {
        m_history = Application::historyManager();
        m_historyMenuModel = new HistoryMenuModel(m_history->historyTreeModel(), this);
        setModel(m_historyMenuModel);
    }
    // initial actions
    for (int i = 0; i < m_initialActions.count(); ++i)
        addAction(m_initialActions.at(i));
    if (!m_initialActions.isEmpty())
        addSeparator();
    setFirstSeparator(m_historyMenuModel->bumpedRows());

    return false;
}


void HistoryMenu::postPopulated()
{
    if (m_history->history().count() > 0)
        addSeparator();

    QAction *showAllAction = Application::instance()->mainWindow()->actionByName("show_history_panel");
    addAction(showAllAction);

    KAction *clearAction = new KAction(i18n("Clear History"), this);
    connect(clearAction, SIGNAL(triggered()), this, SLOT(clearHistory()));
    addAction(clearAction);
}


void HistoryMenu::clearHistory()
{
    int res = KMessageBox::warningYesNo(this, i18n("Are you sure you want to clear the history?"), i18n("Clear History") );

    if (res == KMessageBox::Yes)
    {
        m_history->clear();
    }
}


void HistoryMenu::setInitialActions(QList<QAction*> actions)
{
    m_initialActions = actions;
    for (int i = 0; i < m_initialActions.count(); ++i)
        addAction(m_initialActions.at(i));
}
