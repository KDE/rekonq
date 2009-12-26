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
#include "newtabpage.h"

// Auto Includes
#include "rekonq.h"

// Local Includes
#include "historymodels.h"
#include "bookmarksmanager.h"
#include "application.h"
#include "mainwindow.h"
#include "mainview.h"
#include "previewchooser.h"

// KDE Includes
#include <KStandardDirs>
#include <KIconLoader>
#include <KDebug>
#include <KConfig>
#include <KConfigGroup>
#include <KDialog>

// Qt Includes
#include <QFile>
#include <websnap.h>


NewTabPage::NewTabPage(QWebFrame *frame)
    : m_root(frame->documentElement())
{
    QString htmlFilePath = KStandardDirs::locate("data", "rekonq/htmls/home.html");
    
    QFile file(htmlFilePath);
    bool isOpened = file.open(QIODevice::ReadOnly);
    if (!isOpened)
        kWarning() << "Couldn't open the home.html file";
    
    QString imagesPath = QString("file://") + KGlobal::dirs()->findResourceDir("data", "rekonq/pics/bg.png") + QString("rekonq/pics");
    
    m_html = file.readAll();
    m_html.replace(QString("%2"), imagesPath);
}


NewTabPage::~NewTabPage()
{
}


void NewTabPage::generate(const KUrl &url)
{    
    if(KUrl("about:preview").isParentOf(url))
    {
        if(url.directory() == QString("preview/remove"))
        {
            removePreview(url.fileName().toInt());
            return;
        }
        if(url.directory() == QString("preview/modify"))
        {
            PreviewChooser *pc = new PreviewChooser(url.fileName().toInt());
            connect(pc, SIGNAL(urlChoosed(int,KUrl)), SLOT(setPreview(int,KUrl)));
            pc->show();
            return;
        }
    }
    
    
    QWebPage *page = m_root.webFrame()->page();
    page->mainFrame()->setHtml(m_html);

    m_root = page->mainFrame()->documentElement().findFirst("#content");
    
    browsingMenu(url);
    
    QString title;
    if(url == KUrl("about:home") || url == KUrl("about:favorites"))
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
    
    m_root.document().findFirst("title").setPlainText(title);
}


void NewTabPage::favoritesPage()
{
    QStringList names = ReKonfig::previewNames();
    QStringList urls = ReKonfig::previewUrls();

    m_root.addClass("favorites");
    
    for(int i=0; i<8; ++i)
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
    prev.findFirst("span").appendInside(i18n("Set a Preview..."));
    prev.findFirst("a").setAttribute("href", QString("about:preview/modify/" + QVariant(index).toString()));
    
    setupPreview(prev, index);
    hideControls(prev);
    
    return prev;
}


QWebElement NewTabPage::loadingPreview(int index, KUrl url)
{
    QWebElement prev = markup(".thumbnail");
    
    prev.findFirst(".preview img").setAttribute("src" , 
                QString("file:///") + KStandardDirs::locate("appdata", "pics/busywidget.gif"));
    prev.findFirst("span").appendInside(i18n("Loading Preview..."));
    
    setupPreview(prev, index);
    showControls(prev);
    
    WebSnap *snap = new WebSnap(url);
    snap->SetData(QVariant(index));
    connect(snap, SIGNAL(finished()), SLOT(snapFinished()));
    
    return prev;
}

QWebElement NewTabPage::validPreview(int index, KUrl url, QString title)
{
    QWebElement prev = markup(".thumbnail");
    KUrl previewPath = WebSnap::fileForUrl(url);
    QString iString = QVariant(index).toString();
    
    prev.findFirst(".preview img").setAttribute("src" , previewPath.toMimeDataString());
    prev.findFirst("a").setAttribute("href", url.toMimeDataString());
    prev.findFirst("span a").setAttribute("href", url.toMimeDataString());
    prev.findFirst("span").setPlainText(checkTitle(title));
    
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
    e.findFirst(".modify img").setAttribute("src", QString("file:///") +
    KIconLoader::global()->iconPath("insert-image", KIconLoader::DefaultState));
    
    e.findFirst(".modify").setAttribute("href", QString("about:preview/modify/" + QVariant(index).toString()));
    e.findFirst(".remove").setAttribute("href", QString("about:preview/remove/" + QVariant(index).toString()));
    
    e.setAttribute("id", "preview" + QVariant(index).toString());
}


void NewTabPage::snapFinished()
{
    WebSnap *snap = qobject_cast<WebSnap*>(sender());
    QWebElement prev = m_root.findFirst("#preview" + snap->data().toString());
    prev.replace(validPreview(snap->data().toInt(), snap->snapUrl(), snap->snapTitle()));
    
    // Save the new config
    QStringList names = ReKonfig::previewNames();
    QStringList urls = ReKonfig::previewUrls();
    
    // stripTrailingSlash to be sure to get the same string for same address
    urls.replace(snap->data().toInt(), snap->snapUrl().toString(QUrl::StripTrailingSlash));
    names.replace(snap->data().toInt() , snap->snapTitle());
    
    ReKonfig::setPreviewNames(names);
    ReKonfig::setPreviewUrls(urls);
    
    ReKonfig::self()->writeConfig();
}


void NewTabPage::removePreview(int index)
{
    QWebElement prev = m_root.findFirst("#preview" + QVariant(index).toString());
    
    
    QStringList names = ReKonfig::previewNames();
    QStringList urls = ReKonfig::previewUrls();
    
    urls.replace(index, QString(""));
    names.replace(index, QString(""));
    
    ReKonfig::setPreviewNames(names);
    ReKonfig::setPreviewUrls(urls);
    
    // sync file data
    ReKonfig::self()->writeConfig();
    
    prev.replace(emptyPreview(index));
}


void NewTabPage::setPreview(int index, KUrl url)
{
    if(url.isEmpty())
        return;
    
    QWebElement prev = m_root.findFirst("#preview" + QVariant(index).toString());
    
    QStringList urls = ReKonfig::previewUrls();
    urls.replace(index, url.toMimeDataString());
    ReKonfig::setPreviewUrls(urls);
    ReKonfig::self()->writeConfig();

    prev.replace(loadingPreview(index, url));

    
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


QString NewTabPage::checkTitle(QString title)
{
    if(title.length() > 23)
    {
        title.truncate(20);
        title += "...";
    }
    return title;
}
