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


#include "tabbar.h"
#include "tabbar.moc"

#include "tabwindow.h"
#include "tabhighlighteffect.h"
#include "tabpreviewpopup.h"
#include "webwindow.h"

#include "iconmanager.h"
#include "sessionmanager.h"

#include <KAcceleratorManager>
#include <KAction>
#include <KColorScheme>
#include <KLocalizedString>
#include <KMenu>
#include <KUrl>

#include <QLabel>
#include <QPropertyAnimation>
#include <QSignalMapper>
#include <QStyleOptionFrameV3>
#include <QMouseEvent>
#include <QTimer>


static inline QByteArray highlightPropertyName(int index)
{
    return QByteArray("hAnim").append(QByteArray::number(index));
}


// ------------------------------------------------------------------------------------


TabBar::TabBar(QWidget *parent)
    : KTabBar(parent)
    , m_tabHighlightEffect(new TabHighlightEffect(this))
    , m_animationMapper(new QSignalMapper(this))
{
    setElideMode(Qt::ElideRight);

    setTabsClosable(true);
    setMovable(true);
    setAcceptDrops(true);

    // avoid ambiguos shortcuts. See BUG:275858
    KAcceleratorManager::setNoAccel(this);

    // context menu(s)
    setContextMenuPolicy(Qt::CustomContextMenu);

    connect(this, SIGNAL(contextMenu(int, QPoint)), this, SLOT(contextMenu(int, QPoint)));
    connect(this, SIGNAL(emptyAreaContextMenu(QPoint)), this, SLOT(emptyAreaContextMenu(QPoint)));

    // Highlight effect
    connect(m_animationMapper, SIGNAL(mapped(int)), this, SLOT(removeAnimation(int)));
    setGraphicsEffect(m_tabHighlightEffect);
    m_tabHighlightEffect->setEnabled(true);
}


QSize TabBar::tabSizeHint(int index) const
{
    QWidget* p = qobject_cast<QWidget *>(parent());

    int w;
    if (tabData(index).toBool())
    {
        w = 36;
    }
    else
    {
        int tabWidgetWidth = p->size().width();
        w = c_baseTabWidth;
        if (w * count() > tabWidgetWidth)
        {
            w = tabWidgetWidth / count();
            if (w < c_minTabWidth)
            {
                w = c_minTabWidth;
            }
        }
    }
    
    int h = size().height();

    // this because it may happen sometimes (eg: exiting fullscreen)
    // that tabbar height is set to ZERO. And this is NOT good...
    if (h == 0)
        h = 30;
    
    QSize ts = QSize(w, h);
    return ts;
}


void TabBar::cloneTab()
{
    KAction *a = qobject_cast<KAction *>(sender());
    if (a)
    {
        int index = a->data().toInt();
        emit cloneTab(index);
    }
}


void TabBar::closeTab()
{
    KAction *a = qobject_cast<KAction *>(sender());
    if (a)
    {
        int index = a->data().toInt();
        emit closeTab(index);
    }
}


void TabBar::closeOtherTabs()
{
    KAction *a = qobject_cast<KAction *>(sender());
    if (a)
    {
        int index = a->data().toInt();
        emit closeOtherTabs(index);
    }
}


void TabBar::reloadTab()
{
    KAction *a = qobject_cast<KAction *>(sender());
    if (a)
    {
        int index = a->data().toInt();
        emit reloadTab(index);
    }
}


void TabBar::detachTab()
{
    KAction *a = qobject_cast<KAction *>(sender());
    if (a)
    {
        int index = a->data().toInt();
        emit detachTab(index);
    }
}


void TabBar::reopenLastClosedTab()
{
    KAction *a = qobject_cast<KAction *>(sender());
    if (a)
    {
        int index = a->data().toInt();
        emit restoreClosedTab(index);
    }
}


void TabBar::contextMenu(int tab, const QPoint &pos)
{
    TabWindow *w = qobject_cast<TabWindow *>(parent());

    KAction *a;

    KMenu menu;

    a = new KAction(KIcon("tab-new"), i18n("New &Tab"), this);
    connect(a, SIGNAL(triggered(bool)), w, SLOT(newCleanTab()));
    menu.addAction(a);

    menu.addSeparator();    // ----------------------------------------------------------------

    a = new KAction(KIcon("tab-duplicate"), i18n("Clone"), this);
    a->setData(tab);
    connect(a, SIGNAL(triggered(bool)), this, SLOT(cloneTab()));
    menu.addAction(a);

    a = new KAction(KIcon("view-refresh"), i18n("Reload"), this);
    connect(a, SIGNAL(triggered(bool)), this, SLOT(reloadTab()));
    a->setData(tab);
    menu.addAction(a);

    if (count() > 1)
    {
        a = new KAction(KIcon("tab-detach"), i18n("Detach"), this);
        connect(a, SIGNAL(triggered(bool)), this, SLOT(detachTab()));
        a->setData(tab);
        menu.addAction(a);
    }

    if (tabData(tab).toBool())
    {
        a = new KAction(i18n("Unpin Tab"), this);
        connect(a, SIGNAL(triggered(bool)), this, SLOT(unpinTab()));
        a->setData(tab);
        menu.addAction(a);
    }
    else
    {
        a = new KAction(i18n("Pin Tab"), this);
        connect(a, SIGNAL(triggered(bool)), this, SLOT(pinTab()));
        a->setData(tab);
        menu.addAction(a);
    }
    menu.addSeparator();    // ----------------------------------------------------------------

    a = new KAction(KIcon("tab-close"), i18n("&Close"), this);
    a->setData(tab);
    connect(a, SIGNAL(triggered(bool)), this, SLOT(closeTab()));
    menu.addAction(a);

    if (count() > 1)
    {
        a = new KAction(KIcon("tab-close-other"), i18n("Close &Other Tabs"), this);
        connect(a, SIGNAL(triggered(bool)), this, SLOT(closeOtherTabs()));
        a->setData(tab);
        menu.addAction(a);
    }

    menu.addSeparator();


    a = new KAction(KIcon("tab-new"), i18n("Open Last Closed Tab"), this);
    a->setData(0);  // last closed tab has index 0!
    connect(a, SIGNAL(triggered(bool)), this, SLOT(reopenLastClosedTab()));
    menu.addAction(a);

    if (count() > 1)
    {
        a = new KAction(KIcon("bookmark-new"), i18n("Bookmarks all tabs"), this);
        menu.addAction(a);
    }
    else
    {
        a = new KAction(KIcon("bookmark-new"), i18n("Bookmark"), this);
        menu.addAction(a);
    }

    menu.exec(pos);
}


void TabBar::emptyAreaContextMenu(const QPoint &pos)
{
    TabWindow *w = qobject_cast<TabWindow *>(parent());

    KAction *a;

    KMenu menu;

    a = new KAction(KIcon("tab-new"), i18n("New &Tab"), this);
    connect(a, SIGNAL(triggered(bool)), w, SLOT(newCleanTab()));
    menu.addAction(a);

    a = new KAction(KIcon("tab-new"), i18n("Open Last Closed Tab"), this);
    a->setData(0);  // last closed tab has index 0!
    menu.addAction(a);

    if (count() > 1)
    {
        a = new KAction(KIcon("bookmark-new"), i18n("Bookmarks all tabs"), this);
        menu.addAction(a);
    }
    else
    {
        a = new KAction(KIcon("bookmark-new"), i18n("Bookmark"), this);
        menu.addAction(a);
    }

    menu.exec(pos);
}


void TabBar::setTabHighlighted(int index, bool b)
{
    if (!b)
    {
        removeAnimation(index);
        setTabTextColor(index, KColorScheme(QPalette::Active, KColorScheme::Window).foreground(KColorScheme::NormalText).color());
        return;
    }

    const QByteArray propertyName = highlightPropertyName(index);
    const QColor highlightColor = KColorScheme(QPalette::Active, KColorScheme::Window).foreground(KColorScheme::PositiveText).color();

    if (tabTextColor(index) != highlightColor)
    {
        m_tabHighlightEffect->setEnabled(true);
        m_tabHighlightEffect->setProperty(propertyName, qreal(0.9));
        QPropertyAnimation *anim = new QPropertyAnimation(m_tabHighlightEffect, propertyName);
        m_highlightAnimation.insert(propertyName, anim);

        //setup the animation
        anim->setStartValue(0.9);
        anim->setEndValue(0.0);
        anim->setDuration(500);
        anim->setLoopCount(2);
        anim->start(QAbstractAnimation::DeleteWhenStopped);

        m_animationMapper->setMapping(anim, index);
        connect(anim, SIGNAL(finished()), m_animationMapper, SLOT(map()));

        setTabTextColor(index, highlightColor);
    }
}


QRect TabBar::tabTextRect(int index)
{
    QStyleOptionTabV3 option;
    initStyleOption(&option, index);
    return style()->subElementRect(QStyle::SE_TabBarTabText, &option, this);
}


void TabBar::removeAnimation(int index)
{
    const QByteArray propertyName = highlightPropertyName(index);
    m_tabHighlightEffect->setProperty(propertyName, QVariant()); //destroy the property

    QPropertyAnimation *anim = m_highlightAnimation.take(propertyName);
    m_animationMapper->removeMappings(anim);
    delete anim;

    if (m_highlightAnimation.isEmpty())
        m_tabHighlightEffect->setEnabled(false);
}


void TabBar::tabRemoved(int index)
{
    hideTabPreview();
    removeAnimation(index);
}


void TabBar::mouseMoveEvent(QMouseEvent *event)
{
    KTabBar::mouseMoveEvent(event);

    if (count() == 1)
    {
        return;
    }

    // Find the tab under the mouse
    const int tabIndex = tabAt(event->pos());

    // if found and not the current tab then show tab preview
    if (tabIndex != -1
            && tabIndex != currentIndex()
            && m_currentTabPreviewIndex != tabIndex
            && event->buttons() == Qt::NoButton
       )
    {
        m_currentTabPreviewIndex = tabIndex;

        // if first time over tab, apply a small delay. If not, show it now!
        m_isFirstTimeOnTab
        ? QTimer::singleShot(200, this, SLOT(showTabPreview()))
        : showTabPreview();
    }

    // if current tab or not found then hide previous tab preview
    if (tabIndex == currentIndex() || tabIndex == -1)
    {
        hideTabPreview();
    }
}


void TabBar::leaveEvent(QEvent *event)
{
    hideTabPreview();
    m_isFirstTimeOnTab = true;

    KTabBar::leaveEvent(event);
}


void TabBar::mousePressEvent(QMouseEvent *event)
{
    hideTabPreview();

    // just close tab on middle mouse click
    if (event->button() == Qt::MidButton)
        return;

    KTabBar::mousePressEvent(event);
}


void TabBar::tabLayoutChange()
{
    KTabBar::tabLayoutChange();
    emit tabLayoutChanged();
}


void TabBar::showTabPreview()
{
    if (m_isFirstTimeOnTab)
        m_isFirstTimeOnTab = false;

    //delete previous tab preview
    delete m_previewPopup.data();
    m_previewPopup.clear();

    TabWindow *tabW = qobject_cast<TabWindow *>(parent());

    WebWindow *indexedTab = tabW->webWindow(m_currentTabPreviewIndex);
    WebWindow *currentTab = tabW->webWindow(currentIndex());

    // check if view && currentView exist before using them :)
    if (!currentTab || !indexedTab)
        return;

    // no previews during load
    if (indexedTab->isLoading())
        return;

    int w = c_baseTabWidth;
    int h = w * tabW->size().height() / tabW->size().width();

    m_previewPopup = new TabPreviewPopup(indexedTab->tabPreview(w, h), indexedTab->url().url() , this);

    int tabBarWidth = tabW->size().width();
    int leftIndex = tabRect(m_currentTabPreviewIndex).x() + (tabRect(m_currentTabPreviewIndex).width() - w) / 2;

    if (leftIndex < 0)
    {
        leftIndex = 0;
    }
    else if (leftIndex + w > tabBarWidth)
    {
        leftIndex = tabBarWidth - w;
    }

    QPoint pos(leftIndex, tabRect(m_currentTabPreviewIndex).y() + tabRect(m_currentTabPreviewIndex).height());
    m_previewPopup.data()->show(mapToGlobal(pos));
}


void TabBar::hideTabPreview()
{
    if (!m_previewPopup.isNull())
    {
        m_previewPopup.data()->hide();
    }
    m_currentTabPreviewIndex = -1;
}


void TabBar::pinTab()
{
    KAction *a = qobject_cast<KAction *>(sender());
    if (!a)
        return;
    
    int index = a->data().toInt();

    // Find the available index to move
    int availableIndex = 0;
    for (int i = 0; i < count(); i++)
    {
        if (!tabData(i).toBool())
        {
            availableIndex = i;
            break;
        }
    }

    TabWindow *w = qobject_cast<TabWindow *>(parent());
    w->moveTab(index, availableIndex);
    index = availableIndex;
    
    // set this tab data true to know this has been pinned
    setTabData(index, true);

    tabButton(index, QTabBar::RightSide)->hide();
    setTabText(index, QString());

    // workaround: "fix" the icon (or at least, try to...)
    QLabel *label = qobject_cast<QLabel* >(tabButton(index, QTabBar::LeftSide));
    if (!label)
        label = new QLabel(this);
    
    setTabButton(index, QTabBar::LeftSide, 0);
    setTabButton(index, QTabBar::LeftSide, label);

    KIcon ic = IconManager::self()->iconForUrl(w->webWindow(index)->url());
    label->setPixmap(ic.pixmap(16, 16));

    SessionManager::self()->saveSession();
}


void TabBar::unpinTab()
{
    KAction *a = qobject_cast<KAction *>(sender());
    if (!a)
        return;

    int index = a->data().toInt();

    // set the tab data false to forget this pinned tab
    setTabData(index, false);

    // Find the available index to move
    int availableIndex = 0;
    for (int i = 1; i < count(); i++)
    {
        if (!tabData(i).toBool())
        {
            availableIndex = i - 1;
            break;
        }
    }

    TabWindow *w = qobject_cast<TabWindow *>(parent());
    w->moveTab(index, availableIndex);
    index = availableIndex;
    
    tabButton(index, QTabBar::RightSide)->show();
    setTabText(index, w->webWindow(index)->title());

    // workaround: "fix" the icon (or at least, try to...)
    QLabel *label = qobject_cast<QLabel* >(tabButton(index, QTabBar::LeftSide));
    if (!label)
        label = new QLabel(this);

    setTabButton(index, QTabBar::LeftSide, 0);
    setTabButton(index, QTabBar::LeftSide, label);

    KIcon ic = IconManager::self()->iconForUrl(w->webWindow(index)->url());
    label->setPixmap(ic.pixmap(16, 16));

    SessionManager::self()->saveSession();
}
