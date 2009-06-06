/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2009 by Andrea Diamantini <adjam7 at gmail dot com>
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
#include "historydialog.h"
#include "historydialog.moc"

// Local Includes
#include "history.h"
#include "application.h"

// KDE Includes
#include <KAction>
#include <KUrl>

// Qt Includes
#include <QtCore/QPoint>

#include <QtGui/QWidget>
#include <QtGui/QClipboard>
#include <QtGui/QDesktopWidget>


HistoryDialog::HistoryDialog(QWidget *parent, HistoryManager *setHistory) 
    : KDialog(parent)
    , m_historyWidg(new Ui::historyWidget)
{
    HistoryManager *history = setHistory;
    if (!history)
        history = Application::historyManager();

    setCaption( i18n("History") );
    setButtons( KDialog::Close );

    QWidget *widget = new QWidget;
    m_historyWidg->setupUi(widget);
    setMainWidget(widget);

    m_historyWidg->search->setClearButtonShown(true);

    m_historyWidg->tree->setUniformRowHeights(true);
    m_historyWidg->tree->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_historyWidg->tree->setTextElideMode(Qt::ElideMiddle);

    QAbstractItemModel *model = history->historyTreeModel();
    TreeProxyModel *proxyModel = new TreeProxyModel(this);

    connect(m_historyWidg->search, SIGNAL(textChanged(QString)), proxyModel, SLOT(setFilterFixedString(QString)));
    connect(m_historyWidg->removeButton, SIGNAL(clicked()), m_historyWidg->tree, SLOT(removeOne()));
    connect(m_historyWidg->removeAllButton, SIGNAL(clicked()), history, SLOT(clear()));

    proxyModel->setSourceModel(model);
    m_historyWidg->tree->setModel(proxyModel);
    m_historyWidg->tree->setExpanded(proxyModel->index(0, 0), true);
    m_historyWidg->tree->setAlternatingRowColors(true);

    QFontMetrics fm(font());
    int header = fm.width(QLatin1Char('m')) * 25;
    m_historyWidg->tree->header()->resizeSection(0, header);
    m_historyWidg->tree->header()->setStretchLastSection(true);

    m_historyWidg->tree->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(m_historyWidg->tree, SIGNAL(customContextMenuRequested(const QPoint &)), 
            this, SLOT(customContextMenuRequested(const QPoint &)));

    connect(m_historyWidg->tree, SIGNAL(activated(const QModelIndex&)), this, SLOT(open()));
}


void HistoryDialog::customContextMenuRequested(const QPoint &pos)
{
    KMenu menu;
    QModelIndex index = m_historyWidg->tree->indexAt(pos);
    index = index.sibling(index.row(), 0);
    if (index.isValid() && !m_historyWidg->tree->model()->hasChildren(index))
    {
        menu.addAction(i18n("Open"), this, SLOT(open()));
        menu.addSeparator();
        menu.addAction(i18n("Copy"), this, SLOT(copy()));
    }
    menu.addAction(i18n("Delete"), m_historyWidg->tree, SLOT(removeOne()));
    menu.exec(QCursor::pos());
}


void HistoryDialog::open()
{
    QModelIndex index = m_historyWidg->tree->currentIndex();
    if (!index.parent().isValid())
        return;
    emit openUrl(index.data(HistoryModel::UrlRole).toUrl());
}


void HistoryDialog::copy()
{
    QModelIndex index = m_historyWidg->tree->currentIndex();
    if (!index.parent().isValid())
        return;
    QString url = index.data(HistoryModel::UrlStringRole).toString();

    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(url);
}

QSize HistoryDialog::sizeHint() const
{
    QRect desktopRect = Application::desktop()->screenGeometry();
    QSize size = desktopRect.size() * 0.7;
    return size;
}
