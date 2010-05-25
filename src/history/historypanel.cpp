/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2009 by Domrachev Alexandr <alexandr.domrachev@gmail.com>
* Copyright (C) 2009-2010 by Andrea Diamantini <adjam7 at gmail dot com>
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
#include <KMenu>
#include <KAction>
#include <KMessageBox>


HistoryPanel::HistoryPanel(const QString &title, QWidget *parent, Qt::WindowFlags flags)
        : QDockWidget(title, parent, flags)
        , m_treeView(new PanelTreeView(this))
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

    m_treeView->setUniformRowHeights(true);
    m_treeView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_treeView->setTextElideMode(Qt::ElideMiddle);
    m_treeView->setAlternatingRowColors(true);
    m_treeView->header()->hide();

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
    vBoxLayout->addWidget(m_treeView);

    // add it to the UI
    ui->setLayout(vBoxLayout);
    setWidget(ui);

    //-
    HistoryManager *historyManager = Application::historyManager();
    QAbstractItemModel *model = historyManager->historyTreeModel();

    TreeProxyModel *treeProxyModel = new TreeProxyModel(this);
    treeProxyModel->setSourceModel(model);
    m_treeView->setModel(treeProxyModel);
    m_treeView->setExpanded(treeProxyModel->index(0, 0), true);
    m_treeView->header()->hideSection(1);
    QFontMetrics fm(font());
    int header = fm.width( QL1C('m') ) * 40;
    m_treeView->header()->resizeSection(0, header);

    connect(search, SIGNAL(textChanged(QString)), treeProxyModel, SLOT(setFilterFixedString(QString)));
    connect(m_treeView, SIGNAL(contextMenuItemRequested(const QPoint &)), this, SLOT(contextMenuItem(const QPoint &)));
    connect(m_treeView, SIGNAL(contextMenuGroupRequested(const QPoint &)), this, SLOT(contextMenuGroup(const QPoint &)));
}


void HistoryPanel::contextMenuItem(const QPoint &pos)
{
    KMenu menu;
    KAction* action;

    action = new KAction(KIcon("tab-new"), i18n("Open"), this);
    connect(action, SIGNAL(triggered()), m_treeView, SLOT(openInCurrentTab()));
    menu.addAction(action);

    action = new KAction(KIcon("tab-new"), i18n("Open in New Tab"), this);
    connect(action, SIGNAL(triggered()), m_treeView, SLOT(openInNewTab()));
    menu.addAction(action);

    action = new KAction(KIcon("window-new"), i18n("Open in New Window"), this);
    connect(action, SIGNAL(triggered()), m_treeView, SLOT(openInNewWindow()));
    menu.addAction(action);

    action = new KAction(KIcon("edit-copy"), i18n("Copy Link Address"), this);
    connect(action, SIGNAL(triggered()), m_treeView, SLOT(copyToClipboard()));
    menu.addAction(action);

    menu.exec(m_treeView->mapToGlobal(pos));
}


void HistoryPanel::contextMenuGroup(const QPoint &pos)
{
    KMenu menu;
    KAction* action;

    action = new KAction(KIcon("tab-new"), i18n("Open Folder in Tabs"), this);
    connect(action, SIGNAL(triggered()), this, SLOT(openAll()));
    menu.addAction(action);

    menu.exec(m_treeView->mapToGlobal(pos));
}


void HistoryPanel::openAll()
{
    QModelIndex index = m_treeView->currentIndex();
    if (!index.isValid())
        return;

    QList<KUrl> allChild;

    for (int i = 0; i < index.model()->rowCount(index); i++)
        allChild << qVariantValue<KUrl>(index.child(i, 0).data(Qt::UserRole));

    if (allChild.length() > 8)
    {
        if (!(KMessageBox::warningContinueCancel(this,
                i18n("You are about to open %1 tabs.\nAre you sure?",
                     QString::number(allChild.length()))) == KMessageBox::Continue)
           )
            return;
    }

    for (int i = 0; i < allChild.length(); i++)
        emit openUrl(allChild.at(i).url(), Rekonq::SettingOpenTab);
}

