/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2009 by Andrea Diamantini <adjam7 at gmail dot com>
* Copyright (C) 2009 by Paweł Prażak <pawelprazak at gmail dot com>
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
#include "sidepanel.h"
#include "sidepanel.moc"

// Auto Includes
#include "rekonq.h"

// Local Includes
#include "panelhistory.h"


SidePanel::SidePanel(const QString &title, QWidget *parent, Qt::WindowFlags flags)
        : QDockWidget(title, parent, flags)
        , m_panelHistory(new PanelHistory(this))
{
    setObjectName("sidePanel");
    setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

    setShown(ReKonfig::showSideBar());

    connect(m_panelHistory, SIGNAL(openUrl(const KUrl&)), this, SIGNAL(openUrl(const KUrl&)));

    setWidget(m_panelHistory);
}


SidePanel::~SidePanel()
{
    // Save side panel's state
    ReKonfig::setShowSideBar(!isHidden());

    delete m_panelHistory;
}
