/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2008 Benjamin C. Meyer <ben@meyerhome.net>
* Copyright (C) 2008-2011 by Andrea Diamantini <adjam7 at gmail dot com>
* Copyright (C) 2009 by Paweł Prażak <pawelprazak at gmail dot com>
* Copyright (C) 2009-2011 by Lionel Chauvin <megabigbug@yahoo.fr>
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


// Rekonq Includes
#include "rekonq_defines.h"

// KDE Includes
#include <KTabBar>

//Qt Includes
#include <QSignalMapper>

// Forward Declarations
class KPassivePopup;
class TabHighlightEffect;
class QPropertyAnimation;


/**
 * Tab bar with a few more features such as
 * a context menu and shortcuts
 *
 */
class REKONQ_TESTS_EXPORT TabBar : public KTabBar
{
    Q_OBJECT

public:
    explicit TabBar(QWidget *parent);

    void setTabHighlighted(int index);
    void resetTabHighlighted(int index);
    QRect tabTextRect(int index);
    void setAnimatedTabHighlighting(bool enabled);

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

    virtual void hideEvent(QHideEvent *event);
    virtual void showEvent(QShowEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void leaveEvent(QEvent *event);
    virtual void mousePressEvent(QMouseEvent *event);

    virtual void tabRemoved(int index);

private slots:
    void cloneTab();
    void closeTab();
    void closeOtherTabs();
    void reloadTab();
    void detachTab();

    void contextMenu(int, const QPoint &);
    void emptyAreaContextMenu(const QPoint &);
    void tabMoved(int, int);

    void showTabPreview();

    void removeAnimation(int index);

private:
    //constants
    static const int baseWidthDivisor = 4;
    static const int minWidthDivisor = 8;

    void setupHistoryActions();
    friend class MainView;

    /**
     * the index in which we are seeing a Context menu
     */
    int m_actualIndex;

    QWeakPointer<KPassivePopup> m_previewPopup;

    /**
     * the index of the tab preview shown
     */
    int m_currentTabPreviewIndex;
    bool m_isFirstTimeOnTab;

    //highlightAnimation
    TabHighlightEffect *m_tabHighlightEffect;
    QHash<QByteArray, QPropertyAnimation*> m_highlightAnimation;
    QSignalMapper *m_animationMapper;
};

#endif
