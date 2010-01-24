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

// Auto Includes
#include "rekonq.h"

// Local Includes
#include "application.h"
#include "historymodels.h"

// Qt Includes
#include <QtGui/QLabel>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QTreeView>


// KDE Includes
#include <KLineEdit>
#include <KLocalizedString>


HistoryPanel::HistoryPanel(const QString &title, QWidget *parent, Qt::WindowFlags flags)
    : QDockWidget(title, parent, flags)
{
    setup();
    setShown(ReKonfig::showHistoryPanel());
}


HistoryPanel::~HistoryPanel()
{
    // Save side panel's state
    ReKonfig::setShowHistoryPanel(!isHidden());
}


void HistoryPanel::setup()
{
    setObjectName("historyPanel");
    setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    
    QWidget *ui = new QWidget(this);

    QTreeView *historyTreeView = new QTreeView(this);
    historyTreeView->setUniformRowHeights(true);
    historyTreeView->setSelectionBehavior(QAbstractItemView::SelectRows);
    historyTreeView->setTextElideMode(Qt::ElideMiddle);
    historyTreeView->setAlternatingRowColors(true);

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

    // setup layout
    QVBoxLayout *vBoxLayout = new QVBoxLayout;
    vBoxLayout->setContentsMargins(0, 0, 0, 0);
    vBoxLayout->addWidget(searchBar);
    vBoxLayout->addWidget(historyTreeView);

    // add it to the UI
    ui->setLayout(vBoxLayout);
    setWidget(ui);

    //-
    HistoryManager *historyManager = Application::historyManager();
    QAbstractItemModel *model = historyManager->historyTreeModel();

    TreeProxyModel *treeProxyModel = new TreeProxyModel(this);
    treeProxyModel->setSourceModel(model);
    historyTreeView->setModel(treeProxyModel);
    historyTreeView->setExpanded(treeProxyModel->index(0, 0), true);
    historyTreeView->header()->hideSection(1);
    QFontMetrics fm(font());
    int header = fm.width(QLatin1Char('m')) * 40;
    historyTreeView->header()->resizeSection(0, header);

    connect(search, SIGNAL(textChanged(QString)), treeProxyModel, SLOT(setFilterFixedString(QString)));
    connect(historyTreeView, SIGNAL(activated(const QModelIndex &)), this, SLOT(itemActivated(const QModelIndex &)));
}


void HistoryPanel::itemActivated(const QModelIndex &item)
{
    emit openUrl( item.data(HistoryModel::UrlRole).toUrl() );
}
