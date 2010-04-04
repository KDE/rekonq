/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2010 by Andrea Diamantini <adjam7 at gmail dot com>
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
#include "iconbutton.h"
#include "iconbutton.moc"

// Local Includes
#include "application.h"

// KDE Includes
#include <KDebug>


IconButton::IconButton(QWidget *parent)
    : QToolButton(parent)
{
    QPalette p = palette();
    p.setColor( QPalette::Button, Qt::transparent );
    setPalette(p);

    setCursor(Qt::ArrowCursor);
    setStyleSheet("IconButton { border: none; padding: 0px}");
}


void IconButton::setIconUrl(const KUrl &url, bool trusted)
{
    setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    setIcon( Application::icon(url) );
    setText( url.host() );
    
    if(trusted)
    {
        setStyleSheet("IconButton { background-color:#0F0; padding: 2px }");
    }
    else
    {
        setStyleSheet("IconButton { background-color:#F00; padding: 2px}");
    }
    
    adjustSize();
}


void IconButton::updateIcon(KIcon icon)
{
    setToolButtonStyle(Qt::ToolButtonIconOnly);
    setIcon( icon );

    setStyleSheet("IconButton { background-color:transparent; border: none; padding: 0px}");
    adjustSize();
}
