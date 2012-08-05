/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2008 Benjamin C. Meyer <ben@meyerhome.net>
* Copyright (C) 2008-2012 by Andrea Diamantini <adjam7 at gmail dot com>
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


//Self Includes
#include "tabbar.h"
#include "tabbar.moc"

// Local Includes
#include "rekonq.h"

#include "application.h"
#include "historymanager.h"
#include "iconmanager.h"
#include "mainview.h"
#include "mainwindow.h"
#include "webpage.h"
#include "webtab.h"
#include "websnap.h"
#include "tabhighlighteffect.h"
#include "tabpreviewpopup.h"
#include "searchengine.h"

// KDE Includes
#include <KActionMenu>
#include <KMenu>
#include <KToolBar>
#include <KColorScheme>
#include <KAcceleratorManager>

// Qt Includes
#include <QLabel>
#include <QLayout>
#include <QMouseEvent>
#include <QToolButton>
#include <QPropertyAnimation>
#include <QStyleOptionFrameV3>
#include <QSignalMapper>
#include <QTimer>


static inline QByteArray highlightPropertyName(int index)
{
    return QByteArray("hAnim").append(QByteArray::number(index));
}


// ----------------------------------------------


TabBar::TabBar(QWidget *parent)
    : KTabBar(parent)
    , m_actualIndex(-1)
    , m_currentTabPreviewIndex(-1)
    , m_isFirstTimeOnTab(true)
    , m_tabHighlightEffect(new TabHighlightEffect(this))
    , m_animationMapper(new QSignalMapper(this))
{
    setElideMode(Qt::ElideRight);

    setDocumentMode(true);
    setTabsClosable(true);
    setMovable(true);

    setContextMenuPolicy(Qt::CustomContextMenu);

    // avoid ambiguos shortcuts. See BUG:275858
    KAcceleratorManager::setNoAccel(this);

    connect(this, SIGNAL(contextMenu(int, QPoint)), this, SLOT(contextMenu(int, QPoint)));
    connect(this, SIGNAL(emptyAreaContextMenu(QPoint)), this, SLOT(emptyAreaContextMenu(QPoint)));

    connect(m_animationMapper, SIGNAL(mapped(int)), this, SLOT(removeAnimation(int)));
    setGraphicsEffect(m_tabHighlightEffect);

    setAnimatedTabHighlighting(ReKonfig::animatedTabHighlighting());
    setAcceptDrops(true);
}


QSize TabBar::tabSizeHint(int index) const
{
    Q_UNUSED(index);

    MainView *view = qobject_cast<MainView *>(parent());

    int buttonSize = view->addTabButton()->size().width();
    int tabBarWidth = view->size().width() - buttonSize;
    int baseWidth =  view->sizeHint().width() / baseWidthDivisor;
    int minWidth =  view->sizeHint().width() / minWidthDivisor;

    int w;
    if (baseWidth * count() < tabBarWidth)
    {
        w = baseWidth;
    }
    else
    {
        if (count() > 0 && tabBarWidth / count() > minWidth)
        {
            w = tabBarWidth / count();
        }
        else
        {
            w = minWidth;
        }
    }

    int h = KTabBar::tabSizeHint(index).height();

    QSize ts = QSize(w, h);
    return ts;
}


void TabBar::cloneTab()
{
    emit cloneTab(m_actualIndex);
    m_actualIndex = -1;
}


void TabBar::closeTab()
{
    emit closeTab(m_actualIndex);
    m_actualIndex = -1;
}


void TabBar::closeOtherTabs()
{
    emit closeOtherTabs(m_actualIndex);
    m_actualIndex = -1;
}


void TabBar::reloadTab()
{
    emit reloadTab(m_actualIndex);
    m_actualIndex = -1;
}


void TabBar::detachTab()
{
    emit detachTab(m_actualIndex);
    m_actualIndex = -1;
}


void TabBar::showTabPreview()
{
    if (m_isFirstTimeOnTab)
        m_isFirstTimeOnTab = false;

    //delete previous tab preview
    delete m_previewPopup.data();
    m_previewPopup.clear();

    MainView *mv = qobject_cast<MainView *>(parent());

    WebTab *indexedTab = mv->webTab(m_currentTabPreviewIndex);
    WebTab *currentTab = mv->webTab(currentIndex());

    // check if view && currentView exist before using them :)
    if (!currentTab || !indexedTab)
        return;

    // no previews during load
    if (indexedTab->isPageLoading())
        return;

    // Make sure the hovered webtab match the current size
    // Only the active one is updated by window resize events
    indexedTab->resize(currentTab->size());

    m_previewPopup = new TabPreviewPopup(indexedTab , this);

    int tabWidth = tabSizeHint(m_currentTabPreviewIndex).width();
    int tabBarWidth = mv->size().width();
    int leftIndex = tabRect(m_currentTabPreviewIndex).x() + (tabRect(m_currentTabPreviewIndex).width() - tabWidth) / 2;
    int popupWidth = m_previewPopup.data()->thumbnailSize().width();

    // Center the popup if the tab width is bigger or smaller
    leftIndex += (tabWidth - popupWidth) / 2;

    if (leftIndex < 0)
    {
        leftIndex = 0;
    }
    else if (leftIndex + tabWidth > tabBarWidth)
    {
        leftIndex = tabBarWidth - tabWidth;
    }

    QPoint pos(leftIndex, tabRect(m_currentTabPreviewIndex).y() + tabRect(m_currentTabPreviewIndex).height());
    m_previewPopup.data()->show(mapToGlobal(pos));
}


void TabBar::hideEvent(QHideEvent *event)
{
    if (!event->spontaneous())
    {
        qobject_cast<MainView *>(parent())->addTabButton()->hide();
    }
    KTabBar::hideEvent(event);
}


void TabBar::showEvent(QShowEvent *event)
{
    KTabBar::showEvent(event);
    if (!event->spontaneous())
    {
        qobject_cast<MainView *>(parent())->addTabButton()->show();
    }
}


void TabBar::mouseMoveEvent(QMouseEvent *event)
{
    if (count() == 1)
    {
        return;
    }

    KTabBar::mouseMoveEvent(event);

    if (ReKonfig::hoveringTabOption() == 0)
    {
        //Find the tab under the mouse
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
            if (!m_previewPopup.isNull())
            {
                m_previewPopup.data()->hide();
            }
            m_currentTabPreviewIndex = -1;
        }
    }
}


void TabBar::leaveEvent(QEvent *event)
{
    if (ReKonfig::hoveringTabOption() == 0)
    {
        //if leave tabwidget then hide previous tab preview
        if (!m_previewPopup.isNull())
        {
            m_previewPopup.data()->hide();
        }
        m_currentTabPreviewIndex = -1;
        m_isFirstTimeOnTab = true;
    }

    KTabBar::leaveEvent(event);
}


void TabBar::mousePressEvent(QMouseEvent *event)
{
    if (ReKonfig::hoveringTabOption() == 0)
    {
        if (!m_previewPopup.isNull())
        {
            m_previewPopup.data()->hide();
        }
        m_currentTabPreviewIndex = -1;
    }

    // just close tab on middle mouse click
    if (event->button() == Qt::MidButton)
        return;

    KTabBar::mousePressEvent(event);
}


void TabBar::contextMenu(int tab, const QPoint &pos)
{
    KActionMenu *closedTabsMenu = setupHistoryActions();

    m_actualIndex = tab;

    KMenu menu;
    MainWindow *mainWindow = rApp->mainWindow();

    menu.addAction(mainWindow->actionByName(QL1S("new_tab")));
    menu.addAction(mainWindow->actionByName(QL1S("clone_tab")));
    if (count() > 1)
    {
        menu.addAction(mainWindow->actionByName(QL1S("detach_tab")));
    }
    menu.addAction(mainWindow->actionByName(QL1S("open_last_closed_tab")));
    menu.addAction(closedTabsMenu);
    menu.addSeparator();
    menu.addAction(mainWindow->actionByName(QL1S("close_tab")));
    if (count() > 1)
    {
        menu.addAction(mainWindow->actionByName(QL1S("close_other_tabs")));
    }
    menu.addSeparator();
    menu.addAction(mainWindow->actionByName(QL1S("reload_tab")));
    if (count() > 1)
    {
        menu.addAction(mainWindow->actionByName(QL1S("reload_all_tabs")));
    }
    menu.exec(pos);
}


void TabBar::emptyAreaContextMenu(const QPoint &pos)
{
    KActionMenu *closedTabsMenu = setupHistoryActions();

    KMenu menu;
    MainWindow *mainWindow = rApp->mainWindow();

    menu.addAction(mainWindow->actionByName(QL1S("new_tab")));
    menu.addAction(mainWindow->actionByName(QL1S("open_last_closed_tab")));
    menu.addAction(closedTabsMenu);
    menu.addSeparator();
    menu.addAction(mainWindow->actionByName(QL1S("reload_all_tabs")));

    KToolBar *mainBar = mainWindow->toolBar("mainToolBar");
    if (!mainBar->isVisible())
    {
        menu.addAction(mainBar->toggleViewAction());
    }

    menu.exec(pos);
}


void TabBar::tabRemoved(int index)
{
    if (ReKonfig::hoveringTabOption() == 0)
    {
        if (!m_previewPopup.isNull())
        {
            m_previewPopup.data()->hide();
        }
        m_currentTabPreviewIndex = -1;
    }

    if (ReKonfig::animatedTabHighlighting())
        removeAnimation(index);
}


KActionMenu *TabBar::setupHistoryActions()
{
    MainWindow *w = rApp->mainWindow();
    MainView *mv = qobject_cast<MainView *>(parent());

    QAction *openLastClosedTabAction = w->actionByName(QL1S("open_last_closed_tab"));

    bool closedTabsAvailable = (mv->recentlyClosedTabs().size() > 0);
    openLastClosedTabAction->setEnabled(closedTabsAvailable);

    KActionMenu *am = new KActionMenu(KIcon("tab-new"), i18n("Closed Tabs"), this);
    am->setDelayed(false);
    am->setEnabled(closedTabsAvailable);

    if (am->menu())
        am->menu()->clear();

    if (!closedTabsAvailable)
        return am;

    for (int i = 0; i < mv->recentlyClosedTabs().count(); ++i)
    {
        TabHistory item = mv->recentlyClosedTabs().at(i);
        KAction *a = new KAction(rApp->iconManager()->iconForUrl(item.url), item.title, this);
        a->setData(i);
        connect(a, SIGNAL(triggered()), mv, SLOT(openClosedTab()));
        am->addAction(a);
    }

    return am;
}


QRect TabBar::tabTextRect(int index)
{
    QStyleOptionTabV3 option;
    initStyleOption(&option, index);
    return style()->subElementRect(QStyle::SE_TabBarTabText, &option, this);
}


void TabBar::setTabHighlighted(int index)
{
    const QByteArray propertyName = highlightPropertyName(index);
    const QColor highlightColor = KColorScheme(QPalette::Active, KColorScheme::Window).foreground(KColorScheme::PositiveText).color();

    if (tabTextColor(index) != highlightColor)
    {
        if (ReKonfig::animatedTabHighlighting())
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
        }

        setTabTextColor(index, highlightColor);
    }
}


void TabBar::resetTabHighlighted(int index)
{
    if (ReKonfig::animatedTabHighlighting())
        removeAnimation(index);

    setTabTextColor(index, KColorScheme(QPalette::Active, KColorScheme::Window).foreground(KColorScheme::NormalText).color());
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


void TabBar::setAnimatedTabHighlighting(bool enabled)
{
    if (enabled)
        m_tabHighlightEffect->setEnabled(true);
    else
    {
        m_tabHighlightEffect->setEnabled(false);

        //cleanup
        QHashIterator<QByteArray, QPropertyAnimation*> i(m_highlightAnimation);
        while (i.hasNext())
        {
            i.next();
            m_tabHighlightEffect->setProperty(i.key(), QVariant()); //destroy the property

            QPropertyAnimation *anim = m_highlightAnimation.take(i.key());
            m_animationMapper->removeMappings(anim);
            delete anim;
        }
    }
}


void TabBar::dropEvent(QDropEvent* event)
{
    if (event->mimeData()->hasUrls())
    {
        int urlCount = event->mimeData()->urls().count();
        if (urlCount > 1)
        {
            Q_FOREACH(const QUrl & url, event->mimeData()->urls())
            rApp->loadUrl(url, Rekonq::NewTab);
        }
        else
            rApp->loadUrl(event->mimeData()->urls().first(), Rekonq::NewFocusedTab);
    }
    else if (event->mimeData()->hasText())
    {
        //In case the text is a valid URL
        if (isURLValid(event->mimeData()->text()))
            rApp->loadUrl(KUrl(event->mimeData()->text()), Rekonq::NewFocusedTab);
        else
        {
            KService::Ptr defaultSearchEngine = SearchEngine::defaultEngine();
            if (defaultSearchEngine)
                rApp->loadUrl(KUrl(SearchEngine::buildQuery(defaultSearchEngine, event->mimeData()->text())), Rekonq::NewFocusedTab);
        }
    }
    KTabBar::dropEvent(event);
}


void TabBar::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->hasUrls() || event->mimeData()->hasText())
        event->acceptProposedAction();
    else
        KTabBar::dragEnterEvent(event);
}


bool TabBar::isURLValid(const QString &url)
{
    QString editedURL = url;
    bool isValid = false;
    if (editedURL.startsWith(QL1S("http://")) || editedURL.startsWith(QL1S("https://")) || editedURL.startsWith(QL1S("ftp://")))
        editedURL = editedURL.remove(QRegExp("(http|https|ftp)://"));
    if (editedURL.contains(QL1C('.')) && editedURL.indexOf(QL1C('.')) > 0 && editedURL.indexOf(QL1C('.')) < editedURL.length() && !editedURL.trimmed().contains(QL1C(' '))
            && QUrl::fromUserInput(editedURL).isValid())
        isValid = true;
    return isValid;
}


void TabBar::tabLayoutChange()
{
    KTabBar::tabLayoutChange();
    emit tabLayoutChanged();
}
