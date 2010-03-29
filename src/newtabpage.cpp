/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2009-2010 by Andrea Diamantini <adjam7 at gmail dot com>
* Copyright (C) 2010 by Matthieu Gicquel <matgic78 at gmail dot com>
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
#include "newtabpage.h"
#include "newtabpage.moc"

// Auto Includes
#include "rekonq.h"

// Local Includes
#include "historymodels.h"
#include "bookmarksmanager.h"
#include "application.h"
#include "mainwindow.h"
#include "mainview.h"
#include "websnap.h"
#include "previewselectorbar.h"
#include "webtab.h"

// KDE Includes
#include <KStandardDirs>
#include <KIconLoader>
#include <KDebug>
#include <KConfig>
#include <KConfigGroup>
#include <KDialog>

// Qt Includes
#include <QFile>

// Defines
#define QL1S(x)  QLatin1String(x)


NewTabPage::NewTabPage(QWebFrame *frame)
    : m_root(frame->documentElement())
{
    QString htmlFilePath = KStandardDirs::locate("data", "rekonq/htmls/home.html");
    QString imagesPath = QString("file://") + KGlobal::dirs()->findResourceDir("data", "rekonq/pics/bg.png") + QString("rekonq/pics");    
    
    QFile file(htmlFilePath);
    bool isOpened = file.open(QIODevice::ReadOnly);
    if (!isOpened)
    {
        kDebug() << "Couldn't open the home.html file";
    }
    else
    {
        m_html = file.readAll();
        m_html.replace(QString("%2"), imagesPath);
    }
}


NewTabPage::~NewTabPage()
{
}


void NewTabPage::generate(const KUrl &url)
{
    if(KUrl("about:preview").isParentOf(url))
    {
        if(url.fileName() == QString("add"))
        {
            QStringList names = ReKonfig::previewNames();
            QStringList urls = ReKonfig::previewUrls();
            
            names.append("");
            urls.append("");
            
            ReKonfig::setPreviewNames(names);
            ReKonfig::setPreviewUrls(urls);
            
            // Why doesn't it work well ?
            // m_root.appendInside(emptyPreview(names.length() - 1));
            // Replacing with this :
            generate(KUrl("about:favorites"));
            return;
        }
        if(url.directory() == QString("preview/remove"))
        {
            removePreview(url.fileName().toInt());
            return;
        }
        if(url.directory() == QString("preview/modify"))
        {
            int index = url.fileName().toInt();
            Application::instance()->mainWindow()->currentTab()->createPreviewSelectorBar(index);
            return;
        }
    }
    
    QWebPage *page = m_root.webFrame()->page();
    page->mainFrame()->setHtml(m_html);

    m_root = page->mainFrame()->documentElement().findFirst("#content");
    
    browsingMenu(url);
    
    QString title;
    if(url == KUrl("about:favorites"))
    {
        favoritesPage();
        title = i18n("Favorites");
    }
    else if(url == KUrl("about:closedTabs"))
    {
        closedTabsPage();
        title = i18n("Closed Tabs");
    }
    else if(url == KUrl("about:history"))
    {
        historyPage();
        title = i18n("History");
    }
    else if(url == KUrl("about:bookmarks"))
    {
        bookmarksPage();
        title = i18n("Bookmarks");
    }
    else if(url == KUrl("about:downloads"))
    {
        downloadsPage();
        title = i18n("Downloads");
    }
    
    m_root.document().findFirst("title").setPlainText(title);
}


void NewTabPage::favoritesPage()
{
    m_root.addClass("favorites");
    
    // TODO : create a nice button to replace this ugly link
    m_root.document().findFirst("#navigation").appendOutside("<a href=\"about:preview/add\">Add Preview</a>");
    
    QStringList names = ReKonfig::previewNames();
    QStringList urls = ReKonfig::previewUrls();
    
    for(int i=0; i < urls.count() ; ++i)
    {
        KUrl url = urls.at(i);
        QWebElement prev;
        
        if(url.isEmpty())
            prev = emptyPreview(i);
        else if(!QFile::exists(WebSnap::fileForUrl(url).toLocalFile()))
            prev = loadingPreview(i, url);
        else
            prev = validPreview(i, url, names.at(i));
        
        setupPreview(prev, i);
        
        m_root.appendInside(prev);
    }
}


QWebElement NewTabPage::emptyPreview(int index)
{
    QWebElement prev = markup(".thumbnail");
    
    prev.findFirst(".preview img").setAttribute("src" , QString("file:///") +
                    KIconLoader::global()->iconPath("insert-image", KIconLoader::Desktop));
    prev.findFirst("span a").setPlainText(i18n("Set a Preview..."));
    prev.findFirst("a").setAttribute("href", QString("about:preview/modify/" + QVariant(index).toString()));
    
    setupPreview(prev, index);
    //hideControls(prev);
    
    return prev;
}


QWebElement NewTabPage::loadingPreview(int index, const KUrl &url)
{
    QWebElement prev = markup(".thumbnail");
    
    prev.findFirst(".preview img").setAttribute("src" , 
                QString("file:///") + KStandardDirs::locate("appdata", "pics/busywidget.gif"));
    prev.findFirst("span a").setPlainText(i18n("Loading Preview..."));
    prev.findFirst("a").setAttribute("href", url.toMimeDataString());
    
    setupPreview(prev, index);
    showControls(prev);
    
    new WebSnap(url, m_root.webFrame(), index);
    
    return prev;
}


QWebElement NewTabPage::validPreview(int index, const KUrl &url, const QString &title)
{
    QWebElement prev = markup(".thumbnail");
    KUrl previewPath = WebSnap::fileForUrl(url);
    QString iString = QVariant(index).toString();
    
    prev.findFirst(".preview img").setAttribute("src" , previewPath.toMimeDataString());
    prev.findFirst("a").setAttribute("href", url.toMimeDataString());
    prev.findFirst("span a").setAttribute("href", url.toMimeDataString());
    prev.findFirst("span a").setPlainText(checkTitle(title));
    
    setupPreview(prev, index);
    showControls(prev);
    
    return prev;
}


void NewTabPage::hideControls(QWebElement e)
{
    e.findFirst(".remove").setStyleProperty("visibility", "hidden");
    e.findFirst(".modify").setStyleProperty("visibility", "hidden");
}


void NewTabPage::showControls(QWebElement e)
{
    e.findFirst(".remove").setStyleProperty("visibility", "visible");
    e.findFirst(".modify").setStyleProperty("visibility", "visible");
}


void NewTabPage::setupPreview(QWebElement e, int index)
{
    e.findFirst(".remove img").setAttribute("src", QString("file:///") +
    KIconLoader::global()->iconPath("edit-delete", KIconLoader::DefaultState));
    e.findFirst(".remove").setAttribute("title", "Remove favorite");
    e.findFirst(".modify img").setAttribute("src", QString("file:///") +
    KIconLoader::global()->iconPath("insert-image", KIconLoader::DefaultState));
    e.findFirst(".modify").setAttribute("title", "Set new favorite");
    
    e.findFirst(".modify").setAttribute("href", QString("about:preview/modify/" + QVariant(index).toString()));
    e.findFirst(".remove").setAttribute("href", QString("about:preview/remove/" + QVariant(index).toString()));
    
    e.setAttribute("id", "preview" + QVariant(index).toString());
}


void NewTabPage::snapFinished(int index, const KUrl &url, const QString &title)
{
    // do not try to modify the page if it isn't the newTabPage
    if(m_root.document().findAll("#rekonq-newtabpage").count() == 0)
        return;
    
    QWebElement prev = m_root.findFirst("#preview" + QVariant(index).toString());
    QWebElement newPrev = validPreview(index, url, title);
    
    if(m_root.findAll(".closedTabs").count() != 0)
        hideControls(newPrev);
    
    prev.replace(newPrev);
    
    // update title
    if(m_root.findAll(".favorites").count() != 0)
    {
        QStringList names = ReKonfig::previewNames();
        names.replace(index, title);
        ReKonfig::setPreviewNames(names);
        
        ReKonfig::self()->writeConfig();
    }
}


void NewTabPage::removePreview(int index)
{
    QStringList names = ReKonfig::previewNames();
    QStringList urls = ReKonfig::previewUrls();
    
    if(urls.at(index) == "")
    {
        urls.removeAt(index);
        names.removeAt(index);
        
        // modify config before
        ReKonfig::setPreviewNames(names);
        ReKonfig::setPreviewUrls(urls);
        
        // reload to update index
        generate(KUrl("about:favorites"));
    }
    else
    {
        urls.replace(index, QString(""));
        names.replace(index, QString(""));
        
        QWebElement prev = m_root.findFirst("#preview" + QVariant(index).toString());
        prev.replace(emptyPreview(index));
        
        ReKonfig::setPreviewNames(names);
        ReKonfig::setPreviewUrls(urls);
    }
    
    ReKonfig::self()->writeConfig();
}


void NewTabPage::browsingMenu(const KUrl &currentUrl)
{
    QList<QWebElement> navItems;
    
    KIconLoader *loader = KIconLoader::global();
    
    QWebElement nav = markup(".link"); // Favorites
    nav.findFirst("a").setAttribute("href", "about:favorites");
    nav.findFirst("img").setAttribute("src" , QString("file:///" + 
    loader->iconPath("emblem-favorite", KIconLoader::Desktop ||KIconLoader::SizeSmall)));
    nav.findFirst("a").appendInside(i18n("Favorites"));
    navItems.append(nav);
    
    nav = markup(".link"); // Closed Tabs
    nav.findFirst("a").setAttribute("href", "about:closedTabs");
    nav.findFirst("img").setAttribute("src" , QString("file:///" + 
    loader->iconPath("tab-close", KIconLoader::Desktop ||KIconLoader::SizeSmall)));
    nav.findFirst("a").appendInside(i18n("Closed Tabs"));
    navItems.append(nav);
    
    nav = markup(".link"); // Bookmarks
    nav.findFirst("a").setAttribute("href", "about:bookmarks");
    nav.findFirst("img").setAttribute("src" , QString("file:///" + 
    loader->iconPath("bookmarks", KIconLoader::Desktop ||KIconLoader::SizeSmall)));
    nav.findFirst("a").appendInside(i18n("Bookmarks"));
    navItems.append(nav);
    
    nav = markup(".link"); // History
    nav.findFirst("a").setAttribute("href", "about:history");
    nav.findFirst("img").setAttribute("src" , QString("file:///" + 
    loader->iconPath("view-history", KIconLoader::Desktop ||KIconLoader::SizeSmall)));
    nav.findFirst("a").appendInside(i18n("History"));
    navItems.append(nav);
    
    nav = markup(".link"); // History
    nav.findFirst("a").setAttribute("href", "about:downloads");
    nav.findFirst("img").setAttribute("src" , QString("file:///" + 
    loader->iconPath("download", KIconLoader::Desktop ||KIconLoader::SizeSmall)));
    nav.findFirst("a").appendInside(i18n("Downloads"));
    navItems.append(nav);
    
    QWebElement it;
    foreach(it, navItems)
    {
        if(it.findFirst("a").attribute("href") == currentUrl.toMimeDataString())
            it.addClass("current");
        else if(currentUrl == "about:home" && it.findFirst("a").attribute("href") == "about:favorites")
                it.addClass("current");
        m_root.document().findFirst("#navigation").appendInside(it);
    }
}


void NewTabPage::historyPage()
{
    m_root.addClass("history");
    
    HistoryTreeModel *model = Application::historyManager()->historyTreeModel();
    
    int i = 0;
    do
    {
        QModelIndex index = model->index(i, 0, QModelIndex() );
        if(model->hasChildren(index))
        {
            m_root.appendInside(markup("h3"));
            m_root.lastChild().setPlainText(index.data().toString());
            
            for(int j=0; j< model->rowCount(index); ++j)
            {
                QModelIndex son = model->index(j, 0, index );
                m_root.appendInside(son.data(HistoryModel::DateTimeRole).toDateTime().toString("hh:mm"));
                m_root.appendInside("  ");
                m_root.appendInside(markup("a"));
                m_root.lastChild().setAttribute("href" , son.data(HistoryModel::UrlStringRole).toString());
                m_root.lastChild().appendInside(son.data().toString());
                m_root.appendInside("<br/>");
            }
        }
        i++;
    }
    while( model->hasIndex( i , 0 , QModelIndex() ) );
}


void NewTabPage::bookmarksPage()
{
    m_root.addClass("bookmarks");
    
    KBookmarkGroup bookGroup = Application::bookmarkProvider()->rootGroup();
    if (bookGroup.isNull())
    {
        return;
    }

    KBookmark bookmark = bookGroup.first();
    while (!bookmark.isNull())
    {
        createBookItem(bookmark, m_root);
        bookmark = bookGroup.next(bookmark);
    }
}


void NewTabPage::createBookItem(const KBookmark &bookmark, QWebElement parent)
{
    if (bookmark.isGroup())
    {
        KBookmarkGroup group = bookmark.toGroup();
        KBookmark bm = group.first();
        parent.appendInside(markup("h3"));
        parent.lastChild().setPlainText(group.text());
        parent.appendInside(markup(".bookfolder"));
        while (!bm.isNull())
        {
            createBookItem(bm, parent.lastChild()); // it is .bookfolder
            bm = group.next(bm);
        }
    }
    else if(bookmark.isSeparator())
    {
        parent.appendInside("<hr/>");
    }
    else
    {
        parent.appendInside(markup("a"));
        parent.lastChild().setAttribute("href" , bookmark.url().prettyUrl());
        parent.lastChild().setPlainText(bookmark.text());
        parent.appendInside("<br/>");
    }
}


void NewTabPage::closedTabsPage()
{
    m_root.addClass("closedTabs");
    
    QList<HistoryItem> links = Application::instance()->mainWindow()->mainView()->recentlyClosedTabs();
    
    for(int i=0; i < links.count(); ++i)
    {
        HistoryItem item = links.at(i);
        QWebElement prev;
        
        if(item.url.isEmpty())
            continue;
        else if(!QFile::exists(WebSnap::fileForUrl(item.url).toLocalFile()))
            prev = loadingPreview(i, item.url);
        else
            prev = validPreview(i, item.url, item.title);
        
        prev.setAttribute("id", "preview" + QVariant(i).toString());
        hideControls(prev);
        m_root.appendInside(prev);
    }
}


QString NewTabPage::checkTitle(const QString &title)
{
    QString t(title);
    if(t.length() > 23)
    {
        t.truncate(20);
        t += "...";
    }
    return t;
}


void NewTabPage::downloadsPage()
{
    m_root.addClass("downloads");

    DownloadList list = Application::historyManager()->downloads();
    
    foreach(const DownloadItem &item, list)
    {
        m_root.prependInside(markup("div"));
        
        QWebElement div = m_root.firstChild();
        div.addClass("download");
    
        KUrl u = KUrl(item.destUrlString);
        QString fName = u.fileName();
        QString dir = QL1S("file://") + u.directory();
        
        KIconLoader *loader = KIconLoader::global();
        QString iconPath = "file://" + loader->iconPath(KMimeType::iconNameForUrl(u), KIconLoader::Desktop);
        
        div.appendInside(markup("img"));
        div.lastChild().setAttribute("src", iconPath );
        
        div.appendInside("<strong>" + fName + "</strong>");
        div.appendInside(" - ");
        div.appendInside( item.dateTime.toString("'<em>'dd MMMM yyyy hh:mm'</em>'") );
        div.appendInside("<br/>");
        
        div.appendInside(item.srcUrlString);
        div.appendInside("<br/>");

        div.appendInside(markup("a"));
        div.lastChild().setAttribute("href" , dir);
        div.lastChild().setPlainText("Browse dir");
        
        /* TODO : make this link work
        div.appendInside(" - ");
        div.appendInside(markup("a"));
        div.lastChild().setAttribute("href" , u.toMimeDataString());
        div.lastChild().setPlainText("Open file");*/
    }
}
