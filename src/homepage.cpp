/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2009 by Andrea Diamantini <adjam7 at gmail dot com>
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
#include "homepage.h"
#include "homepage.moc"

// Auto Includes
#include "rekonq.h"

// Local Includes
#include "historymodels.h"
#include "bookmarks.h"
#include "application.h"
#include "mainwindow.h"
#include "mainview.h"

// KDE Includes
#include <KStandardDirs>
#include <KIconLoader>
#include <KDebug>
#include <KConfig>
#include <KConfigGroup>

// Qt Includes
#include <QFile>


HomePage::HomePage(QObject *parent)
    : QObject(parent)
{
    m_homePagePath = KStandardDirs::locate("data", "rekonq/htmls/home.html");
}


HomePage::~HomePage()
{
}


QString HomePage::rekonqHomePage()
{
    QFile file(m_homePagePath);
    bool isOpened = file.open(QIODevice::ReadOnly);
    if (!isOpened)
    {
        kWarning() << "Couldn't open the home.html file";
        return QString("");
    }

    QString speed = speedDial();
    QString search = searchEngines();
    QString lastBlock = ReKonfig::useRecentlyClosedTabs() ? recentlyClosedTabs() : fillRecentHistory(); 
    

    QString html = QString(QLatin1String(file.readAll()))
                        .arg(search)
                        .arg(lastBlock)
                        .arg(speed)
                        ;

    return html;
}


QString HomePage::speedDial()
{
    QStringList names = ReKonfig::previewNames();
    QStringList urls = ReKonfig::previewUrls();
    
    QString speed = QString();
    for(int i = 0; i< urls.count(); ++i)
    {
        QString pageName = QString("page") + QString::number(i);
        
        speed += "<div class=\"thumbnail\">";
        speed += "<a href=\"" + urls.at(i) + "\">";
        speed += "<object type=\"application/image-preview\" width=\"200\">";
        speed += "<param name=\"url\" value=\"" + urls.at(i) + "\">"; 
        speed += "<param name=\"fileName\" value=\"" + pageName + "\">"; 
        speed += "</object>";
        speed += "<br />";
        speed += names.at(i) + "</a></div>";
    }
    return speed;
}


QString HomePage::searchEngines()
{
    QString engines = "<h2>Search Engines</h2>";
    
    // Google search engine
    engines += "<form method=\"get\" action=\"http://www.google.com/search\">";
    engines += "<label for=\"q\">Google:</label>";
    engines += "<input type=\"text\" name=\"q\" />";
    engines += "</form>";
    
    return engines;
}


QString HomePage::recentlyClosedTabs()
{
    QString closed = "<h2>Recently closed tabs</h2>";
    closed += "<ul>";
    
    KUrl::List links = Application::instance()->mainWindow()->mainView()->recentlyClosedTabs();
    
    foreach(const KUrl &url, links)
    {
        closed += "<li><a href=\"" + url.prettyUrl() + "\">" + url.prettyUrl() + "</a></li>";
    }
    
    closed += "</ul>";
    return closed;
}


QString HomePage::fillRecentHistory()
{
    QString history = "<h2>Last 20 visited sites</h2>";
    history += "<ul>";
    
    HistoryTreeModel *model = Application::historyManager()->historyTreeModel();
    
    int i = 0;
    do
    {
        QModelIndex index = model->index(i, 0, QModelIndex() );
        if(model->hasChildren(index))
        {
            for(int j=0; j< model->rowCount(index) && i<20 ; ++j)
            {
                QModelIndex son = model->index(j, 0, index );

                history += "<li>";
                history += QString("<a href=\"") + son.data(HistoryModel::UrlStringRole).toString() + QString("\">");
                history += son.data().toString();
                history += QString("</a>");
                history += "</li>";
                
                i++;
            }
        }
        i++;
    }
    while( i<20 || model->hasIndex( i , 0 , QModelIndex() ) );

    history += "<ul>";
    
    return history;
    
}
