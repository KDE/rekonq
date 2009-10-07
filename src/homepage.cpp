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


QString HomePage::rekonqHomePage(const KUrl &url)
{
    QFile file(m_homePagePath);
    bool isOpened = file.open(QIODevice::ReadOnly);
    if (!isOpened)
    {
        kWarning() << "Couldn't open the home.html file";
        return QString("");
    }

    QString imagesPath = QString("file://") + KGlobal::dirs()->findResourceDir("data", "rekonq/pics/bg.png") + QString("rekonq/pics");
    QString menu = homePageMenu(url);
    
    QString speed;
    if(url == KUrl("rekonq:lastSites"))
    {
        speed = lastVisitedSites();
    }
    if(url == KUrl("rekonq:history"))
    {
        speed = fillHistory();
    }
    if(url == KUrl("rekonq:bookmarks"))
    {
        speed = fillBookmarks();
    }
    if(url == KUrl("rekonq:home") || url == KUrl("rekonq:favorites"))
    {
        speed = fillFavorites();
    }
    
    QString html = QString(QLatin1String(file.readAll()))
                        .arg(imagesPath)
                        .arg(menu)
                        .arg(speed)
                        ;
                        
    return html;
}


QString HomePage::fillFavorites()
{
    QStringList names = ReKonfig::previewNames();
    QStringList urls = ReKonfig::previewUrls();

    QString speed;
    for(int i=0; i<8; ++i)
    {
        QString text = names.at(i);
        if(text.length() > 20)
        {
            text.truncate(17);
            text += "...";
        }
        speed += "<div class=\"thumbnail\">";
        speed += "<object type=\"application/image-preview\" data=\"";
        speed += urls.at(i) + "\" width=\"200\">";
        speed += "</object>";
        speed += "<br /><br />";
        speed += "<a href=\"" + urls.at(i) + "\">" + text + "</a></div>";
    }
    
    return speed;
}


QString HomePage::lastVisitedSites()
{
    HistoryTreeModel *model = Application::historyManager()->historyTreeModel();
    
    QString last;
    int sites = 0;
    int i = 0;
    do
    {
        QModelIndex index = model->index(i, 0, QModelIndex() );
        if(model->hasChildren(index))
        {
            for(int j=0; j< model->rowCount(index) && sites<8; ++j)
            {
                QModelIndex son = model->index(j, 0, index );

                QString text = son.data().toString();
                if(text.length() > 20)
                {
                    text.truncate(17);
                    text += "...";
                }
                last += "<div class=\"thumbnail\">";
                last += "<object type=\"application/image-preview\" data=\"" + son.data(HistoryModel::UrlStringRole).toString();
                last +=  "\" width=\"200\">";
                last += "</object>";
                last += "<br /><br />";
                last += "<a href=\"" + son.data(HistoryModel::UrlStringRole).toString() + "\">" + text + "</a></div>";
                sites++;
            }
        }
        i++;
    }
    while( sites<8 || model->hasIndex( i , 0 , QModelIndex() ) );

    return last;

}


QString HomePage::homePageMenu(KUrl currentUrl)
{
    QString menu = "";
    
    KIconLoader *loader = KIconLoader::global();
    
    menu += "<div class=\"link";
    if(currentUrl == "rekonq:favorites" || currentUrl == "rekonq:home")
        menu += " current";
    menu += "\"><a href=\"rekonq:favorites\">";
    menu += "<img src=\"file:///" + loader->iconPath("rating", KIconLoader::Desktop) + "\" />";
    menu += "Favorites</a></div>";
    
    menu += "<div class=\"link";
    if(currentUrl == "rekonq:lastSites")
        menu += " current";
    menu += "\"><a href=\"rekonq:lastSites\">";
    menu += "<img src=\"file:///" + loader->iconPath("edit-undo", KIconLoader::Desktop) + "\" />";
    menu += "Last Visited</a></div>";
    
    menu += "<div class=\"link";
    if(currentUrl == "rekonq:bookmarks")
        menu += " current";
    menu += "\"><a href=\"rekonq:bookmarks\">";
    menu += "<img src=\"file:///" + loader->iconPath("bookmarks-organize", KIconLoader::Desktop) + "\" />";
    menu += "Bookmarks</a></div>";
    
    menu += "<div class=\"link";
    if(currentUrl == "rekonq:history")
        menu += " current";
    menu += "\"><a href=\"rekonq:history\">";
    menu += "<img src=\"file:///" + loader->iconPath("view-history", KIconLoader::Desktop) + "\" />";
    menu += "History</a></div>";
    
    
    return menu;
}


QString HomePage::fillHistory()
{
    HistoryTreeModel *model = Application::historyManager()->historyTreeModel();
    
    QString history;
    int i = 0;
    do
    {
        QModelIndex index = model->index(i, 0, QModelIndex() );
        if(model->hasChildren(index))
        {
            history += "<h3>" + index.data().toString() + "</h3>";
            for(int j=0; j< model->rowCount(index); ++j)
            {
                QModelIndex son = model->index(j, 0, index );
                history += son.data(HistoryModel::DateTimeRole).toDateTime().toString("hh:mm");
                history += " ";
                history += QString("<a href=\"") + son.data(HistoryModel::UrlStringRole).toString() + QString("\">") + 
                        son.data().toString() + QString("</a>");
                history += "<br />";
            }
        }
        i++;
    }
    while( model->hasIndex( i , 0 , QModelIndex() ) );

    history += "</table>";
    return history;
}


QString HomePage::fillBookmarks()
{
    KBookmarkGroup bookGroup = Application::bookmarkProvider()->rootGroup();
    if (bookGroup.isNull())
    {
        return QString("Error retrieving bookmarks!");
    }

    QString str;
    KBookmark bookmark = bookGroup.first();
    while (!bookmark.isNull())
    {
        str += createBookItem(bookmark);
        bookmark = bookGroup.next(bookmark);
    }
    return str;
}


QString HomePage::createBookItem(const KBookmark &bookmark)
{
    if (bookmark.isGroup())
    {
        QString result = QString("");
        KBookmarkGroup group = bookmark.toGroup();
        KBookmark bm = group.first();
        result += "<h3>" + bookmark.text() + "</h3>";
        result += "<p class=\"bookfolder\">";
        while (!bm.isNull())
        {
            result += createBookItem(bm);
            bm = group.next(bm);
        }
        result += "</p>";
        return result;
    }
 
    if(bookmark.isSeparator())
    {
        return QString("<hr />");
    }
    
    QString books = " ";
    books += "<a href=\"" + bookmark.url().prettyUrl() + "\">" + bookmark.text() + "</a><br />";
    return books;
}
