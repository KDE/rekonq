/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2009 by Domrachev Alexandr <alexandr.domrachev@gmail.com>
* Copyright (C) 2009-2013 by Andrea Diamantini <adjam7 at gmail dot com>
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


#ifndef URLPANEL_H
#define URLPANEL_H


// Rekonq Includes
#include "rekonq_defines.h"

// Qt Includes
#include <QDockWidget>

// Forward Declarations
class PanelTreeView;

class QAbstractItemModel;


class REKONQ_TESTS_EXPORT UrlPanel : public QDockWidget
{
    Q_OBJECT

public:
    explicit UrlPanel(const QString &title, QWidget *parent = 0, Qt::WindowFlags flags = 0);

Q_SIGNALS:
    void openUrl(const KUrl &, const Rekonq::OpenType &);
    void itemHovered(const QString &);

public Q_SLOTS:
    void showing(bool);

protected:
    virtual void setup();
    virtual QAbstractItemModel* model() = 0;

    PanelTreeView* panelTreeView() const
    {
        return _treeView;
    }

protected Q_SLOTS:
    virtual void contextMenuItem(const QPoint &pos) = 0;
    virtual void contextMenuGroup(const QPoint &pos) = 0;
    virtual void contextMenuEmpty(const QPoint &pos) = 0;

private Q_SLOTS:
    void expandTreeView();

private:
    PanelTreeView *_treeView;
    bool _loaded;
};


#endif // URLPANEL_H
