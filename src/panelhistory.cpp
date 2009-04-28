/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2009 by Domrachev Alexandr <alexandr.domrachev@gmail.com>
*
*
* This program is free software; you can redistribute it
* and/or modify it under the terms of the GNU General
* Public License as published by the Free Software Foundation;
* either version 2, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* ============================================================ */


// Self Includes
#include "panelhistory.h"
#include "panelhistory.moc"

// QT Includes
#include <QLabel>

// KDE Includes
#include <KLocalizedString>
#include <KLineEdit>
#include <KUrl>

// Local Includes
#include "history.h"


PanelHistory::PanelHistory(QWidget *parent)
    : QWidget(parent)
    , m_historyTreeView(new QTreeView)
    , m_treeProxyModel(new TreeProxyModel(this))
{
    m_historyTreeView->setUniformRowHeights(true);
    m_historyTreeView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_historyTreeView->setTextElideMode(Qt::ElideMiddle);
    m_historyTreeView->setAlternatingRowColors(true);

    // add search bar
    QHBoxLayout *hBoxLayout = new QHBoxLayout;
    hBoxLayout->setContentsMargins(5, 0, 0, 0);
    QLabel *searchLabel = new QLabel(i18n("Search: "));
    hBoxLayout->addWidget(searchLabel);
    KLineEdit *search = new KLineEdit;
    hBoxLayout->addWidget(search);
    QWidget *searchBar = new QWidget;
    searchBar->setLayout(hBoxLayout);

    // setup view
    QVBoxLayout *vBoxLayout = new QVBoxLayout;
    vBoxLayout->setContentsMargins(0, 0, 0, 0);
    vBoxLayout->addWidget(searchBar);
    vBoxLayout->addWidget(m_historyTreeView);
    setLayout(vBoxLayout);

    //-
    HistoryManager *historyManager = Application::historyManager();
    QAbstractItemModel *model = historyManager->historyTreeModel();
    
    m_treeProxyModel->setSourceModel(model);
    m_historyTreeView->setModel(m_treeProxyModel);
    m_historyTreeView->setExpanded(m_treeProxyModel->index(0, 0), true);
    m_historyTreeView->header()->hideSection(1);
    QFontMetrics fm(font());
    int header = fm.width(QLatin1Char('m')) * 40;
    m_historyTreeView->header()->resizeSection(0, header);
    
    connect(search, SIGNAL(textChanged(QString)), m_treeProxyModel, SLOT(setFilterFixedString(QString)));
    connect(m_historyTreeView, SIGNAL(activated(const QModelIndex&)), this, SLOT(open()));
}


PanelHistory::~PanelHistory()
{
    delete m_treeProxyModel;
    delete m_historyTreeView;
}


void PanelHistory::open()
{
    QModelIndex index = m_historyTreeView->currentIndex();
    if (!index.parent().isValid())
        return;
    emit openUrl(index.data(HistoryModel::UrlRole).toUrl());
}

