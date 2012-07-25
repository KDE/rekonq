/***************************************************************************
 *   Copyright (C) 2012 by Andrea Diamantini <adjam7@gmail.com>                            *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/


#ifndef TAB_BAR
#define TAB_BAR


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
    TabBar(QWidget *parent);

    void setTabHighlighted(int index, bool b);
    QRect tabTextRect(int index);

    static const int genericTabNumber = 6;

protected:
    virtual QSize tabSizeHint(int index) const;

    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void leaveEvent(QEvent *event);
    virtual void mousePressEvent(QMouseEvent *event);

    virtual void tabRemoved(int index);

Q_SIGNALS:
    void cloneTab(int);
    void closeTab(int);
    void closeOtherTabs(int);
    void reloadTab(int);
    void detachTab(int);
    void restoreClosedTab(int);

private Q_SLOTS:
    void cloneTab();
    void closeTab();
    void closeOtherTabs();
    void reloadTab();
    void detachTab();
    void reopenLastClosedTab();

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
};

#endif // TAB_BAR
