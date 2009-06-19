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


#include "historymenu.h"
#include "historymenu.moc"

#include "application.h"

#include <QtGui/QWidget>
#include <QtCore/QModelIndex>

#include <KUrl>

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

    KAction *showAllAction = new KAction(i18n("Show All History"), this);
//     connect(showAllAction, SIGNAL(triggered()), this, SLOT(showHistoryDialog()));
    addAction(showAllAction);

    KAction *clearAction = new KAction(i18n("Clear History"), this);
    connect(clearAction, SIGNAL(triggered()), m_history, SLOT(clear()));
    addAction(clearAction);
}


void HistoryMenu::setInitialActions(QList<QAction*> actions)
{
    m_initialActions = actions;
    for (int i = 0; i < m_initialActions.count(); ++i)
        addAction(m_initialActions.at(i));
}
