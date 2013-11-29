/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2008-2012 by Andrea Diamantini <adjam7 at gmail dot com>
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


#ifndef TAB_BAR
#define TAB_BAR


// Rekonq Includes
#include "rekonq_defines.h"

// KDE Includes
#include <KTabBar>

#include <QPropertyAnimation>

// Forward Declarations
class TabPreviewPopup;
class TabHighlightEffect;

class QSignalMapper;


class TabBar : public KTabBar
{
    Q_OBJECT

public:
    explicit TabBar(QWidget *parent);

    void setTabHighlighted(int index, bool b);
    QRect tabTextRect(int index);

protected:
    virtual QSize tabSizeHint(int index) const;

    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void leaveEvent(QEvent *event);
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);

    virtual void tabInserted(int index);
    virtual void tabRemoved(int index);

    virtual void tabLayoutChange();

Q_SIGNALS:
    void cloneTab(int);
    void closeTab(int);
    void closeOtherTabs(int);
    void reloadTab(int);
    void detachTab(int);
    void tabLayoutChanged();

private Q_SLOTS:
    void cloneTab();
    void closeTab();
    void closeOtherTabs();
    void reloadTab();
    void detachTab();

    void pinTab();
    void unpinTab();

    void contextMenu(int, const QPoint &);
    void emptyAreaContextMenu(const QPoint &);

    void removeAnimation(int index);

    void showTabPreview();
    void hideTabPreview();

private:
    // highlightAnimation
    TabHighlightEffect *m_tabHighlightEffect;
    QHash<QByteArray, QPropertyAnimation*> m_highlightAnimation;
    QSignalMapper *m_animationMapper;

    // tab preview
    QWeakPointer<TabPreviewPopup> m_previewPopup;
    int m_currentTabPreviewIndex;
    bool m_isFirstTimeOnTab;

    static const int c_baseTabWidth = 250;
    static const int c_minTabWidth =  50;
};

#endif // TAB_BAR
