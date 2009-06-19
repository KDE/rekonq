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


#ifndef HISTORYMENU_H
#define HISTORYMENU_H

#include "history.h"

#include <QtCore/QList>
#include <QtGui/QAction>

class ModelMenu;
class QWidget;
class QModelIndex;
class KUrl;

/**
 * Menu that is dynamically populated from the history
 *
 */

class HistoryMenu : public ModelMenu
{
    Q_OBJECT

signals:
    void openUrl(const KUrl &url);

public:
    HistoryMenu(QWidget *parent = 0);
    void setInitialActions(QList<QAction*> actions);

protected:
    bool prePopulated();
    void postPopulated();

private slots:
    void activated(const QModelIndex &index);

private:
    HistoryManager *m_history;
    HistoryMenuModel *m_historyMenuModel;
    QList<QAction*> m_initialActions;
};

#endif
