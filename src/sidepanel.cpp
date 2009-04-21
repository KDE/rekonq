/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2009 by Paweł Prażak <pawelprazak at gmail dot com>
*
*
* This program is free software; you can redistribute it
* and/or modify it under the terms of the GNU General
* Public License as published by the Free Software Foundation;
* either version 2, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* ============================================================ */

// Self Includes
#include "sidepanel.h"

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



