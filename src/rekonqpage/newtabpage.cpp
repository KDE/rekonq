/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2009-2014 by Andrea Diamantini <adjam7 at gmail dot com>
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

// Auto Includes
#include "rekonq.h"

// Local Includes
#include "application.h"
#include "rekonqwindow.h"
#include "tabwidget.h"

#include "previewselectorbar.h"
#include "thumbupdater.h"
#include "websnap.h"
#include "webpage.h"
#include "webtab.h"

#include "tabhistory.h"

#include "bookmarkmanager.h"
#include "downloadmanager.h"
#include "iconmanager.h"

#include "historymanager.h"
#include "historymodels.h"

// KDE Includes
#include <KBookmarkManager>
#include <KIconLoader>
#include <KRun>

// Qt Includes
#include <QAction>
#include <QFile>
#include <QMimeType>
#include <QProcess>
#include <QStandardPaths>
#include <QUrlQuery>
#include <QWebFrame>


NewTabPage::NewTabPage(QWebFrame *frame)
    : QObject(frame)
    , m_root(frame->documentElement())
    , m_showFullHistory(false)
{
    QString htmlFilePath = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QL1S("rekonq/htmls/home.html"));
    QString dataPath = QL1S("file://") + htmlFilePath;
    dataPath.remove(QL1S("/htmls/home.html"));

    QFile file(htmlFilePath);
    bool isOpened = file.open(QIODevice::ReadOnly);
    if (!isOpened)
    {
        qDebug() << "Couldn't open the home.html file";
    }
    else
    {
        m_html = QL1S(file.readAll());
        m_html.replace(QL1S("$DEFAULT_PATH"), dataPath);
        m_html.replace(QL1S("$GENERAL_FONT"), QWebSettings::globalSettings()->fontFamily(QWebSettings::StandardFont));
    }
}


void NewTabPage::generate(const QUrl &url)
{
    QWebFrame *parentFrame = qobject_cast<QWebFrame *>(parent());
    QWebPage *pg = parentFrame->page();
    WebView *view = qobject_cast<WebView *>(pg->parent());
    WebTab *tab = view->parentTab();

    // rekonq:preview links
    if (QUrl( QL1S("rekonq:settings") ).isParentOf(url))
    {
        if (url.fileName() == QL1S("network"))
        {
            QString program = QL1S("kcmshell4");
            QStringList arguments;
            arguments << QL1S("kcm_networkmanagement");
            QProcess *proc = new QProcess(parent());
            proc->start(program, arguments);
            return;
        }

        if (url.fileName() == QL1S("proxy"))
        {
            QString program = QL1S("kcmshell4");
            QStringList arguments;
            arguments << QL1S("proxy");
            QProcess *proc = new QProcess(parent());
            proc->start(program, arguments);
            return;
        }

        if (url.fileName() == QL1S("firewall"))
        {
            QString program = QL1S("kcmshell4");
            QStringList arguments;
            arguments << QL1S("kcm-ufw");
            QProcess *proc = new QProcess(parent());
            proc->start(program, arguments);
            return;
        }
    }

    // rekonq:preview links
    if (QUrl( QL1S("rekonq:preview") ).isParentOf(url))
    {
        if (url.fileName() == QL1S("add"))
        {
            QStringList names = ReKonfig::previewNames();
            QStringList urls = ReKonfig::previewUrls();

            int index = urls.count();

            names.append(QL1S(""));
            urls.append(QL1S(""));

            ReKonfig::setPreviewNames(names);
            ReKonfig::setPreviewUrls(urls);

            loadPageForUrl(QUrl( QL1S("rekonq:favorites") ));

            tab->createPreviewSelectorBar(index);
            return;
        }

        if (url.path().contains(QL1S("remove")))
        {
            int index = url.fileName().toInt();
            removePreview(index);
            return;
        }

        if (url.path().contains(QL1S("modify")))
        {
            int index = url.fileName().toInt();
            tab->createPreviewSelectorBar(index);
            return;
        }

        if (url.path().contains(QL1S("reload")))
        {
            int index = url.fileName().toInt();
            reloadPreview(index);
            return;
        }
    }

    QUrlQuery query(url);
    
    // rekonq:closedtabs links
    if (QUrl( QL1S("rekonq:closedtabs") ).isParentOf(url))
    {
        if (url.fileName() == QL1S("restore"))
        {
            const int tabIndex = query.queryItemValue(QL1S("tab")).toInt();

            rApp->rekonqWindow()->tabWidget()->restoreClosedTab(tabIndex, false);
            return;
        }
    }

    // rekonq:history links
    if (QUrl( QL1S("rekonq:history") ).isParentOf(url))
    {
        if (url.fileName() == QL1S("clear"))
        {
            HistoryManager::self()->clear();
            loadPageForUrl(QUrl( QL1S("rekonq:history") ));
            return;
        }

        if (url.fileName() == QL1S("showAllItems"))
        {
            m_showFullHistory = true;
            loadPageForUrl(QUrl( QL1S("rekonq:history") ));
            return;
        }

        if (url.fileName() == QL1S("search"))
        {
            QString value = query.queryItemValue(QL1S("q"));
            loadPageForUrl(QUrl( QL1S("rekonq:history") ), value);
            return;
        }

        if (url.fileName() == QL1S("remove"))
        {
            int value = query.queryItemValue(QL1S("location")).toInt();
            HistoryManager::self()->removeHistoryLocationEntry(value);
            loadPageForUrl(QUrl( QL1S("rekonq:history") ));
            return;
        }
    }

    // rekonq:downloads links
    if (QUrl( QL1S("rekonq:downloads") ).isParentOf(url))
    {
        if (url.fileName() == QL1S("clear"))
        {
            DownloadManager::self()->clearDownloadsHistory();
            loadPageForUrl(QUrl( QL1S("rekonq:downloads") ));
            return;
        }

        if (url.fileName() == QL1S("search"))
        {
            QString value = query.queryItemValue(QL1S("q"));
            loadPageForUrl(QUrl( QL1S("rekonq:downloads") ), value);
            return;
        }

        if (url.fileName() == QL1S("opendir"))
        {
            QString value = query.queryItemValue(QL1S("q"));
            QUrl dirUrl = QUrl(value);
            (void)new KRun(dirUrl, tab, dirUrl.isLocalFile());
            return;
        }

        if (url.fileName() == QL1S("removeItem"))
        {
            int value = query.queryItemValue(QL1S("item")).toInt();
            DownloadManager::self()->removeDownloadItem(value);
            loadPageForUrl(QUrl( QL1S("rekonq:downloads") ));
            return;
        }
    }

    if (url == QUrl( QL1S("rekonq:bookmarks/edit") ))
    {
        BookmarkManager::self()->slotEditBookmarks();
        return;
    }


    if (url == QUrl( QL1S("rekonq:favorites/save") ))
    {
        saveFavorites();
        return;
    }

    qDebug() << "URL: " << url;
    loadPageForUrl(url);
}


void NewTabPage::loadPageForUrl(const QUrl &url, const QString & filter)
{
    // webFrame can be null. See bug:282092
    QWebFrame *parentFrame = qobject_cast<QWebFrame *>(parent());
    if (!parentFrame)
    {
        qDebug() << "NULL PARENT FRAME: PAGE NOT LOADED";
        return;
    }

    parentFrame->setHtml(m_html);

    m_root = parentFrame->documentElement().findFirst(QL1S("#content"));

    browsingMenu(url);

    QString title;
    QByteArray encodedUrl = url.toEncoded();
    if (encodedUrl == QByteArray("rekonq:favorites"))
    {
        favoritesPage();
//         updateWindowIcon();
        title = i18n("Favorites");
        m_root.document().findFirst(QL1S("title")).setPlainText(title);
        initJS();
        return;
    }
    else if (encodedUrl == QByteArray("rekonq:history"))
    {
        historyPage(filter);
//         updateWindowIcon();
        title = i18n("History");
    }
    else if (encodedUrl == QByteArray("rekonq:bookmarks"))
    {
        bookmarksPage();
//         updateWindowIcon();
        title = i18n("Bookmarks");
    }
    else if (encodedUrl == QByteArray("rekonq:downloads"))
    {
        downloadsPage(filter);
//         updateWindowIcon();
        title = i18n("Downloads");
    }
    else if (encodedUrl == QByteArray("rekonq:closedtabs"))
    {
        closedTabsPage();
//         updateWindowIcon();
        title = i18n("Closed Tabs");
    }

    m_root.document().findFirst(QL1S("title")).setPlainText(title);
}


// ----------------------------------------------------------------------------
// HIGH-LEVEL FUNCTIONS


void NewTabPage::browsingMenu(const QUrl &currentUrl)
{
    QList<QWebElement> navItems;

    // Favorites
    navItems.append(createLinkItem(i18n("Favorites"),
                                   QL1S("rekonq:favorites"),
                                   QL1S("emblem-favorite"),
                                   KIconLoader::Toolbar));

    // Bookmarks
    navItems.append(createLinkItem(i18n("Bookmarks"),
                                   QL1S("rekonq:bookmarks"),
                                   QL1S("bookmarks"),
                                   KIconLoader::Toolbar));

    // History
    navItems.append(createLinkItem(i18n("History"),
                                   QL1S("rekonq:history"),
                                   QL1S("view-history"),
                                   KIconLoader::Toolbar));

    // Downloads
    navItems.append(createLinkItem(i18n("Downloads"),
                                   QL1S("rekonq:downloads"),
                                   QL1S("download"),
                                   KIconLoader::Toolbar));

    // Closed Tabs
    navItems.append(createLinkItem(i18n("Closed Tabs"),
                                   QL1S("rekonq:closedtabs"),
                                   QL1S("tab-close"),
                                   KIconLoader::Toolbar));

    Q_FOREACH(QWebElement it, navItems)
    {
        const QString aTagString(QL1C('a'));
        const QString hrefAttributeString(QL1S("href"));

        if (it.findFirst(aTagString).attribute(hrefAttributeString) == currentUrl.toString())
        {
            it.addClass(QL1S("current"));
        }
        else 
        {
            if (currentUrl == QUrl( QL1S("rekonq:home") ) 
                && it.findFirst(aTagString).attribute(hrefAttributeString) == QL1S("rekonq:favorites"))
            {
                it.addClass(QL1S("current"));
            }
            m_root.document().findFirst(QL1S("#navigation")).appendInside(it);
        }
    }
}


void NewTabPage::favoritesPage()
{
    m_root.addClass(QL1S("favorites"));

    QWebElement add = createLinkItem(i18n("Add Favorite"),
                                     QL1S("rekonq:preview/add"),
                                     QL1S("list-add"),
                                     KIconLoader::Toolbar);
    add.setAttribute(QL1S("class"), QL1S("right"));
    m_root.document().findFirst( QL1S("#actions") ).appendInside(add);

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
        QUrl url = QUrl(urls.at(i));

        QWebElement prev = url.isEmpty()
                           ? emptyPreview(i)
                           : validPreview(i, url, QString::number(i + 1) + QL1S(" - ") + names.at(i));

        m_root.appendInside(prev);
    }
}


void NewTabPage::historyPage(const QString & filter)
{
    m_root.addClass(QL1S("history"));

    QWebElement searchForm = createFormItem(i18n("Search History"), QL1S("rekonq:history/search"));
    searchForm.setAttribute(QL1S("class"), QL1S("left"));

    m_root.document().findFirst(QL1S("#actions")).appendInside(searchForm);

    QWebElement clearHistory = createLinkItem(i18n("Clear History"),
                               QL1S("rekonq:history/clear"),
                               QL1S("edit-clear"),
                               KIconLoader::Toolbar);
    clearHistory.setAttribute(QL1S("class"), QL1S("right"));
    m_root.document().findFirst(QL1S("#actions")).appendInside(clearHistory);

    HistoryTreeModel *model = HistoryManager::self()->historyTreeModel();
    SortFilterProxyModel *proxy = new SortFilterProxyModel(this);
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
    const QString removeIconPath = QL1S("file:///") + KIconLoader::global()->iconPath( QL1S("edit-delete"), KIconLoader::DefaultState);
    QWebElement historyItemElement = markup(QL1S(".historyitem"));
    do
    {
        QModelIndex index = proxy->index(i, 0, QModelIndex());
        if (proxy->hasChildren(index))
        {
            m_root.appendInside(markup(QL1S("h3")));
            m_root.lastChild().setPlainText(index.data().toString());

            m_root.appendInside(markup(QL1S(".historyfolder")));
            QWebElement historyFolderElement = m_root.lastChild();
            for (int j = 0; j < proxy->rowCount(index); ++j)
            {
                QModelIndex son = proxy->index(j, 0, index);
                QUrl u = son.data(HistoryModel::UrlStringRole).toUrl();

                historyFolderElement.appendInside(historyItemElement.clone());
                QWebElement item = historyFolderElement.lastChild();

                item.findFirst(QL1S(".greytext")).setPlainText( son.data(HistoryModel::DateTimeRole).toDateTime().toString( QL1S("hh:mm") ) );

                QWebElement iconElement = item.findFirst(QL1S("img"));
                iconElement.setAttribute(QL1S("src"), IconManager::self()->iconPathForUrl(u));
                iconElement.setAttribute(QL1S("width"), QL1S("16"));
                iconElement.setAttribute(QL1S("height"), QL1S("16"));

                QWebElement linkElement = item.findFirst(QL1S("a"));
                linkElement.setAttribute(QL1S("href") , u.url());
                QString shownUrl = son.data().toString();
                if (shownUrl.length() > maxTextSize)
                {
                    shownUrl.truncate(truncateSize);
                    shownUrl += QL1S("...");
                }
                linkElement.appendInside(shownUrl);

                QWebElement removeElement = item.findFirst(QL1S(".button img"));
                removeElement.setAttribute(QL1S("src"), removeIconPath);

                QWebElement removeLinkElement = item.findFirst(QL1S(".button"));

                int histLoc = HistoryManager::self()->historyFilterModel()->historyLocation(u.url());
                removeLinkElement.setAttribute(QL1S("href"), QL1S("rekonq:history/remove?location=") + QString::number(histLoc));

                historyFolderElement.appendInside(QL1S("<br />"));
            }
        }
        i++;
        if (filterIsEmpty && m_showFullHistory == false && (i == 2))
        {
            m_root.appendInside(markup(QL1S("a")));
            m_root.lastChild().setAttribute(QL1S("class") , QL1S("greybox"));
            m_root.lastChild().setAttribute(QL1S("href") , QL1S("rekonq:history/showAllItems"));
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

    QWebElement editBookmarks = createLinkItem(i18n("Edit Bookmarks"),
                                QL1S("rekonq:bookmarks/edit"),
                                QL1S("bookmarks-organize"),
                                KIconLoader::Toolbar);
    editBookmarks.setAttribute(QL1S("class"), QL1S("right"));
    m_root.document().findFirst(QL1S("#actions")).appendInside(editBookmarks);

    KBookmarkGroup bookGroup = BookmarkManager::self()->rootGroup();
    if (bookGroup.isNull())
    {
        m_root.addClass(QL1S("empty"));
        m_root.setPlainText(i18n("You have no bookmarks"));
        return;
    }

    KBookmark bookmark = bookGroup.first();

    m_root.appendInside(markup(QL1S(".bookmarkfolder")));
    QWebElement rootFolder = m_root.lastChild();
    rootFolder.appendInside(markup(QL1S("a")));
    rootFolder.lastChild().setAttribute(QL1S("href"), QL1S("javascript: toggleChildren(\'Unsorted\')"));
    QWebElement titleElement = rootFolder.lastChild();
    titleElement.appendInside(markup(QL1S("h4")));    
    titleElement.lastChild().setPlainText(i18n("Unsorted"));

    rootFolder.appendInside(markup(QL1S("div")));
    rootFolder.lastChild().setAttribute(QL1S("id"), QL1S("Unsorted"));

    while (!bookmark.isNull())
    {
        createBookmarkItem(bookmark, rootFolder.lastChild());
        bookmark = bookGroup.next(bookmark);
    }
}


void NewTabPage::closedTabsPage()
{
    m_root.addClass(QL1S("closedtabs"));

    QList<TabHistory> links = rApp->rekonqWindow()->tabWidget()->recentlyClosedTabs();

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

        prev = closedTabPreview(i, QUrl(item.url), item.title);

        prev.setAttribute(QL1S("id"),  QL1S("preview") + QVariant(i).toString());

        // hide controls
        prev.findFirst(QL1S(".right")).setStyleProperty(QL1S("visibility"), QL1S("hidden"));
        prev.findFirst(QL1S(".left")).setStyleProperty(QL1S("visibility"), QL1S("hidden"));

        m_root.appendInside(prev);
    }
}


void NewTabPage::downloadsPage(const QString & filter)
{
    m_root.addClass(QL1S("downloads"));

    QWebElement searchForm = createFormItem(i18n("Search Downloads"), QL1S("rekonq:downloads/search"));
    searchForm.setAttribute(QL1S("class"), QL1S("left"));
    m_root.document().findFirst(QL1S("#actions")).appendInside(searchForm);

    QWebElement clearDownloads = createLinkItem(i18n("Clear Downloads"),
                                 QL1S("rekonq:downloads/clear"),
                                 QL1S("edit-clear"),
                                 KIconLoader::Toolbar);
    clearDownloads.setAttribute(QL1S("class"), QL1S("right"));
    m_root.document().findFirst(QL1S("#actions")).appendInside(clearDownloads);

    DownloadList list = DownloadManager::self()->downloads();

    bool filterIsEmpty = filter.isEmpty();

    if (list.isEmpty())
    {
        m_root.addClass(QL1S("empty"));
        m_root.setPlainText(i18n("There are no recently downloaded files to show"));
        return;
    }

    int i = 0;

    Q_FOREACH(DownloadItem * item, list)
    {
        QUrl u = item->destUrl();
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

        QString dir = u.path();
        QString file = dir + QL1C('/') + fName;

        KIconLoader *loader = KIconLoader::global();
        QString iconPath = QL1S("file://") + loader->iconPath(KIO::iconNameForUrl(u), KIconLoader::Desktop);

        div.appendInside(markup(QL1S("img")));
        div.lastChild().setAttribute(QL1S("src"), iconPath);

        div.appendInside(QL1S("<strong>") + fName +  QL1S("</strong>"));
        div.appendInside(QL1S(" - "));
        QString date = item->dateTime().toString();
        div.appendInside(QL1S("<em>") + date +  QL1S("</em>"));
        div.appendInside(QL1S("<br />"));

        div.appendInside(QL1S("<a href=") + srcUrl +  QL1C('>') + srcUrl +  QL1S("</a>"));
        div.appendInside(QL1S("<br />"));

        switch (item->state())
        {
        case DownloadItem::KGetManaged:
            div.appendInside(QL1S("<em>") + i18n("This download is managed by KGet. Check it to grab information about its state") +  QL1S("</em>"));
            break;

        case DownloadItem::Suspended:
            div.appendInside(QL1S("<em>") + i18n("Suspended") +  QL1S("</em>"));
            break;

        case DownloadItem::Downloading:
            div.appendInside(QL1S("<em>") + i18n("Downloading now...") +  QL1S("</em>"));
            break;

        case DownloadItem::Errors:
            div.appendInside(QL1S("<em>") + i18nc("%1 = Error description", "Error: %1", item->errorString()) +  QL1S("</em>"));
            break;

        case DownloadItem::Done:
        default:
            if (QFile::exists(file))
            {
                div.appendInside(markup(QL1S("a")));
                div.lastChild().setAttribute(QL1S("class"), QL1S("greylink"));
                div.lastChild().setAttribute(QL1S("href"), QL1S("rekonq:downloads/opendir?q=") + QL1S("file://") + dir);
                div.lastChild().setPlainText(i18n("Open directory"));

                div.appendInside(QL1S(" - "));

                div.appendInside(markup(QL1S("a")));
                div.lastChild().setAttribute(QL1S("class"), QL1S("greylink"));
                div.lastChild().setAttribute(QL1S("href"), QL1S("file://") + file);
                div.lastChild().setPlainText(i18n("Open file"));
            }
            else
            {
                div.appendInside(QL1S("<em>") + i18n("Removed") +  QL1S("</em>"));
            }

            div.appendInside(QL1S(" - "));

            div.appendInside(markup(QL1S("a")));
            div.lastChild().setAttribute(QL1S("class"), QL1S("greylink"));
            div.lastChild().setAttribute(QL1S("href"), QL1S("rekonq:downloads/removeItem?item=") + QString::number(i));
            div.lastChild().setPlainText(i18n("Remove from list"));

            break;
        }

        i++;
    }

    if (i == 0)
    {
        m_root.addClass(QL1S("empty"));
        m_root.setPlainText(i18n("No matches for string %1 in downloads", filter));
    }
}


// void NewTabPage::tabsPage()
// {
//     m_root.addClass(QL1S("tabs"));
//
//     int wins = 0;
//     Q_FOREACH(const QPointer<MainWindow> &wPointer, rApp->mainWindowList())
//     {
//         m_root.appendInside(markup(QL1S("h3")));
//         m_root.lastChild().setPlainText(i18n("Window"));
//
//         MainWindow *w = wPointer.data();
//
//         const int tabCount = w->mainView()->count();
//         for (int i = 0; i < tabCount; ++i)
//         {
//             QUrl url = w->mainView()->webTab(i)->url();
//
//             if (!WebSnap::existsImage(url))
//             {
//                 qDebug() << "image doesn't exist for url: " << url;
//                 QPixmap preview = WebSnap::renderPagePreview(*w->mainView()->webTab(i)->page());
//                 QString path = WebSnap::imagePathFromUrl(url.url());
//                 preview.save(path);
//             }
//             QString name = w->mainView()->webTab(i)->view()->title();
//             QWebElement prev;
//
//             prev = tabPreview(wins, i, url, name);
//
//             m_root.appendInside(prev);
//         }
//
//         wins++;
//     }
// }


// ----------------------------------------------------------------------------
// LOW-LEVEL FUNCTIONS


QWebElement NewTabPage::emptyPreview(int index)
{
    QWebElement prev = markup(QL1S(".thumbnail"));

    prev.findFirst(QL1S(".preview img")).setAttribute(QL1S("src") ,
            QL1S("file:///") + KIconLoader::global()->iconPath( QL1S("insert-image"), KIconLoader::Desktop));
    prev.findFirst(QL1S("span a")).setPlainText(i18n("Set a Preview..."));
    prev.findFirst(QL1S("a")).setAttribute(QL1S("href"),
                                           QL1S("rekonq:preview/modify/") + QVariant(index).toString());

    setupPreview(prev, index, false);

    return prev;
}


void NewTabPage::reloadPreview(int index)
{
    QString id = QL1S("#preview") + QString::number(index);
    QWebElement thumb = m_root.document().findFirst(id);

    QString urlString = ReKonfig::previewUrls().at(index);
    QString nameString = ReKonfig::previewNames().at(index);

    QString title = checkTitle(QString::number(index + 1) + QL1S(" - ") + nameString);

    ThumbUpdater *t = new ThumbUpdater(thumb, urlString, title);
    t->updateThumb();
}


QWebElement NewTabPage::validPreview(int index, const QUrl &url, const QString &title)
{
    QWebElement prev = markup(QL1S(".thumbnail"));

    QString previewPath = WebSnap::existsImage(url)
                          ? QL1S("file://") + WebSnap::imagePathFromUrl(url)
                          : IconManager::self()->iconPathForUrl(url)
                          ;

    prev.findFirst(QL1S(".preview img")).setAttribute(QL1S("src") , previewPath);
    prev.findFirst(QL1S("a")).setAttribute(QL1S("href"), url.toString());
    prev.findFirst(QL1S("span a")).setAttribute(QL1S("href"), url.toString());
    prev.findFirst(QL1S("span a")).setPlainText(checkTitle(title));

    setupPreview(prev, index, true);
    return prev;
}


QWebElement NewTabPage::tabPreview(int winIndex, int tabIndex, const QUrl &url, const QString &title)
{
    QWebElement prev = markup(QL1S(".thumbnail"));
    QString previewPath = QL1S("file://") + WebSnap::imagePathFromUrl(url);

    QString href = QL1S("rekonq:tabs/show?win=") + QString::number(winIndex) + QL1S("&tab=") + QString::number(tabIndex);

    prev.findFirst(QL1S(".preview img")).setAttribute(QL1S("src") , previewPath);
    prev.findFirst(QL1S("a")).setAttribute(QL1S("href"), href);
    prev.findFirst(QL1S("span a")).setAttribute(QL1S("href"), href);
    prev.findFirst(QL1S("span a")).setPlainText(checkTitle(title));

    setupTabPreview(prev, winIndex, tabIndex);

    prev.findFirst(QL1S(".right")).setStyleProperty(QL1S("visibility"), QL1S("visible"));
    prev.findFirst(QL1S(".left")).setStyleProperty(QL1S("visibility"), QL1S("hidden"));

    return prev;
}


QWebElement NewTabPage::closedTabPreview(int index, const QUrl &url, const QString &title)
{
    QWebElement prev = markup(QL1S(".thumbnail"));

    QString previewPath = WebSnap::existsImage(url)
                          ? QL1S("file://") + WebSnap::imagePathFromUrl(url)
                          : IconManager::self()->iconPathForUrl(url)
                          ;

    QString href = QL1S("rekonq:closedtabs/restore?tab=") + QString::number(index);

    prev.findFirst(QL1S(".preview img")).setAttribute(QL1S("src") , previewPath);
    prev.findFirst(QL1S("a")).setAttribute(QL1S("href"), href);
    prev.findFirst(QL1S("span a")).setAttribute(QL1S("href"), href);
    prev.findFirst(QL1S("span a")).setPlainText(checkTitle(title));

    setupPreview(prev, index, true);
    return prev;
}


void NewTabPage::setupPreview(QWebElement e, int index, bool showControls)
{
    e.findFirst(QL1S(".right img")).setAttribute(QL1S("src"),
            QL1S("file:///") + KIconLoader::global()->iconPath( QL1S("edit-delete"), KIconLoader::DefaultState));

    e.findFirst(QL1S(".right")).setAttribute(QL1S("title"), i18n("Remove favorite"));

    e.findFirst(QL1S(".left img")).setAttribute(QL1S("src"),
            QL1S("file:///") + KIconLoader::global()->iconPath( QL1S("view-refresh"), KIconLoader::DefaultState));

    e.findFirst(QL1S(".left")).setAttribute(QL1S("title"), i18n("Reload thumbnail"));

    e.findFirst(QL1S(".left")).setAttribute(QL1S("href"), QL1S("rekonq:preview/reload/") + QVariant(index).toString());
    e.findFirst(QL1S(".right")).setAttribute(QL1S("href"), QL1S("rekonq:preview/remove/") + QVariant(index).toString());

    e.setAttribute(QL1S("id"), QL1S("preview") + QVariant(index).toString());

    if (showControls)
    {
        e.findFirst(QL1S(".right")).setStyleProperty(QL1S("visibility"), QL1S("visible"));
        e.findFirst(QL1S(".left")).setStyleProperty(QL1S("visibility"), QL1S("visible"));
    }
}


void NewTabPage::setupTabPreview(QWebElement e, int winIndex, int tabIndex)
{
    e.findFirst(QL1S(".right img")).setAttribute(QL1S("src"),
            QL1S("file:///") + KIconLoader::global()->iconPath( QL1S("edit-delete"), KIconLoader::DefaultState));
    e.findFirst(QL1S(".right")).setAttribute(QL1S("title"), QL1S("Close Tab"));

    QString href = QL1S("rekonq:tabs/remove?win=") + QString::number(winIndex) + QL1S("&tab=") + QString::number(tabIndex);
    e.findFirst(QL1S(".right")).setAttribute(QL1S("href"), href);

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

    loadPageForUrl(QUrl( QL1S("rekonq:favorites") ));

    ReKonfig::self()->save();
}


void NewTabPage::createBookmarkGroup(const KBookmark &bookmark, QWebElement parent)
{
    KBookmarkGroup group = bookmark.toGroup();
    KBookmark bm = group.first();

    parent.appendInside(markup(QL1S(".bookmarkfolder")));
    QWebElement folder = parent.lastChild();
    folder.appendInside(markup(QL1S("a")));
    folder.lastChild().setAttribute(QL1S("href"), QL1S("javascript: toggleChildren(\'") + group.fullText() + QL1S("\')"));
    QWebElement titleElement = folder.lastChild();
    titleElement.appendInside(markup(QL1S("h4")));
    titleElement.lastChild().setPlainText(group.fullText());

    folder.appendInside(markup(QL1S("div")));
    folder.lastChild().setAttribute(QL1S("id"), group.fullText());

    while (!bm.isNull())
    {
        createBookmarkItem(bm, folder.lastChild());
        bm = group.next(bm);
    }
}


void NewTabPage::createBookmarkItem(const KBookmark &bookmark, QWebElement parent)
{
    QString cacheDir = QL1S("file://") + QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    QString icon; 
    
    if (bookmark.isGroup())
    {
        createBookmarkGroup(bookmark, m_root);
        return;
    }
    else if (bookmark.isSeparator())
    {
        qDebug() << "SEPARATOR";
        parent.appendInside(QL1S("<hr />"));
    }
    else
    {
        QString b = bookmark.icon();
        if (b.contains(QL1S("favicons")))
            icon = cacheDir + bookmark.icon() + QL1S(".png");
        else
            icon = IconManager::self()->iconPathForUrl(bookmark.url());
        
        parent.appendInside(markup(QL1S("a")));
        QWebElement bookmarkElement = parent.lastChild();
        bookmarkElement.setAttribute(QL1S("href") , bookmark.url().url());
        bookmarkElement.addClass( QL1S("bookmark") );

        bookmarkElement.appendInside(markup(QL1S("img")));
        bookmarkElement.lastChild().setAttribute(QL1S("src") , icon);
        bookmarkElement.lastChild().setAttribute(QL1S("width") , QL1S("16"));
        bookmarkElement.lastChild().setAttribute(QL1S("height") , QL1S("16"));
        bookmarkElement.appendInside(QL1S(" "));
        bookmarkElement.appendInside(checkTitle(bookmark.fullText(), 40));
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


void NewTabPage::initJS()
{
    QWebFrame *parentFrame = qobject_cast<QWebFrame *>(parent());
    QString oldHTML = parentFrame->toHtml();

    QString includes;
    includes += QL1S("<head>");
    includes += QL1S("<script src=\"$DEFAULT_PATH/htmls/jquery-1.7.2.min.js\" type=\"text/javascript\"></script>");
    includes += QL1S("<script src=\"$DEFAULT_PATH/htmls/jquery-ui-1.8.20.custom.min.js\" type=\"text/javascript\"></script>");

    QString htmlFilePath = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QL1S("rekonq/htmls/home.html") );
    QString dataPath = QL1S("file://") + htmlFilePath;
    dataPath.remove(QL1S("/htmls/home.html"));

    includes.replace(QL1S("$DEFAULT_PATH"), dataPath);
    includes.replace(QL1S("$GENERAL_FONT"), QWebSettings::globalSettings()->fontFamily(QWebSettings::StandardFont));
        
    oldHTML.replace(QL1S("<head>"), includes);

    QString javascript;
    javascript += QL1S("<body>");
    javascript += QL1S("<script>");
    javascript += QL1S("$(function() {");
    javascript += QL1S("    $( \"#content\" ).sortable({");
    javascript += QL1S("        revert: true,");
    javascript += QL1S("        cursor: \"move\",");
    javascript += QL1S("        distance: 30,");
    javascript += QL1S("        update: function(event, ui) { window.location.href = \"rekonq:favorites/save\"; }");
    javascript += QL1S("    });");
    javascript += QL1S("    $( \".thumbnail\" ).disableSelection();");
    javascript += QL1S("});");
    javascript += QL1S("</script>");

    oldHTML.replace(QL1S("<body>"), javascript);

    parentFrame->setHtml(oldHTML);
}


void NewTabPage::saveFavorites()
{
    QStringList names = ReKonfig::previewNames();
    QStringList urls = ReKonfig::previewUrls();

    QStringList newNames = names;
    QStringList newUrls = urls;

    QWebElementCollection coll = m_root.document().findAll(QL1S(".thumbnail"));
    QList<QWebElement> list = coll.toList();

    int i = 0;

    Q_FOREACH(QWebElement e, list)
    {
        if (!e.hasAttribute(QL1S("id")))
            continue;

        QString id = e.attribute(QL1S("id"));
        qDebug() << "id: " << id;
        int index = id.remove(QL1S("preview")).toInt();
        qDebug() << "INDEX: " << index;

        newNames.replace(i, names.at(index));
        newUrls.replace(i, urls.at(index));
        i++;
    }

    ReKonfig::setPreviewNames(newNames);
    ReKonfig::setPreviewUrls(newUrls);

    loadPageForUrl(QUrl( QL1S("rekonq:favorites") ));
}
