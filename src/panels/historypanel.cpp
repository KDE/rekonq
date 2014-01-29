/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2009 by Domrachev Alexandr <alexandr.domrachev@gmail.com>
* Copyright (C) 2009-2014 by Andrea Diamantini <adjam7 at gmail dot com>
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

// Auto Includes
#include "rekonq.h"

// Local Includes
#include "paneltreeview.h"

#include "historymanager.h"
#include "historymodels.h"
#include "urlfilterproxymodel.h"

// KDE Includes
#include <KLocalizedString>
#include <KMenu>
#include <KAction>
#include <KMessageBox>

// Qt Includes
#include <QHeaderView>


HistoryPanel::HistoryPanel(const QString &title, QWidget *parent, Qt::WindowFlags flags)
    : UrlPanel(title, parent, flags)
{
    setObjectName("historyPanel");
    setVisible(ReKonfig::showHistoryPanel());
}


HistoryPanel::~HistoryPanel()
{
    ReKonfig::setShowHistoryPanel(!isHidden());
}


void HistoryPanel::contextMenuItem(const QPoint &pos)
{
    KMenu menu;
    KAction* action;

    action = new KAction(QIcon::fromTheme("tab-new"), i18n("Open"), this);
    connect(action, SIGNAL(triggered()), panelTreeView(), SLOT(openInCurrentTab()));
    menu.addAction(action);

    action = new KAction(QIcon::fromTheme("tab-new"), i18n("Open in New Tab"), this);
    connect(action, SIGNAL(triggered()), panelTreeView(), SLOT(openInNewTab()));
    menu.addAction(action);

    action = new KAction(QIcon::fromTheme("window-new"), i18n("Open in New Window"), this);
    connect(action, SIGNAL(triggered()), panelTreeView(), SLOT(openInNewWindow()));
    menu.addAction(action);

    action = new KAction(QIcon::fromTheme("edit-copy"), i18n("Copy Link Address"), this);
    connect(action, SIGNAL(triggered()), panelTreeView(), SLOT(copyToClipboard()));
    menu.addAction(action);

    action = new KAction(QIcon::fromTheme("edit-clear"), i18n("Remove Entry"), this);
    connect(action, SIGNAL(triggered()), this, SLOT(deleteEntry()));
    menu.addAction(action);

    action = new KAction(QIcon::fromTheme("edit-clear"), i18n("Remove all occurrences"), this);
    connect(action, SIGNAL(triggered()), this, SLOT(forgetSite()));
    menu.addAction(action);

    menu.exec(panelTreeView()->mapToGlobal(pos));
}


void HistoryPanel::contextMenuGroup(const QPoint &pos)
{
    KMenu menu;
    KAction* action;

    action = new KAction(QIcon::fromTheme("tab-new"), i18n("Open Folder in Tabs"), this);
    connect(action, SIGNAL(triggered()), this, SLOT(openAll()));
    menu.addAction(action);

    action = new KAction(QIcon::fromTheme("edit-clear"), i18n("Remove Folder"), this);
    connect(action, SIGNAL(triggered()), this, SLOT(deleteGroup()));
    menu.addAction(action);

    menu.exec(panelTreeView()->mapToGlobal(pos));
}


void HistoryPanel::contextMenuEmpty(const QPoint& /*pos*/)
{
}


void HistoryPanel::openAll()
{
    QModelIndex index = panelTreeView()->currentIndex();
    if (!index.isValid())
        return;

    QList<QUrl> allChild;

    for (int i = 0; i < index.model()->rowCount(index); i++)
        allChild << qVariantValue<QUrl>(index.child(i, 0).data(Qt::UserRole));

    if (allChild.length() > 8)
    {
        if (!(KMessageBox::warningContinueCancel(this,
                i18ncp("%1=Number of tabs. Value is always >=8",
                       "You are about to open %1 tabs.\nAre you sure?",
                       "You are about to open %1 tabs.\nAre you sure?",
                       allChild.length())) == KMessageBox::Continue)
           )
            return;
    }

    for (int i = 0; i < allChild.length(); i++)
        emit openUrl(allChild.at(i).url(), Rekonq::NewTab);
}


void HistoryPanel::deleteGroup()
{
    QModelIndex index = panelTreeView()->currentIndex();
    if (!index.isValid())
        return;

    //Getting all URLs of sub items.
    QList<QUrl> allChild;
    for (int i = 0; i < index.model()->rowCount(index); i++)
        allChild << qVariantValue<QUrl>(index.child(i, 0).data(Qt::UserRole));

    for (int i = 0; i < allChild.length(); i++)
        HistoryManager::self()->removeHistoryEntry(allChild.at(i));

}


void HistoryPanel::setup()
{
    UrlPanel::setup();

    panelTreeView()->header()->hideSection(1);

    const UrlFilterProxyModel *proxy = static_cast<const UrlFilterProxyModel*>(panelTreeView()->model());
    panelTreeView()->expand(proxy->index(0, 0));
}


void HistoryPanel::deleteEntry()
{
    QModelIndex index = panelTreeView()->currentIndex();
    if (!index.isValid())
        return;
    removedFolderIndex = index.parent().row();

    HistoryManager::self()->removeHistoryEntry(qVariantValue< QUrl >(index.data(Qt::UserRole)));

    QModelIndex expandItem = panelTreeView()->model()->index(removedFolderIndex, 0);
    if (expandItem.isValid())
        panelTreeView()->expand(expandItem);
}


void HistoryPanel::forgetSite()
{
    QModelIndex index = panelTreeView()->currentIndex();
    if (!index.isValid())
        return;
    removedFolderIndex = index.row();

    QString site = qVariantValue< QUrl >(index.data(Qt::UserRole)).host();
    QList<HistoryItem> toRemove = HistoryManager::self()->find(site);
    for (int i = 0; i < toRemove.length(); i++)
    {
        HistoryManager::self()->removeHistoryEntry(QUrl(toRemove.at(i).url));
    }

    QModelIndex expandItem = panelTreeView()->model()->index(removedFolderIndex, 0);
    if (expandItem.isValid())
        panelTreeView()->expand(expandItem);
}


QAbstractItemModel* HistoryPanel::model()
{
    return HistoryManager::self()->historyTreeModel();
}
