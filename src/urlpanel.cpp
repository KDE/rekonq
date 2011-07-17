/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2009 by Domrachev Alexandr <alexandr.domrachev@gmail.com>
* Copyright (C) 2009-2011 by Andrea Diamantini <adjam7 at gmail dot com>
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
#include "urlpanel.h"
#include "urlpanel.moc"

// Local Includes
#include "paneltreeview.h"
#include "urlfilterproxymodel.h"

// KDE Includes
#include <KLineEdit>
#include <KLocalizedString>

// Qt Includes
#include <QtGui/QLabel>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>


UrlPanel::UrlPanel(const QString &title, QWidget *parent, Qt::WindowFlags flags)
    : QDockWidget(title, parent, flags)
    , _treeView(new PanelTreeView(this))
    , _loaded(false)
{
    setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

    connect(this, SIGNAL(visibilityChanged(bool)), this, SLOT(showing(bool)));
}


void UrlPanel::showing(bool b)
{
    if(!_loaded && b)
    {
        setup();
        _loaded = true;
    }
}


void UrlPanel::setup()
{
    kDebug() << "Loading panel setup...";

    QWidget *ui = new QWidget(this);

    // setup search bar
    QHBoxLayout *searchLayout = new QHBoxLayout;
    searchLayout->setContentsMargins(5, 0, 0, 0);
    QLabel *searchLabel = new QLabel(i18n("&Search:"));
    searchLayout->addWidget(searchLabel);
    KLineEdit *search = new KLineEdit;
    search->setClearButtonShown(true);
    searchLayout->addWidget(search);
    searchLabel->setBuddy(search);

    // setup tree view
    _treeView->setUniformRowHeights(true);
    _treeView->header()->hide();

    // put everything together
    QVBoxLayout *vBoxLayout = new QVBoxLayout;
    vBoxLayout->setContentsMargins(0, 0, 0, 0);
    vBoxLayout->addLayout(searchLayout);
    vBoxLayout->addWidget(_treeView);

    // add it to the UI
    ui->setLayout(vBoxLayout);
    setWidget(ui);

    UrlFilterProxyModel *proxy = new UrlFilterProxyModel(this);
    proxy->setSourceModel(model());
    _treeView->setModel(proxy);

    connect(search, SIGNAL(textChanged(QString)), proxy, SLOT(setFilterFixedString(QString)));
    connect(search, SIGNAL(textChanged(QString)), this, SLOT(expandTreeView()));

    connect(_treeView, SIGNAL(contextMenuItemRequested(const QPoint &)), this, SLOT(contextMenuItem(const QPoint &)));
    connect(_treeView, SIGNAL(contextMenuGroupRequested(const QPoint &)), this, SLOT(contextMenuGroup(const QPoint &)));
    connect(_treeView, SIGNAL(contextMenuEmptyRequested(const QPoint &)), this, SLOT(contextMenuEmpty(const QPoint &)));
}

void UrlPanel::expandTreeView()
{
    _treeView->expandAll();
}
