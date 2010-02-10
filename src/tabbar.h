/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2008 Benjamin C. Meyer <ben@meyerhome.net>
* Copyright (C) 2008-2010 by Andrea Diamantini <adjam7 at gmail dot com>
* Copyright (C) 2009 by Paweł Prażak <pawelprazak at gmail dot com>
* Copyright (C) 2009-2010 by Lionel Chauvin <megabigbug@yahoo.fr>
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


#ifndef TABBAR_H
#define TABBAR_H


// Local Includes
#include "rekonqprivate_export.h"

// Qt Includes
#include <QWeakPointer>

// KDE Includes
#include <KTabBar>

// Forward Declarations
class QPoint;
class QMouseEvent;
class QEvent;

class KPassivePopup;


/**
 * Tab bar with a few more features such as
 * a context menu and shortcuts
 *
 */
class REKONQ_TESTS_EXPORT TabBar : public KTabBar
{
    Q_OBJECT

public:
    TabBar(QWidget *parent);
    ~TabBar();

    void showTabPreview(int tab);

signals:
    void cloneTab(int index);
    void closeTab(int index);
    void closeOtherTabs(int index);
    void reloadTab(int index);
    void reloadAllTabs();
    void detachTab(int index);

protected:
    /**
     * Added to fix tab dimension
     */
    virtual QSize tabSizeHint(int index) const;
    
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void leaveEvent(QEvent *event);
    virtual void mousePressEvent(QMouseEvent *event);
    
private slots:
    void cloneTab();
    void closeTab();
    void closeOtherTabs();
    void reloadTab();
    void detachTab();
    
    void contextMenu(int, const QPoint &);
    void emptyAreaContextMenu(const QPoint &);

private:
    friend class MainView;
    
    /**
     * the index in which we are seeing a Context menu
     */
    int m_actualIndex;

    QWeakPointer<KPassivePopup> m_previewPopup;
    int m_currentTabPreview;
};

#endif
