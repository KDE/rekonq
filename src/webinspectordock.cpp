/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2009 by Nils Weigel <nehlsen at gmail dot com>
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
#include "webinspectordock.h"

// Local Includes
#include "webview.h"
#include "webpage.h"

// Qt Includes
#include <QWebInspector>

// KDE Includes
#include "KAction"
#include "KDebug"


WebInspectorDock::WebInspectorDock(QString title, QWidget *parent) 
    : QDockWidget(title, parent) 
{
    setObjectName("webInspectorDock");
    QWebInspector *inspector = new QWebInspector(this);
    setWidget(inspector);
}
    
void WebInspectorDock::closeEvent(QCloseEvent *event) 
{     
    Q_UNUSED(event);
    toggle(false);
}

MainWindow* WebInspectorDock::mainWindow()
{     
    return qobject_cast< MainWindow* >(parentWidget());
}


void WebInspectorDock::toggle(bool enable)
{
    mainWindow()->actionByName("web_inspector")->setChecked(enable);
    if (enable)
    {
        mainWindow()->currentTab()->settings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);
        findChild<QWebInspector *>()->setPage(mainWindow()->currentTab()->page());
        show();
    }
    else
    {
        hide();
        mainWindow()->currentTab()->settings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, false);
    }
}


void WebInspectorDock::changeCurrentPage() 
{     
    bool enable = mainWindow()->currentTab()->settings()->testAttribute(QWebSettings::DeveloperExtrasEnabled);
    toggle(enable);
}
