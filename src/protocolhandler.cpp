/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2010 by Andrea Diamantini <adjam7 at gmail dot com>
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
#include "protocolhandler.moc"

// Auto Includes
#include "rekonq.h"

// Local Includes
#include "newtabpage.h"
#include "application.h"
#include "mainwindow.h"
#include "mainview.h"
#include "urlbar.h"
#include "historymanager.h"

// KDE Includes
#include <klocalizedstring.h>
#include <KUrl>
#include <KRun>
#include <KToolInvocation>
#include <KStandardDirs>
#include <KDebug>
#include <KMimeType>
#include <KIconLoader>
#include <KDirLister>
#include <KFileItem>
#include <KJob>
#include <kio/udsentry.h>

// Qt Includes
#include <QLatin1String>
#include <QNetworkRequest>
#include <QWebFrame>
#include <QDir>
#include <QFile>
#include <QDateTime>


ProtocolHandler::ProtocolHandler(QObject *parent)
    : QObject(parent)
    , _lister(new KDirLister)
    , _frame(0)
{
    connect( _lister, SIGNAL(newItems(const KFileItemList &)), this, SLOT(showResults(const KFileItemList &)));
}


ProtocolHandler::~ProtocolHandler()
{
}


bool ProtocolHandler::preHandling(const QNetworkRequest &request, QWebFrame *frame)
{
    _url = request.url();
    _frame = frame;
    
    kDebug() << "URL PROTOCOL: " << _url;
    
    // relative urls
    if(_url.isRelative())
        return false;
    
    // "http(s)" (fast) handling
    if( _url.protocol() == QLatin1String("http") || _url.protocol() == QLatin1String("https") )
        return false;
    
    // javascript handling
    if( _url.protocol() == QLatin1String("javascript") )
    {
        QString scriptSource = _url.authority();
        QVariant result = frame->evaluateJavaScript(scriptSource);
        return true;
    }
    
    // "mailto" handling
    if ( _url.protocol() == QLatin1String("mailto") )
    {
        KToolInvocation::invokeMailer(_url);
        return true;
    }

    // "about" handling
    if ( _url.protocol() == QLatin1String("about") )
    {
        NewTabPage p(frame);
        p.generate(_url);
        return true;
    }
    
    return false;
}


bool ProtocolHandler::postHandling(const QNetworkRequest &request, QWebFrame *frame)
{
    _url = request.url();
    _frame = frame;
    
    kDebug() << "URL PROTOCOL: " << _url;
    
    // "http(s)" (fast) handling
    if( _url.protocol() == QLatin1String("http") || _url.protocol() == QLatin1String("https") )
        return false;
    
    // "mailto" handling: It needs to be handled both here(mail links clicked)
    // and in prehandling (mail url launched)
    if ( _url.protocol() == QLatin1String("mailto") )
    {
        KToolInvocation::invokeMailer(_url);
        return true;
    }

    // "ftp" handling. A little bit "hard" handling this. Hope I found
    // the best solution.
    // My idea is: webkit cannot handle in any way ftp. So we have surely to return true here.
    // We start trying to guess what the url represent: it's a dir? show its contents (and download them).
    // it's a file? download it. It's another thing? beat me, but I don't know what to do...
    if( _url.protocol() == QLatin1String("ftp") )
    {
        KIO::StatJob *job = KIO::stat(_url);
        connect(job, SIGNAL(result(KJob*)), this, SLOT( slotMostLocalUrlResult(KJob*) ));
        return true;
    }
    
    // "file" handling. This is quite trivial :)
    if(_url.protocol() == QLatin1String("file") )
    {
        QFileInfo fileInfo( _url.path() );
        if(fileInfo.isDir())
        {
            _lister->openUrl(_url);
            Application::instance()->mainWindow()->mainView()->urlBar()->setUrl(_url);
            return true;
        }
    }
    
    return false;
}


QString ProtocolHandler::dirHandling(const KFileItemList &list)
{

    KFileItem mainItem = _lister->rootItem();
    KUrl rootUrl = mainItem.url();
    
    if (mainItem.isNull()) 
    {
        QString errStr = i18nc("%1=an URL", "Error opening '%1': No such file or directory.", rootUrl.prettyUrl() );
        return errStr;
    }
    
    if (!mainItem.isReadable()) 
    {
        QString errStr = i18nc("%1=an URL", "Unable to read %1", rootUrl.prettyUrl() );
        return errStr;
    }
    
     // display "rekonq info" page
    QString infoFilePath =  KStandardDirs::locate("data", "rekonq/htmls/rekonqinfo.html");
    QFile file(infoFilePath);

    bool isOpened = file.open(QIODevice::ReadOnly);
    if (!isOpened)
    {
        return QString("rekonq error, sorry :(");
    }

    QString title = _url.prettyUrl(); 
    QString msg = i18nc("%1=an URL", "<h1>Index of %1</h1>", _url.prettyUrl());


    if(rootUrl.cd(".."))
    {
        QString path = rootUrl.prettyUrl();
        QString uparrow = KIconLoader::global()->iconPath( "arrow-up", KIconLoader::Small );
        msg += "<img src=\"file://" + uparrow + "\" alt=\"up-arrow\" />";
        msg += "<a href=\"" + path + "\">" + i18n("Up to higher level directory") + "</a><br /><br />";
    }
    
    msg += "<table width=\"100%\">";
    msg += "<tr><th align=\"left\">" + i18n("Name") + "</th><th>" + i18n("Size") + "</th><th>" + i18n("Last Modified") + "</th></tr>";

    foreach(const KFileItem &item, list)
    {
        msg += "<tr>";
        QString fullPath = item.url().prettyUrl();
        
        QString iconName = item.iconName();
        QString icon = QString("file://") + KIconLoader::global()->iconPath( iconName, KIconLoader::Small );
        
        msg += "<td width=\"70%\">";
        msg += "<img src=\"" + icon + "\" alt=\"" + iconName + "\" /> ";
        msg += "<a href=\"" + fullPath + "\">" + item.name() + "</a>";
        msg += "</td>";
        
        msg += "<td align=\"right\">";
        if(item.isFile())
        {
            msg += QString::number( item.size()/1024 ) + " KB";
        }
        msg += "</td>";
        
        msg += "<td align=\"right\">";
        msg += item.timeString();
        msg += "</td>";
        
        msg += "</tr>";
    }
    msg += "</table>";
    
         
    QString html = QString(QLatin1String(file.readAll()))
                            .arg(title)
                            .arg(msg)
                            ;
                           
    return html;
}


void ProtocolHandler::showResults(const KFileItemList &list)
{
    if(_lister->rootItem().isFile())
    {
        WebPage *page = qobject_cast<WebPage *>( _frame->page() );
        page->downloadUrl( _lister->rootItem().url() );
        return;
    }
    
    if ( list.isEmpty() )
        return;
    
    QString html = dirHandling(list);
    _frame->setHtml(html);

    Application::instance()->mainWindow()->mainView()->urlBar()->setUrl(_url);
    Application::historyManager()->addHistoryEntry( _url.prettyUrl() );
}


void ProtocolHandler::slotMostLocalUrlResult(KJob *job)
{
    if(job->error())
    {
        // TODO
        kDebug() << "error";
    }
    else
    {
        KIO::StatJob *statJob = static_cast<KIO::StatJob*>(job);
        KIO::UDSEntry entry = statJob->statResult();
        if( entry.isDir() )
            _lister->openUrl(_url);
        else
            emit downloadUrl(_url);
    }
}
