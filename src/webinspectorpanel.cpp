/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2010 by Matthieu Gicquel <matgic78 at gmail dot com>
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


// Self Includes
#include "webinspectorpanel.h"
#include "webinspectorpanel.moc"

// Local Includes
#include "mainwindow.h"
#include "webpage.h"
#include "webtab.h"

// Qt Includes
#include <QtGui/QAction>
#include <QtWebKit/QWebInspector>


WebInspectorPanel::WebInspectorPanel(QString title, QWidget *parent)
    : QDockWidget(title, parent)
    , _inspector(0)
{
    setObjectName("webInspectorDock");
}


void WebInspectorPanel::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event);
    toggle(false);
}


void WebInspectorPanel::toggle(bool enable)
{
    MainWindow *w = qobject_cast<MainWindow *>(parent());
    w->actionByName(QL1S("web_inspector"))->setChecked(enable);
    if(enable)
    {
        w->currentTab()->page()->settings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);
        if(!_inspector)
        {
            _inspector = new QWebInspector(this);
            _inspector->setPage(w->currentTab()->page());
            setWidget(_inspector);
        }
        show();
    }
    else
    {
        w->currentTab()->page()->settings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, false);
        delete _inspector;
        _inspector = 0;
        hide();
    }
}


void WebInspectorPanel::changeCurrentPage()
{
    MainWindow *w = qobject_cast<MainWindow *>(parent());
    bool enable = w->currentTab()->page()->settings()->testAttribute(QWebSettings::DeveloperExtrasEnabled);
    toggle(enable);
}
