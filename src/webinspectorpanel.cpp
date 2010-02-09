/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2009 by Matthieu Gicquel<matgic78 at gmail dot com>
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
#include "webtab.h"
#include "webview.h"
#include "webpage.h"

// KDE Includes
#include "KAction"
#include "KDebug"

// Qt Includes
#include <QWebInspector>


WebInspectorPanel::WebInspectorPanel(QString title, QWidget *parent) 
    : QDockWidget(title, parent) 
{
    setObjectName("webInspectorDock");
    setWidget( new QWebInspector(this) );
}
    

void WebInspectorPanel::closeEvent(QCloseEvent *event) 
{     
    Q_UNUSED(event);
    toggle(false);
}


MainWindow* WebInspectorPanel::mainWindow()
{     
    return qobject_cast< MainWindow* >(parentWidget());
}


void WebInspectorPanel::toggle(bool enable)
{
    mainWindow()->actionByName("web_inspector")->setChecked(enable);
    if (enable)
    {
        mainWindow()->currentTab()->view()->settings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);
        findChild<QWebInspector *>()->setPage(mainWindow()->currentTab()->page());
        show();
    }
    else
    {
        hide();
        mainWindow()->currentTab()->view()->settings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, false);
    }
}


void WebInspectorPanel::changeCurrentPage() 
{     
    bool enable = mainWindow()->currentTab()->view()->settings()->testAttribute(QWebSettings::DeveloperExtrasEnabled);
    toggle(enable);
}
