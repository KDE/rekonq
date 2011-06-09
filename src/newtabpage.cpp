/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2009-2011 by Andrea Diamantini <adjam7 at gmail dot com>
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
#include "bookmarkprovider.h"
#include "downloadmanager.h"
#include "historymodels.h"
#include "mainview.h"
#include "mainwindow.h"
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
#include <QtCore/QFile>
#include <QtGui/QAction>
#include <QtWebKit/QWebFrame>


NewTabPage::NewTabPage(QWebFrame *frame)
        : QObject(frame)
        , m_root(frame->documentElement())
{
    QString htmlFilePath = KStandardDirs::locate("data", "rekonq/htmls/home.html");
    QString imagesPath = QL1S("file://") + KGlobal::dirs()->findResourceDir("data", "rekonq/pics/bg.png") + QL1S("rekonq/pics");

    QFile file(htmlFilePath);
    bool isOpened = file.open(QIODevice::ReadOnly);
    if (!isOpened)
    {
        kDebug() << "Couldn't open the home.html file";
    }
    else
    {
        m_html = file.readAll();
        m_html.replace(QL1S("%2"), imagesPath);
    }
}


void NewTabPage::generate(const KUrl &url)
{
    if (KUrl("about:preview").isParentOf(url))
    {
        if (url.fileName() == QL1S("add"))
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
        if (url.directory() == QL1S("preview/remove"))
        {
            removePreview(url.fileName().toInt());
            return;
        }
        if (url.directory() == QL1S("preview/modify"))
        {
            int index = url.fileName().toInt();
            rApp->mainWindow()->currentTab()->createPreviewSelectorBar(index);
            return;
        }
    }
    if (url.fileName() == QL1S("clear"))
    {
        rApp->mainWindow()->actionByName("clear_private_data")->trigger();
        generate(QString(QL1S("about:") + url.directory()));
        return;
    }
    if (url == KUrl("about:bookmarks/edit"))
    {
        rApp->bookmarkProvider()->bookmarkManager()->slotEditBookmarks();
        return;
    }

    WebPage *page = qobject_cast <WebPage *>(m_root.webFrame()->page());
    page->mainFrame()->setHtml(m_html);
    page->setIsOnRekonqPage(true);

    m_root = page->mainFrame()->documentElement().findFirst(QL1S("#content"));

    browsingMenu(url);

    QString title;
    QByteArray encodedUrl = url.toEncoded();
    if (encodedUrl == QByteArray("about:favorites"))
    {
        favoritesPage();
        title = i18n("Favorites");
    }
    else if (encodedUrl == QByteArray("about:closedTabs"))
    {
        closedTabsPage();
        title = i18n("Closed Tabs");
    }
    else if (encodedUrl == QByteArray("about:history"))
    {
        historyPage();
        title = i18n("History");
    }
    else if (encodedUrl == QByteArray("about:bookmarks"))
    {
        bookmarksPage();
        title = i18n("Bookmarks");
    }
    else if (encodedUrl == QByteArray("about:downloads"))
    {
        downloadsPage();
        title = i18n("Downloads");
    }

    m_root.document().findFirst(QL1S("title")).setPlainText(title);
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
        QWebElement prev;

        if (url.isEmpty())
            prev = emptyPreview(i);
        else if (!WebSnap::existsImage(url))
            prev = loadingPreview(i, url);
        else
            prev = validPreview(i, url, QString::number(i+1) + " - " + names.at(i));

        setupPreview(prev, i);

        m_root.appendInside(prev);
    }
}


QWebElement NewTabPage::emptyPreview(int index)
{
    QWebElement prev = markup(QL1S(".thumbnail"));

    prev.findFirst(QL1S(".preview img")).setAttribute(QL1S("src") ,
            QL1S("file:///") + KIconLoader::global()->iconPath("insert-image", KIconLoader::Desktop));
    prev.findFirst(QL1S("span a")).setPlainText(i18n("Set a Preview..."));
    prev.findFirst(QL1S("a")).setAttribute(QL1S("href"),
                                           QL1S("about:preview/modify/") + QVariant(index).toString());

    setupPreview(prev, index);
    //hideControls(prev);

    return prev;
}


QWebElement NewTabPage::loadingPreview(int index, const KUrl &url)
{
    QWebElement prev = markup(QL1S(".thumbnail"));

    prev.findFirst(QL1S(".preview img")).setAttribute(QL1S("src"),
            QL1S("file:///") + KStandardDirs::locate("appdata", "pics/busywidget.gif"));
    prev.findFirst(QL1S("span a")).setPlainText(i18n("Loading Preview..."));
    prev.findFirst(QL1S("a")).setAttribute(QL1S("href"), url.toMimeDataString());

    setupPreview(prev, index);
    showControls(prev);

    // NOTE: we need the page frame for two reasons
    // 1) to link to the WebPage calling the snapFinished slot
    // 2) to "auto-destroy" snaps on tab closing :)
    QWebFrame *frame = qobject_cast<QWebFrame *>(parent());
    WebSnap *snap = new WebSnap(url, frame);
    connect(snap, SIGNAL(snapDone(bool)), frame->page(), SLOT(updateImage(bool)), Qt::UniqueConnection);
    return prev;
}


QWebElement NewTabPage::validPreview(int index, const KUrl &url, const QString &title)
{
    QWebElement prev = markup(QL1S(".thumbnail"));
    QString previewPath = QL1S("file://") + WebSnap::imagePathFromUrl(url);

    prev.findFirst(QL1S(".preview img")).setAttribute(QL1S("src") , previewPath);
    prev.findFirst(QL1S("a")).setAttribute(QL1S("href"), url.toMimeDataString());
    prev.findFirst(QL1S("span a")).setAttribute(QL1S("href"), url.toMimeDataString());
    prev.findFirst(QL1S("span a")).setPlainText(checkTitle(title));

    setupPreview(prev, index);
    showControls(prev);
    return prev;
}


void NewTabPage::hideControls(QWebElement e)
{
    e.findFirst(QL1S(".remove")).setStyleProperty(QL1S("visibility"), QL1S("hidden"));
    e.findFirst(QL1S(".modify")).setStyleProperty(QL1S("visibility"), QL1S("hidden"));
}


void NewTabPage::showControls(QWebElement e)
{
    e.findFirst(QL1S(".remove")).setStyleProperty(QL1S("visibility"), QL1S("visible"));
    e.findFirst(QL1S(".modify")).setStyleProperty(QL1S("visibility"), QL1S("visible"));
}


void NewTabPage::setupPreview(QWebElement e, int index)
{
    e.findFirst(QL1S(".remove img")).setAttribute(QL1S("src"), QL1S("file:///") + KIconLoader::global()->iconPath("edit-delete", KIconLoader::DefaultState));
    e.findFirst(QL1S(".remove")).setAttribute(QL1S("title"), QL1S("Remove favorite"));
    e.findFirst(QL1S(".modify img")).setAttribute(QL1S("src"), QL1S("file:///") + KIconLoader::global()->iconPath("insert-image", KIconLoader::DefaultState));
    e.findFirst(QL1S(".modify")).setAttribute(QL1S("title"), QL1S("Set new favorite"));

    e.findFirst(QL1S(".modify")).setAttribute(QL1S("href"), QL1S("about:preview/modify/") + QVariant(index).toString());
    e.findFirst(QL1S(".remove")).setAttribute(QL1S("href"), QL1S("about:preview/remove/") + QVariant(index).toString());

    e.setAttribute(QL1S("id"), QL1S("preview") + QVariant(index).toString());
}


void NewTabPage::snapFinished()
{
    // Update page, but only if open
    if (m_root.document().findAll(QL1S("#rekonq-newtabpage")).count() == 0)
        return;
    if (m_root.findAll(QL1S(".favorites")).count() == 0 && m_root.findAll(QL1S(".closedTabs")).count() == 0)
        return;

    QStringList urls = ReKonfig::previewUrls();
    QStringList names = ReKonfig::previewNames();

    for (int i = 0; i < urls.count(); i++)
    {
        KUrl url = KUrl(urls.at(i));
        QString title = names.at(i);

        if (WebSnap::existsImage(url))
        {
            QWebElement prev = m_root.findFirst(QL1S("#preview") + QVariant(i).toString());
            if (KUrl(prev.findFirst("a").attribute(QL1S("href"))) == url)
            {
                QWebElement newPrev = validPreview(i, url, title);

                if (m_root.findAll(QL1S(".closedTabs")).count() != 0)
                    hideControls(newPrev);

                prev.replace(newPrev);
            }
        }
    }
}


void NewTabPage::removePreview(int index)
{
    QStringList names = ReKonfig::previewNames();
    QStringList urls = ReKonfig::previewUrls();

    urls.removeAt(index);
    names.removeAt(index);

    ReKonfig::setPreviewNames(names);
    ReKonfig::setPreviewUrls(urls);

    generate(KUrl("about:favorites"));

    ReKonfig::self()->writeConfig();
}


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

    foreach(QWebElement it, navItems)
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


void NewTabPage::historyPage()
{
    m_root.addClass(QL1S("history"));

    const QWebElement clearData = createLinkItem(i18n("Clear Private Data"),
                                  QL1S("about:history/clear"),
                                  QL1S("edit-clear"),
                                  KIconLoader::Toolbar);
    m_root.document().findFirst(QL1S("#actions")).appendInside(clearData);

    HistoryTreeModel *model = rApp->historyManager()->historyTreeModel();

    if (model->rowCount() == 0)
    {
        m_root.addClass(QL1S("empty"));
        m_root.setPlainText(i18n("Your browsing history is empty"));
        return;
    }

    int i = 0;
    QString faviconsDir = KStandardDirs::locateLocal("cache" , "favicons/" , true);
    QString icon = QL1S("file://") + KGlobal::dirs()->findResource("icon", "oxygen/16x16/mimetypes/text-html.png");
    do
    {
        QModelIndex index = model->index(i, 0, QModelIndex());
        if (model->hasChildren(index))
        {
            m_root.appendInside(markup(QL1S("h3")));
            m_root.lastChild().setPlainText(index.data().toString());

            for (int j = 0; j < model->rowCount(index); ++j)
            {
                QModelIndex son = model->index(j, 0, index);
                KUrl u = son.data(HistoryModel::UrlStringRole).toUrl();

                QString b = faviconsDir + u.host() + QL1S(".png");
                if (QFile::exists(b))
                    icon = QL1S("file://") + b;

                m_root.appendInside(son.data(HistoryModel::DateTimeRole).toDateTime().toString("hh:mm"));
                m_root.appendInside(QL1S("  "));
                m_root.appendInside(markup(QL1S("img")));
                m_root.lastChild().setAttribute(QL1S("src"), icon);
                m_root.lastChild().setAttribute(QL1S("width"), QL1S("16"));
                m_root.lastChild().setAttribute(QL1S("height"), QL1S("16"));
                m_root.appendInside(QL1S(" "));
                m_root.appendInside(markup(QL1S("a")));
                m_root.lastChild().setAttribute(QL1S("href") , u.url());
                m_root.lastChild().appendInside(son.data().toString());
                m_root.appendInside(QL1S("<br />"));
            }
        }
        i++;
    }
    while (model->hasIndex(i , 0 , QModelIndex()));
}


void NewTabPage::bookmarksPage()
{
    m_root.addClass(QL1S("bookmarks"));

    const QWebElement editBookmarks = createLinkItem(i18n("Edit Bookmarks"),
                                      QL1S("about:bookmarks/edit"),
                                      QL1S("bookmarks-organize"),
                                      KIconLoader::Toolbar);
    m_root.document().findFirst(QL1S("#actions")).appendInside(editBookmarks);

    KBookmarkGroup bookGroup = rApp->bookmarkProvider()->rootGroup();
    if (bookGroup.isNull())
    {
        m_root.addClass(QL1S("empty"));
        m_root.setPlainText(i18n("You have no bookmarks"));
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
    QString cacheDir = QL1S("file://") + KStandardDirs::locateLocal("cache" , "" , true);
    QString icon = QL1S("file://") + KGlobal::dirs()->findResource("icon", "oxygen/16x16/mimetypes/text-html.png");
    if (bookmark.isGroup())
    {
        KBookmarkGroup group = bookmark.toGroup();
        KBookmark bm = group.first();
        parent.appendInside(markup(QL1S("h3")));
        parent.lastChild().setPlainText(group.fullText());
        parent.appendInside(markup(QL1S(".bookfolder")));
        while (!bm.isNull())
        {
            createBookItem(bm, parent.lastChild()); // it is .bookfolder
            bm = group.next(bm);
        }
    }
    else if (bookmark.isSeparator())
    {
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
        parent.lastChild().setPlainText(bookmark.fullText());
        parent.appendInside(QL1S("<br />"));
    }
}


void NewTabPage::closedTabsPage()
{
    m_root.addClass(QL1S("closedTabs"));

    QList<HistoryItem> links = rApp->mainWindow()->mainView()->recentlyClosedTabs();

    if (links.isEmpty())
    {
        m_root.addClass(QL1S("empty"));
        m_root.setPlainText(i18n("There are no recently closed tabs"));
        return;
    }

    for (int i = 0; i < links.count(); ++i)
    {
        HistoryItem item = links.at(i);
        QWebElement prev;

        if (item.url.isEmpty())
            continue;

        prev = WebSnap::existsImage(KUrl(item.url))
               ? validPreview(i, item.url, item.title)
               : loadingPreview(i, item.url);

        prev.setAttribute(QL1S("id"),  QL1S("preview") + QVariant(i).toString());
        hideControls(prev);
        m_root.appendInside(prev);
    }
}


QString NewTabPage::checkTitle(const QString &title)
{
    QString t(title);
    if (t.length() > 23)
    {
        t.truncate(20);
        t +=  QL1S("...");
    }
    return t;
}


void NewTabPage::downloadsPage()
{
    m_root.addClass(QL1S("downloads"));

    const QWebElement clearData = createLinkItem(i18n("Clear Private Data"),
                                  QL1S("about:downloads/clear"),
                                  QL1S("edit-clear"),
                                  KIconLoader::Toolbar);
    m_root.document().findFirst(QL1S("#actions")).appendInside(clearData);

    DownloadList list = rApp->downloadManager()->downloads();

    if (list.isEmpty())
    {
        m_root.addClass(QL1S("empty"));
        m_root.setPlainText(i18n("There are no recently downloaded files to show"));
        return;
    }

    foreach(DownloadItem *item, list)
    {
        m_root.prependInside(markup(QL1S("div")));

        QWebElement div = m_root.firstChild();
        div.addClass(QL1S("download"));

        KUrl u = KUrl(item->destinationUrl());
        QString fName = u.fileName();
        QString dir = QL1S("file://") + u.directory();
        QString file = dir +  QL1C('/') + fName;

        KIconLoader *loader = KIconLoader::global();
        QString iconPath = QL1S("file://") + loader->iconPath(KMimeType::iconNameForUrl(u), KIconLoader::Desktop);

        div.appendInside(markup(QL1S("img")));
        div.lastChild().setAttribute(QL1S("src"), iconPath);

        div.appendInside(QL1S("<strong>") + fName +  QL1S("</strong>"));
        div.appendInside(QL1S(" - "));
        QString date = KGlobal::locale()->formatDateTime(item->dateTime(), KLocale::FancyLongDate);
        div.appendInside(QL1S("<em>") + date +  QL1S("</em>"));
        div.appendInside(QL1S("<br />"));

        div.appendInside(QL1S("<a href=") + item->originUrl() +  QL1C('>') + item->originUrl() +  QL1S("</a>"));
        div.appendInside(QL1S("<br />"));

        div.appendInside(markup(QL1S("a")));
        div.lastChild().setAttribute(QL1S("href"), dir);
        div.lastChild().setPlainText(i18n("Open directory"));

        div.appendInside(QL1S(" - "));
        div.appendInside(markup(QL1S("a")));
        div.lastChild().setAttribute(QL1S("href"), file);
        div.lastChild().setPlainText(i18n("Open file"));
    }
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
