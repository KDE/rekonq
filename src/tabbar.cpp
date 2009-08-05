/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2008 Benjamin C. Meyer <ben@meyerhome.net>
* Copyright (C) 2008-2009 by Andrea Diamantini <adjam7 at gmail dot com>
* Copyright (C) 2009 by Paweł Prażak <pawelprazak at gmail dot com>
* Copyright (C) 2009 by Lionel Chauvin <megabigbug@yahoo.fr>
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
#include "application.h"
#include "mainwindow.h"
#include "history.h"
#include "urlbar.h"
#include "webview.h"

// KDE Includes
#include <KShortcut>
#include <KStandardShortcut>
#include <KDebug>
#include <KGlobalSettings>

// Qt Includes
#include <QtCore/QString>

#include <QtGui/QFont>


TabBar::TabBar(QWidget *parent)
        : KTabBar(parent)
        , m_parent(parent)
{
    setElideMode(Qt::ElideRight);
    setContextMenuPolicy(Qt::CustomContextMenu);
    setDocumentMode(true);
    setMovable(true);
    connect(this, SIGNAL(customContextMenuRequested(const QPoint &)), this,
            SLOT(contextMenuRequested(const QPoint &)));

    // tabbar font
    QFont standardFont = KGlobalSettings::generalFont();
    QString fontFamily = standardFont.family();
    int dim = standardFont.pointSize();
    setFont(QFont(fontFamily, dim - 1));
}


TabBar::~TabBar()
{
}


QSize TabBar::tabSizeHint(int index) const
{
    //TODO Create a SuperTabWidget class

    int buttonSize = ((MainView *)m_parent)->addTabButton()->size().width();
    int tabBarWidth = m_parent->size().width() - buttonSize;
    int baseWidth =  m_parent->sizeHint().width()/4;
    int minWidth =  m_parent->sizeHint().width()/8;

    int w;
    if (baseWidth*count()<tabBarWidth)
    {
        w = baseWidth;
    }
    else 
    {
        if (tabBarWidth/count()>minWidth)
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


void TabBar::contextMenuRequested(const QPoint &position)
{
    KMenu menu;
    MainWindow *mainWindow = Application::instance()->mainWindow();

    menu.addAction(mainWindow->actionByName(QLatin1String("new_tab")));
    int index = tabAt(position);
    if (-1 != index)
    {
        m_actualIndex = index;

        menu.addAction(KIcon("tab-duplicate"), i18n("Clone Tab"), this, SLOT(cloneTab()));
        menu.addSeparator();
        menu.addAction(KIcon("tab-close"), i18n("&Close Tab"), this, SLOT(closeTab()));
        menu.addAction(KIcon("tab-close-other"), i18n("Close &Other Tabs"), this, SLOT(closeOtherTabs()));
        menu.addSeparator();
        menu.addAction(KIcon("view-refresh"), i18n("Reload Tab"), this, SLOT(reloadTab()));
    }
    else
    {
        menu.addSeparator();
    }
    menu.addAction(i18n("Reload All Tabs"), this, SIGNAL(reloadAllTabs()));
    menu.exec(QCursor::pos());
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
