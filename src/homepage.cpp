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

// KDE Includes
#include <KStandardDirs>
#include <KDebug>

// Qt Includes
#include <QFile>


HomePage::HomePage(QObject *parent)
    : QObject(parent)
{
    m_homePagePath = KStandardDirs::locate("data", "rekonq/htmls/home.html");
    m_imagesPath = "file://" + KStandardDirs::locate("appdata", "pics/");
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

    QString history = fillHistory();

    QString bookmarks = fillBookmarks();

    QString html = QString(QLatin1String(file.readAll()))
                        .arg(m_imagesPath)
                        .arg(history)
                        .arg(bookmarks);

    return html;
}


QString HomePage::fillHistory()
{
    QString history = QString();
    HistoryTreeModel *model = Application::historyManager()->historyTreeModel();
    
    int i = 0;
    do
    {
        QModelIndex index = model->index(i, 0, QModelIndex() );
        if(model->hasChildren(index))
        {
            QString s = QString::number(i);
            history += createSubMenu(index.data().toString(), s);
            history += "<p id=\"y" + s + "\" class=\"indent\" style=\"display:none\">";
            for(int j=0; j< model->rowCount(index); ++j)
            {
                QModelIndex son = model->index(j, 0, index );
                history += QString("<a href=\"") + son.data(HistoryModel::UrlStringRole).toString() + QString("\">") + 
                        son.data().toString() + QString("</a><br />");
            }
            history += "</p>";
        }
        else
        {
            history += QString("<p> NO CHILDREN: ") + index.data().toString() + QString("</p>");
        }
        i++;
    }
    while( model->hasIndex( i , 0 , QModelIndex() ) );

    return history;
    
}


QString HomePage::fillBookmarks()
{
    KBookmarkGroup toolBarGroup = Application::bookmarkProvider()->rootGroup();
    if (toolBarGroup.isNull())
    {
        return QString("Error retrieving bookmarks!");
    }

    QString str = QString("");
    KBookmark bookmark = toolBarGroup.first();
    while (!bookmark.isNull())
    {
        str += createBookItem(bookmark);
        bookmark = toolBarGroup.next(bookmark);
    }
    
    return str;
}


QString HomePage::createSubMenu(const QString &item, const QString &s)
{
    QString menu = "<div onClick=\"ToggleVisibility('x" + s + "','y" + s + "')\">";

    menu += "<p><img id=\"x" + s + "\" src=\"" + m_imagesPath + "closed.png\" /> <b><u>" + item + "</u></b></p></div>";
    return menu;
}


QString HomePage::createBookItem(const KBookmark &bookmark)
{
    static int i = 0;
    
    if (bookmark.isGroup())
    {
        QString result = QString("");
        QString ss = "b" + QString::number(i);
        i++;
        
        KBookmarkGroup group = bookmark.toGroup();
        KBookmark bm = group.first();
        result += createSubMenu( bookmark.text() , ss );
        result += "<p id=\"y" + ss + "\" class=\"indent\" style=\"display:none\">";

        while (!bm.isNull())
        {
            result += createBookItem(bm);    //menuAction->addAction(fillBookmarkBar(bm));
            bm = group.next(bm);
        }
        result += "</p>";
        return result;
    }
 
    if(bookmark.isSeparator())
    {
        return QString("<hr />");
    }
    return "<a href=\"" + bookmark.url().prettyUrl() + "\">" + bookmark.text() + "</a><br />";
}
