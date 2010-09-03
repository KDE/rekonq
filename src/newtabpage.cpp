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
#include "application.h"
#include "bookmarkprovider.h"
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
    if (KUrl("about:preview").isParentOf(url))
    {
        if (url.fileName() == QString("add"))
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
        if (url.directory() == QString("preview/remove"))
        {
            removePreview(url.fileName().toInt());
            return;
        }
        if (url.directory() == QString("preview/modify"))
        {
            int index = url.fileName().toInt();
            Application::instance()->mainWindow()->currentTab()->createPreviewSelectorBar(index);
            return;
        }
    }
    if (url.fileName() == QString("clear"))
    {
        Application::instance()->mainWindow()->actionByName("clear_private_data")->trigger();
        generate(QString("about:" + url.directory()));
        return;
    }
    if (url == KUrl("about:bookmarks/edit"))
    {
        Application::bookmarkProvider()->bookmarkManager()->slotEditBookmarks();
        return;
    }

    WebPage *page = qobject_cast <WebPage *>(m_root.webFrame()->page());
    page->mainFrame()->setHtml(m_html);
    page->setIsOnRekonqPage(true);

    m_root = page->mainFrame()->documentElement().findFirst("#content");

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

    m_root.document().findFirst("title").setPlainText(title);
}


void NewTabPage::favoritesPage()
{
    m_root.addClass("favorites");

    const QWebElement add = createLinkItem(i18n("Add Favorite"),
                                           QLatin1String("about:preview/add"),
                                           QLatin1String("list-add"),
                                           KIconLoader::Toolbar);
    m_root.document().findFirst("#actions").appendInside(add);

    QStringList names = ReKonfig::previewNames();
    QStringList urls = ReKonfig::previewUrls();

    if (urls.isEmpty())
    {
        m_root.addClass("empty");
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

    // NOTE: we need the page frame for two reasons
    // 1) to link to the WebPage calling the snapFinished slot
    // 2) to "auto-destroy" snaps on tab closing :)
    QWebFrame *frame = qobject_cast<QWebFrame *>(parent());
    WebSnap *snap = new WebSnap(url, frame);
    connect(snap, SIGNAL(snapDone(bool)), frame->page(), SLOT(updateImage(bool)));
    return prev;
}


QWebElement NewTabPage::validPreview(int index, const KUrl &url, const QString &title)
{
    QWebElement prev = markup(".thumbnail");
    QString previewPath = QL1S("file://") + WebSnap::imagePathFromUrl(url);
    QString iString = QVariant(index).toString();

    prev.findFirst(".preview img").setAttribute("src" , previewPath);
    prev.findFirst("a").setAttribute("href", url.toMimeDataString());   // NOTE ?
    prev.findFirst("span a").setAttribute("href", url.toMimeDataString());  // NOTE ?
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


void NewTabPage::snapFinished()
{
    // Update page, but only if open
    if (m_root.document().findAll("#rekonq-newtabpage").count() == 0)
        return;
    if (m_root.findAll(".favorites").count() == 0 && m_root.findAll(".closedTabs").count() == 0)
        return;

    QStringList urls = ReKonfig::previewUrls();
    QStringList names = ReKonfig::previewNames();

    for (int i = 0; i < urls.count(); i++)
    {
        KUrl url = KUrl(urls.at(i));
        QString title = names.at(i);

        if (WebSnap::existsImage(url))
        {
            QWebElement prev = m_root.findFirst("#preview" + QVariant(i).toString());
            if (KUrl(prev.findFirst("a").attribute("href")) == url)
            {
                QWebElement newPrev = validPreview(i, url, title);

                if (m_root.findAll(".closedTabs").count() != 0)
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
                                   QLatin1String("about:favorites"),
                                   QLatin1String("emblem-favorite"),
                                   KIconLoader::Toolbar));

    // Closed Tabs
    navItems.append(createLinkItem(i18n("Closed Tabs"),
                                   QLatin1String("about:closedTabs"),
                                   QLatin1String("tab-close"),
                                   KIconLoader::Toolbar));

    // Bookmarks
    navItems.append(createLinkItem(i18n("Bookmarks"),
                                   QLatin1String("about:bookmarks"),
                                   QLatin1String("bookmarks"),
                                   KIconLoader::Toolbar));

    // History
    navItems.append(createLinkItem(i18n("History"),
                                   QLatin1String("about:history"),
                                   QLatin1String("view-history"),
                                   KIconLoader::Toolbar));

    // Downloads
    navItems.append(createLinkItem(i18n("Downloads"),
                                   QLatin1String("about:downloads"),
                                   QLatin1String("download"),
                                   KIconLoader::Toolbar));

    foreach(QWebElement it, navItems)
    {
        const QString aTagString('a');
        const QString hrefAttributeString(QLatin1String("href"));

        if (it.findFirst(aTagString).attribute(hrefAttributeString) == currentUrl.toMimeDataString())
            it.addClass(QLatin1String("current"));
        else if (currentUrl == QLatin1String("about:home") && it.findFirst(aTagString).attribute(hrefAttributeString) == QLatin1String("about:favorites"))
            it.addClass(QLatin1String("current"));
        m_root.document().findFirst(QLatin1String("#navigation")).appendInside(it);
    }
}


void NewTabPage::historyPage()
{
    m_root.addClass("history");

    const QWebElement clearData = createLinkItem(i18n("Clear Private Data"),
                                                 QLatin1String("about:history/clear"),
                                                 QLatin1String("edit-clear"),
                                                 KIconLoader::Toolbar);
    m_root.document().findFirst("#actions").appendInside(clearData);

    HistoryTreeModel *model = Application::historyManager()->historyTreeModel();

    if (model->rowCount() == 0)
    {
        m_root.addClass("empty");
        m_root.setPlainText(i18n("Your browsing history is empty"));
        return;
    }

    int i = 0;
    do
    {
        QModelIndex index = model->index(i, 0, QModelIndex());
        if (model->hasChildren(index))
        {
            m_root.appendInside(markup("h3"));
            m_root.lastChild().setPlainText(index.data().toString());

            for (int j = 0; j < model->rowCount(index); ++j)
            {
                QModelIndex son = model->index(j, 0, index);
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
    while (model->hasIndex(i , 0 , QModelIndex()));
}


void NewTabPage::bookmarksPage()
{
    m_root.addClass("bookmarks");

    const QWebElement editBookmarks = createLinkItem(i18n("Edit Bookmarks"),
                                                     QLatin1String("about:bookmarks/edit"),
                                                     QLatin1String("bookmarks-organize"),
                                                     KIconLoader::Toolbar);
    m_root.document().findFirst("#actions").appendInside(editBookmarks);

    KBookmarkGroup bookGroup = Application::bookmarkProvider()->rootGroup();
    if (bookGroup.isNull())
    {
        m_root.addClass("empty");
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
    if (bookmark.isGroup())
    {
        KBookmarkGroup group = bookmark.toGroup();
        KBookmark bm = group.first();
        parent.appendInside(markup("h3"));
        parent.lastChild().setPlainText(group.fullText());
        parent.appendInside(markup(".bookfolder"));
        while (!bm.isNull())
        {
            createBookItem(bm, parent.lastChild()); // it is .bookfolder
            bm = group.next(bm);
        }
    }
    else if (bookmark.isSeparator())
    {
        parent.appendInside("<hr/>");
    }
    else
    {
        parent.appendInside(markup("a"));
        parent.lastChild().setAttribute("href" , bookmark.url().prettyUrl());
        parent.lastChild().setPlainText(bookmark.fullText());
        parent.appendInside("<br/>");
    }
}


void NewTabPage::closedTabsPage()
{
    m_root.addClass("closedTabs");

    QList<HistoryItem> links = Application::instance()->mainWindow()->mainView()->recentlyClosedTabs();

    if (links.isEmpty())
    {
        m_root.addClass("empty");
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

        prev.setAttribute("id", "preview" + QVariant(i).toString());
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
        t += "...";
    }
    return t;
}


void NewTabPage::downloadsPage()
{
    m_root.addClass("downloads");

    const QWebElement clearData = createLinkItem(i18n("Clear Private Data"),
                                                 QLatin1String("about:downloads/clear"),
                                                 QLatin1String("edit-clear"),
                                                 KIconLoader::Toolbar);
    m_root.document().findFirst("#actions").appendInside(clearData);

    DownloadList list = Application::instance()->downloads();

    if (list.isEmpty())
    {
        m_root.addClass("empty");
        m_root.setPlainText(i18n("There are no recently downloaded files to show"));
        return;
    }

    foreach(const DownloadItem &item, list)
    {
        m_root.prependInside(markup("div"));

        QWebElement div = m_root.firstChild();
        div.addClass("download");

        KUrl u = KUrl(item.destUrlString);
        QString fName = u.fileName();
        QString dir = QL1S("file://") + u.directory();
        QString file = dir + '/' + fName;

        KIconLoader *loader = KIconLoader::global();
        QString iconPath = "file://" + loader->iconPath(KMimeType::iconNameForUrl(u), KIconLoader::Desktop);

        div.appendInside(markup("img"));
        div.lastChild().setAttribute("src", iconPath);

        div.appendInside("<strong>" + fName + "</strong>");
        div.appendInside(" - ");
        QString date = KGlobal::locale()->formatDateTime(item.dateTime, KLocale::FancyLongDate);
        div.appendInside("<em>" + date + "</em>");
        div.appendInside("<br/>");

        div.appendInside("<a href=" + item.srcUrlString + '>' + item.srcUrlString + "</a>");
        div.appendInside("<br/>");

        div.appendInside(markup("a"));
        div.lastChild().setAttribute("href", dir);
        div.lastChild().setPlainText(i18n("Open directory"));

        div.appendInside(" - ");
        div.appendInside(markup("a"));
        div.lastChild().setAttribute("href", file);
        div.lastChild().setPlainText(i18n("Open file"));
    }
}

QWebElement NewTabPage::createLinkItem(const QString &title, const QString &urlString, const QString &iconPath, int groupOrSize) const
{
    const KIconLoader * const loader = KIconLoader::global();

    QWebElement nav = markup(QLatin1String(".link"));
    nav.findFirst(QString('a')).setAttribute(QLatin1String("href"), urlString);
    nav.findFirst(QLatin1String("img")).setAttribute(QLatin1String("src"),
                                                     QString::fromLatin1("file://") + loader->iconPath(iconPath, groupOrSize));
    nav.findFirst(QLatin1String("span")).appendInside(title);
    return nav;
}
