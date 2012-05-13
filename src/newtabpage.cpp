/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2009-2012 by Andrea Diamantini <adjam7 at gmail dot com>
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
#include "application.h"
#include "bookmarkmanager.h"
#include "downloadmanager.h"
#include "iconmanager.h"
#include "historymanager.h"
#include "historymodels.h"
#include "mainview.h"
#include "mainwindow.h"
#include "previewselectorbar.h"
#include "thumbupdater.h"
#include "urlfilterproxymodel.h"
#include "websnap.h"
#include "webpage.h"
#include "webtab.h"

// KDE Includes
#include <KBookmarkManager>
#include <KIconLoader>
#include <KLocale>
#include <KMimeType>
#include <KStandardDirs>

// Qt Includes
#include <QFile>
#include <QAction>
#include <QWebFrame>


NewTabPage::NewTabPage(QWebFrame *frame)
    : QObject(frame)
    , m_root(frame->documentElement())
    , m_showFullHistory(false)
{
    QString htmlFilePath = KStandardDirs::locate("data", "rekonq/htmls/home.html");
    QString dataPath = QL1S("file://") + htmlFilePath;
    dataPath.remove(QL1S("/htmls/home.html"));
    
    QFile file(htmlFilePath);
    bool isOpened = file.open(QIODevice::ReadOnly);
    if (!isOpened)
    {
        kDebug() << "Couldn't open the home.html file";
    }
    else
    {
        m_html = file.readAll();
        m_html.replace(QL1S("%2"), dataPath);
    }
}


void NewTabPage::generate(const KUrl &url)
{
    // about:preview links
    if (KUrl("about:preview").isParentOf(url))
    {
        if (url.fileName() == QL1S("add"))
        {
            QStringList names = ReKonfig::previewNames();
            QStringList urls = ReKonfig::previewUrls();

            int index = urls.count();

            names.append("");
            urls.append("");

            ReKonfig::setPreviewNames(names);
            ReKonfig::setPreviewUrls(urls);

            loadPageForUrl(KUrl("about:favorites"));

            rApp->mainWindow()->currentTab()->createPreviewSelectorBar(index);
            return;
        }

        if (url.directory() == QL1S("preview/remove"))
        {
            int index = url.fileName().toInt();
            removePreview(index);
            return;
        }

        if (url.directory() == QL1S("preview/modify"))
        {
            int index = url.fileName().toInt();
            rApp->mainWindow()->currentTab()->createPreviewSelectorBar(index);
            return;
        }

        if (url.directory() == QL1S("preview/reload"))
        {
            int index = url.fileName().toInt();
            reloadPreview(index);
            return;
        }
    }

    // about:tabs links
    if (KUrl("about:tabs").isParentOf(url))
    {
        if (url.fileName() == QL1S("show"))
        {
            const int winIndex = url.queryItem(QL1S("win")).toInt();
            const int tabIndex = url.queryItem(QL1S("tab")).toInt();

            MainWindow *w = rApp->mainWindowList().at(winIndex).data();

            // close about:tabs tab
            rApp->mainWindow()->mainView()->closeTab(rApp->mainWindow()->mainView()->currentIndex());

            // show requested tab
            w->mainView()->setCurrentIndex(tabIndex);
            if (w != rApp->mainWindow())
                w->raise();
            return;
        }

        if (url.fileName() == QL1S("remove"))
        {
            const int winIndex = url.queryItem(QL1S("win")).toInt();
            const int tabIndex = url.queryItem(QL1S("tab")).toInt();

            MainWindow *w = rApp->mainWindowList().at(winIndex).data();
            w->mainView()->closeTab(tabIndex);
            loadPageForUrl(KUrl("about:tabs"));
            return;
        }
    }

    // about:closedTabs links
    if (KUrl("about:closedTabs").isParentOf(url))
    {
        if (url.fileName() == QL1S("restore"))
        {
            const int tabIndex = url.queryItem(QL1S("tab")).toInt();

            rApp->mainWindow()->mainView()->restoreClosedTab(tabIndex, false);
            return;
        }
    }

    // about:history links
    if (KUrl("about:history").isParentOf(url))
    {
        if (url.fileName() == QL1S("clear"))
        {
            rApp->historyManager()->clear();
            loadPageForUrl(KUrl("about:history"));
            return;
        }

        if (url.fileName() == QL1S("showAllItems"))
        {
            m_showFullHistory = true;
            loadPageForUrl(KUrl("about:history"));
            return;
        }

        if (url.fileName() == QL1S("search"))
        {
            QString value = url.queryItemValue( QL1S("q") );
            loadPageForUrl(KUrl("about:history"), value);
            return;
        }

    }

    // about:downloads links
    if (KUrl("about:downloads").isParentOf(url))
    {
        if (url.fileName() == QL1S("clear"))
        {
            rApp->downloadManager()->clearDownloadsHistory();
            loadPageForUrl(KUrl("about:downloads"));
            return;
        }

        if (url.fileName() == QL1S("search"))
        {
            QString value = url.queryItemValue( QL1S("q") );
            loadPageForUrl(KUrl("about:downloads"), value);
            return;
        }

    }
    
    if (url == KUrl("about:bookmarks/edit"))
    {
        rApp->bookmarkManager()->slotEditBookmarks();
        return;
    }

    loadPageForUrl(url);
}


void NewTabPage::loadPageForUrl(const KUrl &url, const QString & filter)
{
    // webFrame can be null. See bug:282092
    QWebFrame *parentFrame = qobject_cast<QWebFrame *>(parent());
    if (!parentFrame)
    {
        kDebug() << "NULL PARENT FRAME: PAGE NOT LOADED";
        return;
    }

    parentFrame->setHtml(m_html);

    m_root = parentFrame->documentElement().findFirst(QL1S("#content"));

    browsingMenu(url);

    QString title;
    QByteArray encodedUrl = url.toEncoded();
    if (encodedUrl == QByteArray("about:favorites"))
    {
        favoritesPage();
        updateWindowIcon();
        title = i18n("Favorites");
    }
    else if (encodedUrl == QByteArray("about:closedTabs"))
    {
        closedTabsPage();
        updateWindowIcon();
        title = i18n("Closed Tabs");
    }
    else if (encodedUrl == QByteArray("about:history"))
    {
        historyPage(filter);
        updateWindowIcon();
        title = i18n("History");
    }
    else if (encodedUrl == QByteArray("about:bookmarks"))
    {
        bookmarksPage();
        updateWindowIcon();
        title = i18n("Bookmarks");
    }
    else if (encodedUrl == QByteArray("about:downloads"))
    {
        downloadsPage(filter);
        updateWindowIcon();
        title = i18n("Downloads");
    }
    else if (encodedUrl == QByteArray("about:tabs"))
    {
        tabsPage();
        updateWindowIcon();
        title = i18n("Tabs");
    }

    m_root.document().findFirst(QL1S("title")).setPlainText(title);
}


// ----------------------------------------------------------------------------
// HIGH-LEVEL FUNCTIONS


void NewTabPage::browsingMenu(const KUrl &currentUrl)
{
    QList<QWebElement> navItems;

    // Favorites
    navItems.append(createLinkItem(i18n("Favorites"),
                                   QL1S("about:favorites"),
                                   QL1S("emblem-favorite"),
                                   KIconLoader::Toolbar));

    // Closed Tabs
    navItems.append(createLinkItem(i18n("Closed Tabs"),
                                   QL1S("about:closedTabs"),
                                   QL1S("tab-close"),
                                   KIconLoader::Toolbar));

    // Bookmarks
    navItems.append(createLinkItem(i18n("Bookmarks"),
                                   QL1S("about:bookmarks"),
                                   QL1S("bookmarks"),
                                   KIconLoader::Toolbar));

    // History
    navItems.append(createLinkItem(i18n("History"),
                                   QL1S("about:history"),
                                   QL1S("view-history"),
                                   KIconLoader::Toolbar));

    // Downloads
    navItems.append(createLinkItem(i18n("Downloads"),
                                   QL1S("about:downloads"),
                                   QL1S("download"),
                                   KIconLoader::Toolbar));

    // Tabs
    navItems.append(createLinkItem(i18n("Tabs"),
                                   QL1S("about:tabs"),
                                   QL1S("tab-duplicate"),
                                   KIconLoader::Toolbar));

    Q_FOREACH(QWebElement it, navItems)
    {
        const QString aTagString(QL1C('a'));
        const QString hrefAttributeString(QL1S("href"));

        if (it.findFirst(aTagString).attribute(hrefAttributeString) == currentUrl.toMimeDataString())
            it.addClass(QL1S("current"));
        else if (currentUrl == QL1S("about:home") && it.findFirst(aTagString).attribute(hrefAttributeString) == QL1S("about:favorites"))
            it.addClass(QL1S("current"));
        m_root.document().findFirst(QL1S("#navigation")).appendInside(it);
    }
}


void NewTabPage::favoritesPage()
{
    m_root.addClass(QL1S("favorites"));

    const QWebElement add = createLinkItem(i18n("Add Favorite"),
                                           QL1S("about:preview/add"),
                                           QL1S("list-add"),
                                           KIconLoader::Toolbar);
    m_root.document().findFirst("#actions").appendInside(add);
    
    QStringList names = ReKonfig::previewNames();
    QStringList urls = ReKonfig::previewUrls();

    if (urls.isEmpty())
    {
        m_root.addClass(QL1S("empty"));
        m_root.setPlainText(i18n("You can add a favorite by clicking the \"Add Favorite\" button in the top-right corner of this page"));
        return;
    }

    for (int i = 0; i < urls.count() ; ++i)
    {
        KUrl url = KUrl(urls.at(i));

        QWebElement prev = url.isEmpty()
                           ? emptyPreview(i)
                           : validPreview(i, url, QString::number(i + 1) + " - " + names.at(i));

        m_root.appendInside(prev);
    }
}


void NewTabPage::historyPage(const QString & filter)
{
    m_root.addClass(QL1S("history"));

    const QWebElement searchForm = createFormItem(i18n("Search History"), QL1S("about:history/search"));
    m_root.document().findFirst(QL1S("#actions")).appendInside(searchForm);
    
    const QWebElement clearHistory = createLinkItem(i18n("Clear History"),
                                     QL1S("about:history/clear"),
                                     QL1S("edit-clear"),
                                     KIconLoader::Toolbar);
    m_root.document().findFirst(QL1S("#actions")).appendInside(clearHistory);

    HistoryTreeModel *model = rApp->historyManager()->historyTreeModel();
    UrlFilterProxyModel *proxy = new UrlFilterProxyModel(this);
    proxy->setSourceModel(model);

    bool filterIsEmpty = filter.isEmpty();
    
    if (!filterIsEmpty)
        proxy->setFilterFixedString(filter);

    if (proxy->rowCount() == 0)
    {
        if (filterIsEmpty)
        {
            m_root.addClass(QL1S("empty"));
            m_root.setPlainText(i18n("Your browsing history is empty"));
        }
        else
        {
            m_root.addClass(QL1S("empty"));
            m_root.setPlainText(i18n("No matches for string %1 in history", filter));
        }
        return;
    }

    int i = 0;
    const int maxTextSize = 103;
    const int truncateSize = 100;
    do
    {
        QModelIndex index = proxy->index(i, 0, QModelIndex());
        if (proxy->hasChildren(index))
        {
            m_root.appendInside(markup(QL1S("h3")));
            m_root.lastChild().setPlainText(index.data().toString());

            m_root.appendInside(markup(QL1S(".historyfolder")));
            QWebElement little = m_root.lastChild();
            for (int j = 0; j < proxy->rowCount(index); ++j)
            {
                QModelIndex son = proxy->index(j, 0, index);
                KUrl u = son.data(HistoryModel::UrlStringRole).toUrl();

                little.appendInside(son.data(HistoryModel::DateTimeRole).toDateTime().toString("hh:mm"));
                little.appendInside(QL1S("&nbsp;&nbsp;"));
                little.appendInside(markup(QL1S("img")));
                little.lastChild().setAttribute(QL1S("src"), rApp->iconManager()->iconPathForUrl(u));
                little.lastChild().setAttribute(QL1S("width"), QL1S("16"));
                little.lastChild().setAttribute(QL1S("height"), QL1S("16"));
                little.appendInside(QL1S("&nbsp;&nbsp;"));
                little.appendInside(markup(QL1S("a")));
                little.lastChild().setAttribute(QL1S("href") , u.url());

                QString shownUrl = son.data().toString();
                if (shownUrl.length() > maxTextSize)
                {
                    shownUrl.truncate(truncateSize);
                    shownUrl += QL1S("...");
                }
                little.lastChild().appendInside(shownUrl);

                little.appendInside(QL1S("<br />"));
            }
        }
        i++;
        if (filterIsEmpty && m_showFullHistory == false && (i == 2))
        {
            m_root.appendInside(markup(QL1S("a")));
            m_root.lastChild().setAttribute(QL1S("class") , QL1S("greybox"));
            m_root.lastChild().setAttribute(QL1S("href") , QL1S("about:history/showAllItems"));
            m_root.lastChild().setPlainText(i18n("Show full History"));
            return;
        }
    }
    while (proxy->hasIndex(i , 0 , QModelIndex()));

    m_showFullHistory = false;
}


void NewTabPage::bookmarksPage()
{
    m_root.addClass(QL1S("bookmarks"));

    const QWebElement editBookmarks = createLinkItem(i18n("Edit Bookmarks"),
                                      QL1S("about:bookmarks/edit"),
                                      QL1S("bookmarks-organize"),
                                      KIconLoader::Toolbar);
    m_root.document().findFirst(QL1S("#actions")).appendInside(editBookmarks);

    KBookmarkGroup bookGroup = rApp->bookmarkManager()->rootGroup();
    if (bookGroup.isNull())
    {
        m_root.addClass(QL1S("empty"));
        m_root.setPlainText(i18n("You have no bookmarks"));
        return;
    }

    KBookmark bookmark = bookGroup.first();

    m_root.appendInside(markup(QL1S(".bookmarkfolder")));
    QWebElement rootFolder = m_root.lastChild();
    rootFolder.appendInside(markup(QL1S("h4")));
    rootFolder.lastChild().setPlainText(i18n("ROOT"));
    
    while (!bookmark.isNull())
    {
        createBookmarkItem(bookmark, rootFolder);
        bookmark = bookGroup.next(bookmark);
    }
}


void NewTabPage::closedTabsPage()
{
    m_root.addClass(QL1S("closedTabs"));

    QList<TabHistory> links = rApp->mainWindow()->mainView()->recentlyClosedTabs();

    if (links.isEmpty())
    {
        m_root.addClass(QL1S("empty"));
        m_root.setPlainText(i18n("There are no recently closed tabs"));
        return;
    }

    for (int i = 0; i < links.count(); ++i)
    {
        TabHistory item = links.at(i);
        QWebElement prev;

        if (item.url.isEmpty())
            continue;

        prev = closedTabPreview(i, item.url, item.title);

        prev.setAttribute(QL1S("id"),  QL1S("preview") + QVariant(i).toString());
        hideControls(prev);
        m_root.appendInside(prev);
    }
}


void NewTabPage::downloadsPage(const QString & filter)
{
    m_root.addClass(QL1S("downloads"));

    const QWebElement searchForm = createFormItem(i18n("Search Downloads"), QL1S("about:downloads/search"));
    m_root.document().findFirst(QL1S("#actions")).appendInside(searchForm);

    const QWebElement clearDownloads = createLinkItem(i18n("Clear Downloads"),
                                       QL1S("about:downloads/clear"),
                                       QL1S("edit-clear"),
                                       KIconLoader::Toolbar);
    m_root.document().findFirst(QL1S("#actions")).appendInside(clearDownloads);

    DownloadList list = rApp->downloadManager()->downloads();

    bool filterIsEmpty = filter.isEmpty();
    
    if (list.isEmpty())
    {
        m_root.addClass(QL1S("empty"));
        m_root.setPlainText(i18n("There are no recently downloaded files to show"));
        return;
    }

    Q_FOREACH(DownloadItem * item, list)
    {
        KUrl u = KUrl(item->destinationUrl());
        QString fName = u.fileName();

        QString srcUrl = item->originUrl();
        
        if (!filterIsEmpty)
        {
            if (!fName.contains(filter, Qt::CaseInsensitive) && !srcUrl.contains(filter, Qt::CaseInsensitive))
                continue;
        }
        
        m_root.prependInside(markup(QL1S("div")));

        QWebElement div = m_root.firstChild();
        div.addClass(QL1S("download"));

        QString dir = u.directory();
        QString file = dir + QL1C('/') + fName;

        KIconLoader *loader = KIconLoader::global();
        QString iconPath = QL1S("file://") + loader->iconPath(KMimeType::iconNameForUrl(u), KIconLoader::Desktop);

        div.appendInside(markup(QL1S("img")));
        div.lastChild().setAttribute(QL1S("src"), iconPath);

        div.appendInside(QL1S("<strong>") + fName +  QL1S("</strong>"));
        div.appendInside(QL1S(" - "));
        QString date = KGlobal::locale()->formatDateTime(item->dateTime(), KLocale::FancyLongDate);
        div.appendInside(QL1S("<em>") + date +  QL1S("</em>"));
        div.appendInside(QL1S("<br />"));

        div.appendInside(QL1S("<a href=") + srcUrl +  QL1C('>') + srcUrl +  QL1S("</a>"));
        div.appendInside(QL1S("<br />"));

        if (QFile::exists(file))
        {
            div.appendInside(markup(QL1S("a")));
            div.lastChild().setAttribute(QL1S("class"), QL1S("greylink"));
            div.lastChild().setAttribute(QL1S("href"), QL1S("file://") + dir);
            div.lastChild().setPlainText(i18n("Open directory"));

            div.appendInside(QL1S(" - "));
            
            div.appendInside(markup(QL1S("a")));
            div.lastChild().setAttribute(QL1S("class"), QL1S("greylink"));
            div.lastChild().setAttribute(QL1S("href"), QL1S("file://") + file);
            div.lastChild().setPlainText(i18n("Open file"));
        }
        else
        {
            div.appendInside(QL1S("<em>") + QL1S("Removed") +  QL1S("</em>"));
        }
    }
}


void NewTabPage::tabsPage()
{
    m_root.addClass(QL1S("tabs"));

    int wins = 0;
    Q_FOREACH(const QWeakPointer<MainWindow> &wPointer, rApp->mainWindowList())
    {
        m_root.appendInside(markup(QL1S("h3")));
        m_root.lastChild().setPlainText("Window");

        MainWindow *w = wPointer.data();

        const int tabCount = w->mainView()->count();
        for (int i = 0; i < tabCount; ++i)
        {
            KUrl url = w->mainView()->webTab(i)->url();

            if (!WebSnap::existsImage(url))
            {
                kDebug() << "image doesn't exist for url: " << url;
                QPixmap preview = WebSnap::renderPagePreview(*w->mainView()->webTab(i)->page());
                QString path = WebSnap::imagePathFromUrl(url.url());
                preview.save(path);
            }
            QString name = w->mainView()->webTab(i)->view()->title();
            QWebElement prev;

            prev = tabPreview(wins, i, url, name);

            m_root.appendInside(prev);
        }

        wins++;
    }
}


// ----------------------------------------------------------------------------
// LOW-LEVEL FUNCTIONS


QWebElement NewTabPage::emptyPreview(int index)
{
    QWebElement prev = markup(QL1S(".thumbnail"));

    prev.findFirst(QL1S(".preview img")).setAttribute(QL1S("src") ,
            QL1S("file:///") + KIconLoader::global()->iconPath("insert-image", KIconLoader::Desktop));
    prev.findFirst(QL1S("span a")).setPlainText(i18n("Set a Preview..."));
    prev.findFirst(QL1S("a")).setAttribute(QL1S("href"),
                                           QL1S("about:preview/modify/") + QVariant(index).toString());

    setupPreview(prev, index);

    return prev;
}


void NewTabPage::reloadPreview(int index)
{
    QString id = QL1S("#preview") + QString::number(index);
    QWebElement thumb = m_root.document().findFirst(id);

    QString urlString = ReKonfig::previewUrls().at(index);
    QString nameString = ReKonfig::previewNames().at(index);

    kDebug() << "URL: " << urlString;
    kDebug() << "NAME: " << nameString;

    ThumbUpdater *t = new ThumbUpdater(thumb, urlString, nameString);
    t->updateThumb();
}


QWebElement NewTabPage::validPreview(int index, const KUrl &url, const QString &title)
{
    QWebElement prev = markup(QL1S(".thumbnail"));

    QString previewPath = WebSnap::existsImage(url)
                          ? QL1S("file://") + WebSnap::imagePathFromUrl(url)
                          : rApp->iconManager()->iconPathForUrl(url)
                          ;

    prev.findFirst(QL1S(".preview img")).setAttribute(QL1S("src") , previewPath);
    prev.findFirst(QL1S("a")).setAttribute(QL1S("href"), url.toMimeDataString());
    prev.findFirst(QL1S("span a")).setAttribute(QL1S("href"), url.toMimeDataString());
    prev.findFirst(QL1S("span a")).setPlainText(checkTitle(title));

    setupPreview(prev, index);
    showControls(prev);
    return prev;
}


QWebElement NewTabPage::tabPreview(int winIndex, int tabIndex, const KUrl &url, const QString &title)
{
    QWebElement prev = markup(QL1S(".thumbnail"));
    QString previewPath = QL1S("file://") + WebSnap::imagePathFromUrl(url);

    QString href = QL1S("about:tabs/show?win=") + QString::number(winIndex) + QL1S("&tab=") + QString::number(tabIndex);

    prev.findFirst(QL1S(".preview img")).setAttribute(QL1S("src") , previewPath);
    prev.findFirst(QL1S("a")).setAttribute(QL1S("href"), href);
    prev.findFirst(QL1S("span a")).setAttribute(QL1S("href"), href);
    prev.findFirst(QL1S("span a")).setPlainText(checkTitle(title));

    setupTabPreview(prev, winIndex, tabIndex);
    prev.findFirst(QL1S(".remove")).setStyleProperty(QL1S("visibility"), QL1S("visible"));
    prev.findFirst(QL1S(".reload")).setStyleProperty(QL1S("visibility"), QL1S("hidden"));
    return prev;
}


QWebElement NewTabPage::closedTabPreview(int index, const KUrl &url, const QString &title)
{
    QWebElement prev = markup(QL1S(".thumbnail"));

    QString previewPath = WebSnap::existsImage(url)
                          ? QL1S("file://") + WebSnap::imagePathFromUrl(url)
                          : rApp->iconManager()->iconPathForUrl(url)
                          ;

    QString href = QL1S("about:closedTabs/restore?tab=") + QString::number(index);

    prev.findFirst(QL1S(".preview img")).setAttribute(QL1S("src") , previewPath);
    prev.findFirst(QL1S("a")).setAttribute(QL1S("href"), href);
    prev.findFirst(QL1S("span a")).setAttribute(QL1S("href"), href);
    prev.findFirst(QL1S("span a")).setPlainText(checkTitle(title));

    setupPreview(prev, index);
    showControls(prev);
    return prev;
}


void NewTabPage::hideControls(QWebElement e)
{
    e.findFirst(QL1S(".remove")).setStyleProperty(QL1S("visibility"), QL1S("hidden"));
    e.findFirst(QL1S(".reload")).setStyleProperty(QL1S("visibility"), QL1S("hidden"));
}


void NewTabPage::showControls(QWebElement e)
{
    e.findFirst(QL1S(".remove")).setStyleProperty(QL1S("visibility"), QL1S("visible"));
    e.findFirst(QL1S(".reload")).setStyleProperty(QL1S("visibility"), QL1S("visible"));
}


void NewTabPage::setupPreview(QWebElement e, int index)
{
    e.findFirst(QL1S(".remove img")).setAttribute(QL1S("src"), QL1S("file:///") + KIconLoader::global()->iconPath("edit-delete", KIconLoader::DefaultState));
    e.findFirst(QL1S(".remove")).setAttribute(QL1S("title"), i18n("Remove favorite"));
    e.findFirst(QL1S(".reload img")).setAttribute(QL1S("src"), QL1S("file:///") + KIconLoader::global()->iconPath("view-refresh", KIconLoader::DefaultState));
    e.findFirst(QL1S(".reload")).setAttribute(QL1S("title"), i18n("Set new favorite"));

    e.findFirst(QL1S(".reload")).setAttribute(QL1S("href"), QL1S("about:preview/reload/") + QVariant(index).toString());
    e.findFirst(QL1S(".remove")).setAttribute(QL1S("href"), QL1S("about:preview/remove/") + QVariant(index).toString());

    e.setAttribute(QL1S("id"), QL1S("preview") + QVariant(index).toString());
}


void NewTabPage::setupTabPreview(QWebElement e, int winIndex, int tabIndex)
{
    e.findFirst(QL1S(".remove img")).setAttribute(QL1S("src"), QL1S("file:///") + KIconLoader::global()->iconPath("edit-delete", KIconLoader::DefaultState));
    e.findFirst(QL1S(".remove")).setAttribute(QL1S("title"), QL1S("Close Tab"));

    QString href = QL1S("about:tabs/remove?win=") + QString::number(winIndex) + QL1S("&tab=") + QString::number(tabIndex);
    e.findFirst(QL1S(".remove")).setAttribute(QL1S("href"), href);

    e.setAttribute(QL1S("id"), QL1S("win") + QString::number(winIndex) + QL1S("tab") + QString::number(tabIndex));
}


void NewTabPage::removePreview(int index)
{
    QStringList names = ReKonfig::previewNames();
    QStringList urls = ReKonfig::previewUrls();

    urls.removeAt(index);
    names.removeAt(index);

    ReKonfig::setPreviewNames(names);
    ReKonfig::setPreviewUrls(urls);

    loadPageForUrl(KUrl("about:favorites"));

    ReKonfig::self()->writeConfig();
}


void NewTabPage::createBookmarkGroup(const KBookmark &bookmark, QWebElement parent)
{
    KBookmarkGroup group = bookmark.toGroup();
    KBookmark bm = group.first();

    parent.appendInside(markup(QL1S(".bookmarkfolder")));
    QWebElement folder = parent.lastChild();
    folder.appendInside(markup(QL1S("h4")));
    folder.lastChild().setPlainText(group.fullText());

    while (!bm.isNull())
    {
        createBookmarkItem(bm, folder);
        bm = group.next(bm);
    }    
}


void NewTabPage::createBookmarkItem(const KBookmark &bookmark, QWebElement parent)
{
    QString cacheDir = QL1S("file://") + KStandardDirs::locateLocal("cache" , "" , true);
    QString icon = QL1S("file://") + KGlobal::dirs()->findResource("icon", "oxygen/16x16/mimetypes/text-html.png");

    if (bookmark.isGroup())
    {
        createBookmarkGroup(bookmark, m_root);
        return;
    }
    else if (bookmark.isSeparator())
    {
        kDebug() << "SEPARATOR";
        parent.appendInside(QL1S("<hr />"));
    }
    else
    {
        QString b = bookmark.icon();
        if (b.contains(QL1S("favicons")))
            icon = cacheDir + bookmark.icon() + QL1S(".png");

        parent.appendInside(markup(QL1S("img")));
        parent.lastChild().setAttribute(QL1S("src") , icon);
        parent.lastChild().setAttribute(QL1S("width") , QL1S("16"));
        parent.lastChild().setAttribute(QL1S("height") , QL1S("16"));
        parent.appendInside(QL1S(" "));
        parent.appendInside(markup(QL1S("a")));
        parent.lastChild().setAttribute(QL1S("href") , bookmark.url().prettyUrl());
        parent.lastChild().setPlainText( checkTitle(bookmark.fullText(), 40) );
        parent.appendInside(QL1S("<br />"));
    }
}


QString NewTabPage::checkTitle(const QString &title, int max)
{
    QString t(title);
    if (t.length() > max)
    {
        t.truncate(max - 3);
        t +=  QL1S("...");
    }
    return t;
}


QWebElement NewTabPage::createLinkItem(const QString &title, const QString &urlString, const QString &iconPath, int groupOrSize) const
{
    const KIconLoader * const loader = KIconLoader::global();

    QWebElement nav = markup(QL1S(".link"));
    nav.findFirst(QL1S("a")).setAttribute(QL1S("href"), urlString);
    nav.findFirst(QL1S("img")).setAttribute(QL1S("src"), QL1S("file://") + loader->iconPath(iconPath, groupOrSize));
    nav.findFirst(QL1S("span")).appendInside(title);
    return nav;
}


QWebElement NewTabPage::createFormItem(const QString &title, const QString &urlString) const
{
    QWebElement form = markup(QL1S("form"));

    form.setAttribute(QL1S("method"), QL1S("GET"));
    form.setAttribute(QL1S("action"), urlString);

    form.appendInside(markup(QL1S("input")));
    form.lastChild().setAttribute(QL1S("type"), QL1S("text"));
    form.lastChild().setAttribute(QL1S("name"), QL1S("q"));
    
    form.appendInside(markup(QL1S("input")));
    form.lastChild().setAttribute(QL1S("type"), QL1S("submit"));
    form.lastChild().setAttribute(QL1S("value"), title);
    
    return form;
}


void NewTabPage::updateWindowIcon()
{
    int currentIndex = rApp->mainWindow()->mainView()->currentIndex();
    rApp->mainWindow()->changeWindowIcon(currentIndex);
}
