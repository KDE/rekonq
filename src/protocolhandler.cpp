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
#include <klocalizedstring.h>
#include <KUrl>
#include <KRun>
#include <KToolInvocation>
#include <KStandardDirs>
#include <KDebug>
#include <KMimeType>
#include <KIconLoader>

// Qt Includes
#include <QLatin1String>
#include <QNetworkRequest>
#include <QWebFrame>
#include <QDir>
#include <QFile>
#include <QDateTime>


ProtocolHandler::ProtocolHandler()
{
}


ProtocolHandler::~ProtocolHandler()
{
}


bool ProtocolHandler::handle(const QNetworkRequest &request, QWebFrame *frame)
{
    KUrl url( request.url() );
    
    // "mailto" handling
    if ( url.protocol() == QLatin1String("mailto") )
    {
        KToolInvocation::invokeMailer(url);
        return true;
    }

    // "about" handling
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
        QString html = fileHandling(url);
        kDebug() << html;
        frame->setHtml( html );
//         KUrl::List list;
//         list.append(url);
//         KRun::run("dolphin %u",url,0);

        return true;
    }
    
    return false;
}


QString ProtocolHandler::fileHandling(const KUrl &url)
{
    QDir dir(url.toLocalFile());
    
    if (!dir.exists()) 
    {
        QString errStr = i18n("Error opening: %1: No such file or directory", dir.absolutePath() );
        return errStr;
    }
    if (!dir.isReadable()) 
    {
        QString errStr = i18n("Unable to read %1", dir.absolutePath() );
        return errStr;
    }
    
     // display "not found" page
    QString notfoundFilePath =  KStandardDirs::locate("data", "rekonq/htmls/notfound.html");
    QFile file(notfoundFilePath);

    bool isOpened = file.open(QIODevice::ReadOnly);
    if (!isOpened)
    {
        return QString("rekonq error, sorry :(");
    }

    QString title = url.path(); 
    QString imagesPath = QString("file://") + KGlobal::dirs()->findResourceDir("data", "rekonq/pics/bg.png") + QString("rekonq/pics");
    QString msg = "<h1>" + url.path() + "</h1>";

    dir.setFilter(QDir::AllDirs | QDir::Files | QDir::NoDotAndDotDot);
    QFileInfoList entries = dir.entryInfoList();

    msg += "<table>";
    msg += "<tr><th>" + i18n("Name") + "</th><th>" + i18n("Size") + "</th><th>" + i18n("Last Modified") + "</th></tr>";
    
    foreach(const QFileInfo &item, entries)
    {
        msg += "<tr>";
        QString fullPath = QString("file://") + item.absoluteFilePath();
        
        QString iconName = KMimeType::defaultMimeTypePtr()->iconNameForUrl(fullPath);
        kDebug() << "***************************************" << iconName << "********************************";
        QString icon = QString("file://") + KIconLoader::global()->iconPath( iconName, KIconLoader::Small );
        
        msg += "<td>";
        msg += "<img src=\"" + icon + "\" alt=\"" + iconName + "\" /> ";
        msg += "<a href=\"" + fullPath + "\">" + item.fileName() + "</a>";
        msg += "</td>";
        
        msg += "<td>";
        msg += QString::number( item.size()/1024 ) + "KB";
        msg += "</td>";
        
        msg += "<td>";
        msg += item.lastModified().toString("dd/MM/yyyy hh:mm:ss");
        msg += "</td>";
        
        msg += "</tr>";
    }
    msg += "</table>";
    
         
    QString html = QString(QLatin1String(file.readAll()))
                            .arg(title)
                            .arg(imagesPath)
                            .arg(msg)
                            ;
                           
    return html;
}
