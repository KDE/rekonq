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

#ifndef PANELHISTORY_H
#define PANELHISTORY_H

// Qt Includes
#include <QWidget>

// Local Includes
#include "application.h"

class QTreeView;
class KUrl;
class TreeProxyModel;


class PanelHistory : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(PanelHistory)

public:
    explicit PanelHistory(QWidget *parent = 0);
    virtual ~PanelHistory();

signals:
    void openUrl(const KUrl &);

private slots:
    void open();

private:
    QTreeView *m_historyTreeView;
    TreeProxyModel *m_treeProxyModel;
    
};

#endif // PANELHISTORY_H
