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

// Local Includes
#include "historymodels.h"
#include "bookmarks.h"
#include "application.h"
#include "mainwindow.h"
#include "mainview.h"
#include "websnap.h"

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
    QString closedtabs = recentlyClosedTabs();
    

    QString html = QString(QLatin1String(file.readAll()))
                        .arg(search)
                        .arg(closedtabs)
                        .arg(speed)
                        ;

    return html;
}


QString HomePage::speedDial()
{
    KUrl::List ul ;
    ul << KUrl("http://www.google.com") << KUrl("http://www.kde.org") << KUrl("http://sourceforge.net")
    << KUrl("http://www.slacky.eu") << KUrl("http://kde-apps.org") << KUrl("http://www.kernel.org") 
    << KUrl("http://it.wikipedia.org") << KUrl("http://www.adjam.org") << KUrl("http://wordpress.com");
    
    QString speed = QString();
    for(int i = 0; i< ul.count(); ++i)
    {
        KUrl url = ul.at(i);
        QString fileName = QString("thumb") + QString::number(i) + QString(".png");
        QString path = KStandardDirs::locateLocal("cache", QString("thumbs/") + fileName, true);
        if( !QFile::exists(path) )
        {
            kDebug() << "websnap";
            WebSnap *ws = new WebSnap(url, fileName);
        }
        
        speed += "<div class=\"thumbnail\">";
        speed += "<a href=\"" + url.prettyUrl() + "\">";
        speed += "<img src=\"" + path + "\" width=\"200\" alt=\"" + url.prettyUrl() + "\" />";
        speed += "<br />";
        speed += url.prettyUrl() + "</a></div>";
    }
    return speed;
}


QString HomePage::searchEngines()
{
    QString engines = "<h2>Search Engines</h2>";
    
//     KConfig config("kuriikwsfilterrc"); //Share with konqueror
//     KConfigGroup cg = config.group("General");
//     QStringList favoriteEngines;
//     favoriteEngines << "google" << "wikipedia"; //defaults
//     favoriteEngines = cg.readEntry("FavoriteSearchEngines", favoriteEngines);
// 
//     foreach (const QString &engine, favoriteEngines)
//     {
//         if(!engine.isEmpty())
//         {
//             engines += engine + ": <input type=\"text\" name=\"" + engine + "\" /><br />";
//         }
//     }

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

    KUrl::List links = Application::instance()->mainWindow()->mainView()->recentlyClosedTabs();
    
    foreach(const KUrl &url, links)
    {
        closed += "<a href=\"" + url.prettyUrl() + "\">" + url.prettyUrl() + "</a><br />";
    }
    return closed;
}
