/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2011-2012 by Andrea Diamantini <adjam7 at gmail dot com>
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
#include "webview.h"
#include "webview.moc"

// Local Includes
#include "searchengine.h"

// KDE Includes
#include <KIO/Job>
#include <KIO/RenameDialog>
#include <KIO/JobUiDelegate>

#include <KGlobalSettings>
#include <KStandardDirs>
#include <KFileDialog>
#include <KJobUiDelegate>
#include <KLocalizedString>
#include <KMenu>
#include <KAction>
#include <KUrl>
#include <KRun>
#include <KToolInvocation>
#include <KActionMenu>

// Qt Includes
#include <QUrl>
#include <QDebug>
#include <QWebHitTestResult>
#include <QWebHistory>
#include <QNetworkRequest>
#include <QPointer>
#include <QWebSettings>
#include <QApplication>
#include <QMimeData>
#include <QClipboard>

// Defines
#define QL1S(x) QLatin1String(x)


WebView::WebView(QWidget *parent)
    : KWebView(parent)
    , m_page(0)
{
    page()->setForwardUnsupportedContent(true);
    connect(this, SIGNAL(linkShiftClicked(KUrl)), page(), SLOT(downloadUrl(KUrl)));

    setContextMenuPolicy(Qt::CustomContextMenu);

    connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(menuRequested(QPoint)));
}


WebPage *WebView::page()
{
    if (!m_page)
    {
        m_page = new WebPage(this);
        setPage(m_page);
    }
    return m_page;
}


void WebView::menuRequested(const QPoint &pos)
{
    QWebHitTestResult result = page()->mainFrame()->hitTestContent(pos);

    KMenu menu(this);
    QAction *a;

    KAction *sendByMailAction = new KAction(this);
    sendByMailAction->setIcon(KIcon("mail-send"));
    connect(sendByMailAction, SIGNAL(triggered(bool)), this, SLOT(sendByMail()));

    KAction *openInDefaultBrowserAction = new KAction(KIcon("window-new"), i18n("Open in default browser"), this);
    connect(openInDefaultBrowserAction, SIGNAL(triggered(bool)), this, SLOT(openLinkInDefaultBrowser()));

    // Choose right context
    int resultHit = 0;
    if (result.linkUrl().isEmpty())
        resultHit = WebView::EmptySelection;
    else
        resultHit = WebView::LinkSelection;

    if (!result.pixmap().isNull())
        resultHit |= WebView::ImageSelection;

    if (result.isContentSelected())
        resultHit = WebView::TextSelection;

    // -----------------------------------------------------------
    // Ok, let's start filling up the menu...

    // is content editable? Add PASTE
    if (result.isContentEditable())
    {
        menu.addAction(pageAction(KWebPage::Paste));
        menu.addSeparator();
    }


    // EMPTY PAGE ACTIONS ----------------------------------------
    if (resultHit == WebView::EmptySelection)
    {
        // send by mail: page url
        sendByMailAction->setData(page()->currentFrame()->url());
        sendByMailAction->setText(i18n("Share page url"));

        // navigation
        QWebHistory *history = page()->history();
        if (history->canGoBack())
        {
            menu.addAction(pageAction(KWebPage::Back));
        }

        if (history->canGoForward())
        {
            menu.addAction(pageAction(KWebPage::Forward));
        }

        menu.addAction(pageAction(KWebPage::Reload));

        menu.addSeparator();

        // Page Actions
        menu.addAction(pageAction(KWebPage::SelectAll));

        menu.addAction(pageAction(KWebPage::DownloadLinkToDisk));

    }

    // LINK ACTIONS ------------------------------------------
    if (resultHit & WebView::LinkSelection)
    {
        // send by mail: link url
        sendByMailAction->setData(result.linkUrl());
        sendByMailAction->setText(i18n("Share link"));

        openInDefaultBrowserAction->setData(result.linkUrl());
        menu.addAction(openInDefaultBrowserAction);

        menu.addSeparator();

        a = pageAction(KWebPage::DownloadLinkToDisk);
        menu.addAction(a);
        menu.addAction(pageAction(KWebPage::CopyLinkToClipboard));
    }

    // IMAGE ACTION -----------------------------------------
    if (resultHit & WebView::ImageSelection)
    {
        // send by mail: image url
        sendByMailAction->setData(result.imageUrl());
        sendByMailAction->setText(i18n("Share image link"));

        menu.addSeparator();

        a = new KAction(KIcon("view-preview"), i18n("&View Image"), this);
        a->setData(result.imageUrl());
        connect(a, SIGNAL(triggered(Qt::MouseButtons, Qt::KeyboardModifiers)),
                this, SLOT(viewImage(Qt::MouseButtons, Qt::KeyboardModifiers)));
        menu.addAction(a);

        menu.addAction(pageAction(KWebPage::DownloadImageToDisk));

        a = new KAction(KIcon("view-media-visualization"), i18n("&Copy Image Location"), this);
        a->setData(result.imageUrl());
        connect(a, SIGNAL(triggered(Qt::MouseButtons, Qt::KeyboardModifiers)), this, SLOT(slotCopyImageLocation()));
        menu.addAction(a);

    }

    // ACTIONS FOR TEXT SELECTION ----------------------------
    if (resultHit & WebView::TextSelection)
    {
        // send by mail: text
        sendByMailAction->setData(selectedText());
        sendByMailAction->setText(i18n("Share selected text"));

        if (result.isContentEditable())
        {
            // actions for text selected in field
            menu.addAction(pageAction(KWebPage::Cut));
        }

        a = pageAction(KWebPage::Copy);
        if (!result.linkUrl().isEmpty())
            a->setText(i18n("Copy Text")); //for link
        else
            a->setText(i18n("Copy"));
        menu.addAction(a);

        if (selectedText().contains('.') && selectedText().indexOf('.') < selectedText().length()
                && !selectedText().trimmed().contains(" ")
           )
        {
            QString text = selectedText();
            text = text.trimmed();
            KUrl urlLikeText(text);
            if (urlLikeText.isValid())
            {
                QString truncatedUrl = text;
                const int maxTextSize = 18;
                if (truncatedUrl.length() > maxTextSize)
                {
                    const int truncateSize = 15;
                    truncatedUrl.truncate(truncateSize);
                    truncatedUrl += QL1S("...");
                }

                openInDefaultBrowserAction->setData(QUrl(urlLikeText));
                menu.addAction(openInDefaultBrowserAction);

                menu.addSeparator();
            }
        }

        // Default SearchEngine
        KService::Ptr defaultEngine = SearchEngine::defaultEngine();
        if (defaultEngine) // check if a default engine is set
        {
            a = new KAction(i18nc("Search selected text with the default search engine", "Search with %1", defaultEngine->name()), this);
            a->setIcon(QWebSettings::iconForUrl(SearchEngine::buildQuery(defaultEngine, "")));
            a->setData(defaultEngine->entryPath());
            connect(a, SIGNAL(triggered(bool)), this, SLOT(search()));
            menu.addAction(a);
        }

        // All favourite ones
        KActionMenu *searchMenu = new KActionMenu(KIcon("edit-find"), i18nc("@title:menu", "Search"), this);

        Q_FOREACH(const KService::Ptr & engine, SearchEngine::favorites())
        {
            a = new KAction(i18nc("@item:inmenu Search, %1 = search engine", "With %1", engine->name()), this);
            a->setIcon(QWebSettings::iconForUrl(SearchEngine::buildQuery(engine, "")));
            a->setData(engine->entryPath());
            connect(a, SIGNAL(triggered(bool)), this, SLOT(search()));
            searchMenu->addAction(a);
        }

        a = new KAction(KIcon("edit-find"), i18n("On Current Page"), this);
        connect(a, SIGNAL(triggered()), this, SLOT(findSelectedText()));
        searchMenu->addAction(a);

        if (!searchMenu->menu()->isEmpty())
        {
            menu.addAction(searchMenu);
        }
    }

    // DEFAULT ACTIONs (on the bottom) -----------------------
    menu.addSeparator();

    // FIXME: bookmarks management
//     if (resultHit & WebView::LinkSelection)
//     {
//         a = new KAction(KIcon("bookmark-new"), i18n("&Bookmark link"), this);
//         a->setData(result.linkUrl());
//         connect(a, SIGNAL(triggered(bool)), this, SLOT(bookmarkLink()));
//         menu.addAction(a);
//     }
//     else
//     {
//         a = new KAction(KIcon("bookmark-new"), i18n("&Add Bookmark"), this);
//         connect(a, SIGNAL(triggered(bool)), this, SLOT(bookmarkCurrentPage()));
//         menu.addAction(a);
//     }
    menu.addAction(sendByMailAction);


    menu.exec(mapToGlobal(pos));
}


void WebView::openLinkInDefaultBrowser()
{
    KAction *a = qobject_cast<KAction*>(sender());
    KUrl u(a->data().toUrl());

    (void)new KRun(u, this, 0);
}


void WebView::sendByMail()
{
    KAction *a = qobject_cast<KAction*>(sender());
    QString url = a->data().toString();

    KToolInvocation::invokeMailer("", "", "", "", url);
}


void WebView::findSelectedText()
{
    QWebPage::FindFlags options = QWebPage::FindWrapsAroundDocument;

    findText(selectedText(), options);
}


void WebView::search()
{
    KAction *a = qobject_cast<KAction*>(sender());
    KService::Ptr engine = KService::serviceByDesktopPath(a->data().toString());
    KUrl urlSearch = KUrl(SearchEngine::buildQuery(engine, selectedText()));

    load(urlSearch);
}


void WebView::viewImage(Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers)
{
    Q_UNUSED(buttons);
    Q_UNUSED(modifiers);

    KAction *a = qobject_cast<KAction*>(sender());
    KUrl url(a->data().toUrl());

    load(url);
}


void WebView::slotCopyImageLocation()
{
    KAction *a = qobject_cast<KAction*>(sender());
    KUrl imageUrl(a->data().toUrl());
#ifndef QT_NO_MIMECLIPBOARD
    // Set it in both the mouse selection and in the clipboard
    QMimeData* mimeData = new QMimeData;
    imageUrl.populateMimeData(mimeData);
    QApplication::clipboard()->setMimeData(mimeData, QClipboard::Clipboard);
    mimeData = new QMimeData;
    imageUrl.populateMimeData(mimeData);
    QApplication::clipboard()->setMimeData(mimeData, QClipboard::Selection);
#else
    QApplication::clipboard()->setText(imageUrl.url());
#endif
}
