/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2009 by Andrea Diamantini <adjam7 at gmail dot com>*
* Copyright (C) 2009 by Domrachev Alexandr <alexandr.domrachev@gmail.com>
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
#include "historypanel.h"
#include "historypanel.moc"

// Qt Includes
#include <QtGui/QLabel>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>

// KDE Includes
#include <KLineEdit>
#include <KLocalizedString>


HistoryPanel::HistoryPanel(QWidget *parent)
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
    QLabel *searchLabel = new QLabel(i18n("Search:"));
    hBoxLayout->addWidget(searchLabel);
    KLineEdit *search = new KLineEdit;
    search->setClearButtonShown(true);
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


HistoryPanel::~HistoryPanel()
{
    delete m_treeProxyModel;
    delete m_historyTreeView;
}


void HistoryPanel::open()
{
    QModelIndex index = m_historyTreeView->currentIndex();
    if (!index.parent().isValid())
        return;
    emit openUrl(index.data(HistoryModel::UrlRole).toUrl());
}

