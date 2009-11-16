/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2009 by Nils Weigel <nehlsen at gmail dot com>
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


// rekonq includes
#include "bookmarkspanel.h"
#include "bookmarkstreemodel.h"

// Auto Includes
#include "rekonq.h"

// Qt includes
#include <QHBoxLayout>
#include <QLabel>
#include <QTreeView>
#include <QSortFilterProxyModel>
#include <QHeaderView>

// KDE includes
#include <KLineEdit>
#include <KLocalizedString>

BookmarksPanel::BookmarksPanel(const QString &title, QWidget *parent, Qt::WindowFlags flags):
	QDockWidget(title, parent, flags)
{
	setup();

    setShown(ReKonfig::showBookmarksPanel());
}


BookmarksPanel::~BookmarksPanel()
{
    ReKonfig::setShowBookmarksPanel(!isHidden());

	delete ui;
}

void BookmarksPanel::bookmarkActivated( const QModelIndex &index )
{
	if( index.isValid() )
		emit openUrl( qVariantValue< KUrl >( index.data( Qt::UserRole ) ) );
}

void BookmarksPanel::setup()
{
	setObjectName("bookmarksPanel");
    setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

	ui = new QWidget(this);

    // setup search bar
    QHBoxLayout *searchLayout = new QHBoxLayout;
    searchLayout->setContentsMargins(5, 0, 0, 0);
    QLabel *searchLabel = new QLabel(i18n("Search:"));
    searchLayout->addWidget(searchLabel);
    KLineEdit *search = new KLineEdit;
    search->setClearButtonShown(true);
    searchLayout->addWidget(search);

	// setup tree view
	QTreeView *treeView = new QTreeView(ui);
    treeView->setUniformRowHeights(true);
    treeView->setSelectionBehavior(QAbstractItemView::SelectRows);
    treeView->setTextElideMode(Qt::ElideMiddle);
    treeView->setAlternatingRowColors(true);
	treeView->header()->hide();
	treeView->setRootIsDecorated( false );

    // put everything together
    QVBoxLayout *vBoxLayout = new QVBoxLayout;
    vBoxLayout->setContentsMargins(0, 0, 0, 0);
    vBoxLayout->addLayout(searchLayout);
    vBoxLayout->addWidget(treeView);

	// add it to the UI
    ui->setLayout(vBoxLayout);
	setWidget(ui);

	BookmarksTreeModel *model = new BookmarksTreeModel( this );
	treeView->setModel( model );

	connect( treeView, SIGNAL( activated(QModelIndex) ), this, SLOT( bookmarkActivated(QModelIndex) ) );

// 	QSortFilterProxyModel *proxy = new QSortFilterProxyModel(ui);
    //-
//     HistoryManager *historyManager = Application::historyManager();
//     QAbstractItemModel *model = historyManager->historyTreeModel();
//
//     m_treeProxyModel->setSourceModel(model);
//     m_treeView->setModel(m_treeProxyModel);
//     m_treeView->setExpanded(m_treeProxyModel->index(0, 0), true);
//     m_treeView->header()->hideSection(1);
//     QFontMetrics fm(font());
//     int header = fm.width(QLatin1Char('m')) * 40;
//     m_treeView->header()->resizeSection(0, header);
//
//     connect(search, SIGNAL(textChanged(QString)), m_treeProxy, SLOT(setFilterFixedString(QString)));
//     connect(m_treeView, SIGNAL(activated(const QModelIndex&)), this, SLOT(open()));
}
