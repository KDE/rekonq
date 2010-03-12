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


//Self Includes
#include "tabbar.h"
#include "tabbar.moc"

// Local Includes
#include "rekonq.h"
#include "application.h"
#include "mainwindow.h"
#include "urlbar.h"
#include "webtab.h"
#include "websnap.h"
#include "mainview.h"

// KDE Includes
#include <KShortcut>
#include <KStandardShortcut>
#include <KGlobalSettings>
#include <KPassivePopup>
#include <KMenu>

// Qt Includes
#include <QString>
#include <QFont>
#include <QToolButton>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QVBoxLayout>


#define BASE_WIDTH_DIVISOR    4
#define MIN_WIDTH_DIVISOR     8


TabBar::TabBar(QWidget *parent)
        : KTabBar(parent)
        , m_currentTabPreview(-1)
{
    setElideMode(Qt::ElideRight);

    setDocumentMode(true);
    setTabsClosable(true);
    setMovable(true);

    setContextMenuPolicy(Qt::CustomContextMenu);

    connect(this, SIGNAL(contextMenu(int, const QPoint &)), this, SLOT(contextMenu(int, const QPoint &)));
    connect(this, SIGNAL(emptyAreaContextMenu(const QPoint &)), this, SLOT(emptyAreaContextMenu(const QPoint &)));
}


TabBar::~TabBar()
{
}


QSize TabBar::tabSizeHint(int index) const
{
    MainView *view = qobject_cast<MainView *>(parent());
    
    int buttonSize = view->addTabButton()->size().width();
    int tabBarWidth = view->size().width() - buttonSize;
    int baseWidth =  view->sizeHint().width()/BASE_WIDTH_DIVISOR;
    int minWidth =  view->sizeHint().width()/MIN_WIDTH_DIVISOR;

    int w;
    if (baseWidth*count() < tabBarWidth)
    {
        w = baseWidth;
    }
    else 
    {
        if (count() > 0 && tabBarWidth/count() > minWidth)
        {
            w = tabBarWidth/count();
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
}


void TabBar::closeTab()
{
    emit closeTab(m_actualIndex);
}


void TabBar::closeOtherTabs()
{
    emit closeOtherTabs(m_actualIndex);
}


void TabBar::reloadTab()
{
    emit reloadTab(m_actualIndex);
}


void TabBar::detachTab()
{
    emit detachTab(m_actualIndex);
}


void TabBar::showTabPreview(int tab)
{
    MainView *mv = qobject_cast<MainView *>(parent());
    
    WebTab *indexedTab = mv->webTab(tab);
    WebTab *currentTab = mv->webTab(currentIndex());

    // check if view && currentView exist before using them :)
    if(!currentTab || !indexedTab)
        return;
    
    int w = tabSizeHint(tab).width();
    int h = w * ( (0.0 + currentTab->height()) / currentTab->width() );

    //delete previous tab preview
    delete m_previewPopup.data();
    m_previewPopup.clear();
    
    m_previewPopup = new KPassivePopup(this);
    m_previewPopup.data()->setFrameShape(QFrame::StyledPanel);
    m_previewPopup.data()->setFrameShadow(QFrame::Plain);
    m_previewPopup.data()->setFixedSize(w, h);
    
    QLabel *l = new QLabel();
    l->setPixmap( WebSnap::renderPreview( *indexedTab->page() , w, h) );
    
    m_previewPopup.data()->setView(l);
    m_previewPopup.data()->layout()->setAlignment(Qt::AlignTop);
    m_previewPopup.data()->layout()->setMargin(0);

    QPoint pos( tabRect(tab).x() , tabRect(tab).y() + tabRect(tab).height() - 3);
    m_previewPopup.data()->show(mapToGlobal(pos));
}


void TabBar::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton)
    {
        // hide addNewTabButton when moving tabs
        MainView *view = qobject_cast<MainView *>(parent());
        QTimer::singleShot(200, view->addTabButton(), SLOT(hide()));
    }
        
    if (ReKonfig::alwaysShowTabPreviews())
    {
        //Find the tab under the mouse
        int i = 0;
        int tab = -1;
        while (i<count() && tab==-1)
        {
            if (tabRect(i).contains(event->pos())) 
            {
                tab = i;
            }
            i++;
        }

        //if found and not the current tab then show tab preview
        if (tab != -1 && tab != currentIndex() && m_currentTabPreview != tab && event->buttons() == Qt::NoButton)
        {
            showTabPreview(tab);
            m_currentTabPreview = tab;
        }

        //if current tab or not found then hide previous tab preview
        if (tab==currentIndex() || tab==-1)
        {
            if ( !m_previewPopup.isNull() )
            {
                m_previewPopup.data()->hide();
            }
            m_currentTabPreview = -1;
        }
    }

    KTabBar::mouseMoveEvent(event);
}


void TabBar::leaveEvent(QEvent *event)
{
    if (ReKonfig::alwaysShowTabPreviews())
    {
        //if leave tabwidget then hide previous tab preview
        if ( !m_previewPopup.isNull() )
        {
            m_previewPopup.data()->hide();
        }
        m_currentTabPreview = -1;
    }

    KTabBar::leaveEvent(event);
}


void TabBar::mousePressEvent(QMouseEvent *event)
{
    // just close tab on middle mouse click
    if (event->button() == Qt::MidButton)
        return;
    
    KTabBar::mousePressEvent(event);
}


void TabBar::contextMenu(int tab, const QPoint &pos)
{
    m_actualIndex = tab;

    KMenu menu;
    MainWindow *mainWindow = Application::instance()->mainWindow();

    menu.addAction(mainWindow->actionByName(QLatin1String("new_tab")));
    menu.addAction( mainWindow->actionByName("clone_tab") );
    menu.addAction( mainWindow->actionByName("detach_tab") );
    menu.addSeparator();
    menu.addAction( mainWindow->actionByName("close_tab") );
    menu.addAction( mainWindow->actionByName("close_other_tabs") );
    menu.addSeparator();
    menu.addAction( mainWindow->actionByName("reload_tab") );
    menu.addAction( mainWindow->actionByName("reload_all_tabs") );

    menu.exec(pos);
}


void TabBar::emptyAreaContextMenu(const QPoint &pos)
{
    KMenu menu;
    MainWindow *mainWindow = Application::instance()->mainWindow();

    menu.addAction(mainWindow->actionByName(QLatin1String("new_tab")));
    menu.addSeparator();
    menu.addAction( mainWindow->actionByName("reload_all_tabs") );

    menu.exec(pos);
}


void TabBar::mouseReleaseEvent(QMouseEvent *event)
{
    MainView *mv = qobject_cast<MainView *>(parent());
    QTimer::singleShot(200, mv->addTabButton(), SLOT(show()));
    
    KTabBar::mouseReleaseEvent(event);
}


void TabBar::tabRemoved(int index)
{
    Q_UNUSED(index)
    if (ReKonfig::alwaysShowTabPreviews())
    {
        if ( !m_previewPopup.isNull() )
        {
            m_previewPopup.data()->hide();
        }
        m_currentTabPreview = -1;
    }
}
