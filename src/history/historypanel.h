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


#ifndef HISTORYPANEL_H
#define HISTORYPANEL_H


// Local Includes
#include "rekonqprivate_export.h"
#include "application.h"
#include "paneltreeview.h"

// Qt Includes
#include <QDockWidget>

// Forward Declarations
class KUrl;
class QWidget;
class QModelIndex;


class REKONQ_TESTS_EXPORT HistoryPanel : public QDockWidget
{
    Q_OBJECT

public:
    explicit HistoryPanel(const QString &title, QWidget *parent = 0, Qt::WindowFlags flags = 0);
    ~HistoryPanel();

signals:
    void openUrl(const KUrl &, const Rekonq::OpenType &);
    void itemHovered(const QString &);

private slots:
    void contextMenuItem(const QPoint &pos);
    void contextMenuGroup(const QPoint &pos);
    void openAll();

private:
    void setup();
    PanelTreeView *m_treeView;
};

#endif // HISTORYPANEL_H
