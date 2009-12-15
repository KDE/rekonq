/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2009 by Andrea Diamantini <adjam7 at gmail dot com>
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
#include "protocolhandler.h"

// Local Includes
#include "newtabpage.h"

// KDE Includes
#include <KUrl>
#include <KRun>
#include <KToolInvocation>

// Qt Includes
#include <QLatin1String>
#include <QNetworkRequest>
#include <QWebFrame>


ProtocolHandler::ProtocolHandler()
{
}


ProtocolHandler::~ProtocolHandler()
{
}


bool ProtocolHandler::handle(const QNetworkRequest &request, QWebFrame *frame)
{
    KUrl url( request.url() );
    
    // mailto handling
    if ( url.protocol() == QLatin1String("mailto") )
    {
        KToolInvocation::invokeMailer(url);
        return true;
    }

    if ( url.protocol() == QLatin1String("about") )
    {
        if( url == KUrl("about:closedTabs")
            || url == KUrl("about:history")
            || url == KUrl("about:bookmarks")
            || url == KUrl("about:favorites")
            || url == KUrl("about:home")
            )
        {
            NewTabPage p(frame);
            p.generate(url);
        
            return true;
        }
    }

    // "ftp" handling
    if(url.protocol() == QLatin1String("ftp"))
    {
        KUrl::List list;
        list.append(url);
        KRun::run("dolphin %u",url,0);

        return true;
    }
    
    // "file" handling
    if(url.protocol() == QLatin1String("file"))
    {
        KUrl::List list;
        list.append(url);
        KRun::run("dolphin %u",url,0);

        return true;
    }
    
    return false;
}
