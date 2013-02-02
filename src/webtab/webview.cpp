/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2008-2012 by Andrea Diamantini <adjam7 at gmail dot com>
* Copyright (C) 2009-2011 by Lionel Chauvin <megabigbug@yahoo.fr>
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

// Auto Includes
#include "rekonq.h"

// App Includes
#include "application.h"

// Local Includes
#include "adblockmanager.h"
#include "bookmarkmanager.h"
#include "downloadmanager.h"
#include "iconmanager.h"

#include "searchengine.h"
#include "webpage.h"
#include "webtab.h"
#include "webwindow.h"

// KDE Includes
#include <KAction>
#include <KActionMenu>
#include <KLocalizedString>
#include <KMenu>
#include <KStandardAction>
#include <KStandardDirs>
#include <KToolInvocation>

#include <sonnet/speller.h>
#include <Sonnet/Dialog>
#include <sonnet/backgroundchecker.h>

// Qt Includes
#include <QFile>
#include <QTimer>

#include <QApplication>
#include <QClipboard>
#include <QContextMenuEvent>
#include <QLabel>

#include <QWebFrame>
#include <QWebHistory>
#include <QNetworkRequest>


// needed for the spellCheck
static QVariant execJScript(QWebHitTestResult result, const QString& script)
{
    QWebElement element(result.element());
    if (element.isNull())
        return QVariant();
    return element.evaluateJavaScript(script);
}


// --------------------------------------------------------------------------------------------------


WebView::WebView(QWidget* parent, bool isPrivateBrowsing)
    : KWebView(parent, false)
    , m_autoScrollTimer(new QTimer(this))
    , m_verticalAutoScrollSpeed(0)
    , m_horizontalAutoScrollSpeed(0)
    , m_isViewAutoScrolling(false)
    , m_autoScrollIndicator(QPixmap(KStandardDirs::locate("appdata" , "pics/autoscroll.png")))
    , m_smoothScrollTimer(new QTimer(this))
    , m_dy(0)
    , m_smoothScrollSteps(0)
    , m_isViewSmoothScrolling(false)
    , m_accessKeysPressed(false)
    , m_accessKeysActive(false)
    , m_parentTab(qobject_cast<WebTab *>(parent))
    , m_isPrivateBrowsing(isPrivateBrowsing)
{
    // loadUrl signal
    connect(this, SIGNAL(loadUrl(KUrl,Rekonq::OpenType)), rApp, SLOT(loadUrl(KUrl,Rekonq::OpenType)));

    // Auto scroll timer
    connect(m_autoScrollTimer, SIGNAL(timeout()), this, SLOT(scrollFrameChanged()));
    m_autoScrollTimer->setInterval(100);

    // Smooth scroll timer
    connect(m_smoothScrollTimer, SIGNAL(timeout()), this, SLOT(scrollTick()));
    m_smoothScrollTimer->setInterval(16);

    connect(this, SIGNAL(loadStarted()), this, SLOT(loadStarted()));
}


WebView::~WebView()
{
    if (m_isViewSmoothScrolling)
        stopSmoothScrolling();
}


void WebView::load(const QUrl &url)
{
    load(QNetworkRequest(url));
}


void WebView::load(const QNetworkRequest &req, QNetworkAccessManager::Operation op, const QByteArray &body)
{
    QNetworkRequest request = req;
    const QUrl &reqUrl = request.url();
    if (reqUrl.host() == url().host())
    {
        request.setRawHeader(QByteArray("Referer"), url().toEncoded());
    }

    KWebView::load(request, op, body);
}


void WebView::loadStarted()
{
    hideAccessKeys();
}


WebPage *WebView::page()
{
    WebPage *p = qobject_cast<WebPage *>(KWebView::page());
    if (!p)
    {
        p = new WebPage(this, m_isPrivateBrowsing);
        setPage(p);
    }
    return p;
}


void WebView::setPage(WebPage *pg)
{
    KWebView::setPage(pg);

    WebWindow *w = m_parentTab->webWindow();
    if (w)
        pg->setWindow(w);
}


bool WebView::popupSpellMenu(QContextMenuEvent *event)
{
    // return false if not handled
    if (!ReKonfig::automaticSpellChecking())
        return false;

    QWebElement element(m_contextMenuHitResult.element());
    if (element.isNull())
        return false;

    int selStart = element.evaluateJavaScript("this.selectionStart").toInt();
    int selEnd = element.evaluateJavaScript("this.selectionEnd").toInt();
    if (selEnd != selStart)
        return false; // selection, handle normally

    // No selection - Spell Checking only
    // Get word
    QString text = element.evaluateJavaScript("this.value").toString();
    QRegExp ws("\\b");
    int s1 = text.lastIndexOf(ws, selStart);
    int s2 = text.indexOf(ws, selStart);
    QString word = text.mid(s1, s2 - s1).trimmed();

    // sanity check
    if (word.isEmpty())
        return false;

    kDebug() << s1 << ":" << s2 << ":" << word << ":";
    Sonnet::Speller spellor;
    if (spellor.isCorrect(word))
        return false; // no need to popup spell menu

    // find alternates
    QStringList words = spellor.suggest(word);

    // Construct popup menu
    QMenu mnu(this);

    // Add alternates
    if (words.isEmpty())
    {
        QAction *a = mnu.addAction(i18n("No suggestions for %1", word));
        a->setEnabled(false);
    }
    else
    {
        QStringListIterator it(words);
        while (it.hasNext())
        {
            QString w = it.next();
            QAction *aWord = mnu.addAction(w);
            aWord->setData(w);
        }
    }

    // Add dictionary options
    mnu.addSeparator();
    QAction *aIgnore = mnu.addAction(i18n("Ignore"));
    QAction *aAddToDict = mnu.addAction(i18n("Add to Dictionary"));

    QAction *aSpellChoice = mnu.exec(event->globalPos());
    if (aSpellChoice)
    {
        if (aSpellChoice == aAddToDict)
            spellor.addToPersonal(word);
        else if (aSpellChoice == aIgnore)
        {
            // Ignore :)
        }
        else
        {
            // Choose a replacement word
            QString w = aSpellChoice->data().toString();
            if (!w.isEmpty())
            {
                // replace word
                QString script(QL1S("this.value=this.value.substring(0,"));
                script += QString::number(s1);
                script += QL1S(") + \'");
                script +=  w.replace('\'', "\\\'"); // Escape any Quote marks in replacement word
                script += QL1C('\'') + QL1S("+this.value.substring(");
                script += QString::number(s2);
                script += QL1C(')');
                
                element.evaluateJavaScript(script);
                // reposition cursor
                element.evaluateJavaScript("this.selectionEnd=this.selectionStart=" + QString::number(selStart) + QL1C(';'));
            }
        }
    }

    return true;
}


void WebView::contextMenuEvent(QContextMenuEvent *event)
{
    m_contextMenuHitResult = page()->mainFrame()->hitTestContent(event->pos());

    if (m_contextMenuHitResult.isContentEditable())
    {
        // Check to see if handled by speller
        if (popupSpellMenu(event))
            return;
    }

    WebWindow *webwin = m_parentTab->webWindow();

    KMenu menu(this);

    KAction *sendByMailAction = new KAction(&menu);
    sendByMailAction->setIcon(KIcon("mail-send"));
    connect(sendByMailAction, SIGNAL(triggered(bool)), this, SLOT(sendByMail()));

    // Choose right context
    int resultHit = 0;
    if (m_contextMenuHitResult.linkUrl().isEmpty())
        resultHit = WebView::EmptySelection;
    else
        resultHit = WebView::LinkSelection;

    if (!m_contextMenuHitResult.pixmap().isNull())
        resultHit |= WebView::ImageSelection;

    if (m_contextMenuHitResult.isContentSelected())
        resultHit = WebView::TextSelection;

    // --------------------------------------------------------------------------------
    // Ok, let's start filling up the menu...

    // is content editable? Add PASTE
    if (m_contextMenuHitResult.isContentEditable())
    {
        menu.addAction(pageAction(KWebPage::Paste));
        menu.addSeparator();
    }

    QAction *a;

    // EMPTY PAGE ACTIONS -------------------------------------------------------------
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

        //Frame
        KActionMenu *frameMenu = new KActionMenu(i18n("Current Frame"), &menu);
        frameMenu->addAction(pageAction(KWebPage::OpenFrameInNewWindow));

        a = new KAction(KIcon("document-print-frame"), i18n("Print Frame"), &menu);
        connect(a, SIGNAL(triggered()), m_parentTab, SLOT(printFrame()));
        frameMenu->addAction(a);

        menu.addAction(frameMenu);

        menu.addSeparator();

        // Page Actions
        menu.addAction(pageAction(KWebPage::SelectAll));

        if (webwin)
            menu.addAction(webwin->actionByName(KStandardAction::name(KStandardAction::SaveAs)));

        if (!KStandardDirs::findExe("kget").isNull() && ReKonfig::kgetList())
        {
            a = new KAction(KIcon("kget"), i18n("List All Links"), &menu);
            connect(a, SIGNAL(triggered(bool)), page(), SLOT(downloadAllContentsWithKGet()));
            menu.addAction(a);
        }

        if (webwin)
        {
            menu.addAction(webwin->actionByName("page_source"));
            menu.addAction(webwin->actionByName("web_inspector"));
        }
    }

    // LINK ACTIONS -------------------------------------------------------------------
    if (resultHit & WebView::LinkSelection)
    {
        // send by mail: link url
        sendByMailAction->setData(m_contextMenuHitResult.linkUrl());
        sendByMailAction->setText(i18n("Share link"));

        a = new KAction(KIcon("tab-new"), i18n("Open in New &Tab"), &menu);
        a->setData(m_contextMenuHitResult.linkUrl());
        connect(a, SIGNAL(triggered(bool)), this, SLOT(openLinkInNewTab()));
        menu.addAction(a);

        a = new KAction(KIcon("window-new"), i18n("Open in New &Window"), &menu);
        a->setData(m_contextMenuHitResult.linkUrl());
        connect(a, SIGNAL(triggered(bool)), this, SLOT(openLinkInNewWindow()));
        menu.addAction(a);

        if (webwin)
        {
            a = new KAction(KIcon("view-media-artist"), i18n("Open in Private &Window"), &menu);
            a->setData(m_contextMenuHitResult.linkUrl());
            connect(a, SIGNAL(triggered(bool)), this, SLOT(openLinkInPrivateWindow()));
            menu.addAction(a);
        }

        menu.addSeparator();

        // Don't show dots if we are NOT going to ask for download path
        a = pageAction(KWebPage::DownloadLinkToDisk);
        if (ReKonfig::askDownloadPath())
            a->setText(i18n("Save Link..."));
        else
            a->setText(i18n("Save Link"));

        menu.addAction(a);
        menu.addAction(pageAction(KWebPage::CopyLinkToClipboard));
    }

    // IMAGE ACTIONS ------------------------------------------------------------------
    if (resultHit & WebView::ImageSelection)
    {
        // send by mail: image url
        sendByMailAction->setData(m_contextMenuHitResult.imageUrl());
        sendByMailAction->setText(i18n("Share image link"));

        menu.addSeparator();

        a = new KAction(KIcon("view-preview"), i18n("&View Image"), &menu);
        a->setData(m_contextMenuHitResult.imageUrl());
        connect(a, SIGNAL(triggered(Qt::MouseButtons,Qt::KeyboardModifiers)),
                this, SLOT(viewImage(Qt::MouseButtons,Qt::KeyboardModifiers)));
        menu.addAction(a);

        a = new KAction(KIcon("document-save"), i18n("Save image as..."), &menu);
        a->setData(m_contextMenuHitResult.imageUrl());
        connect(a, SIGNAL(triggered(Qt::MouseButtons,Qt::KeyboardModifiers)), this, SLOT(saveImage()));
        menu.addAction(a);

        a = new KAction(KIcon("view-media-visualization"), i18n("&Copy Image Location"), &menu);
        a->setData(m_contextMenuHitResult.imageUrl());
        connect(a, SIGNAL(triggered(Qt::MouseButtons,Qt::KeyboardModifiers)), this, SLOT(slotCopyImageLocation()));
        menu.addAction(a);

        if (AdBlockManager::self()->isEnabled())
        {
            a = new KAction(KIcon("preferences-web-browser-adblock"), i18n("Block image"), &menu);
            a->setData(m_contextMenuHitResult.imageUrl());
            connect(a, SIGNAL(triggered(Qt::MouseButtons,Qt::KeyboardModifiers)), this, SLOT(blockImage()));
            menu.addAction(a);
        }
    }

    // ACTIONS FOR TEXT SELECTION -----------------------------------------------------
    if (resultHit & WebView::TextSelection)
    {
        // send by mail: text
        sendByMailAction->setData(selectedText());
        sendByMailAction->setText(i18n("Share selected text"));

        if (m_contextMenuHitResult.isContentEditable())
        {
            // actions for text selected in field
            menu.addAction(pageAction(KWebPage::Cut));
        }

        a = pageAction(KWebPage::Copy);
        if (!m_contextMenuHitResult.linkUrl().isEmpty())
            a->setText(i18n("Copy Text")); //for link
        else
            a->setText(i18n("Copy"));
        menu.addAction(a);

        if (selectedText().contains('.')
                && selectedText().indexOf('.') < selectedText().length()
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

                // open selected text url in a new tab
                QAction * const openInNewTabAction = new KAction(KIcon("tab-new"),
                        i18n("Open '%1' in New Tab", truncatedUrl), &menu);
                openInNewTabAction->setData(QUrl(urlLikeText));
                connect(openInNewTabAction, SIGNAL(triggered(bool)), this, SLOT(openLinkInNewTab()));
                menu.addAction(openInNewTabAction);
                //open selected text url in a new window
                QAction * const openInNewWindowAction = new KAction(KIcon("window-new"),
                        i18n("Open '%1' in New Window", truncatedUrl), &menu);
                openInNewWindowAction->setData(QUrl(urlLikeText));
                connect(openInNewWindowAction, SIGNAL(triggered(bool)), this, SLOT(openLinkInNewWindow()));
                menu.addAction(openInNewWindowAction);
                menu.addSeparator();
            }
        }

        //Default SearchEngine
        KService::Ptr defaultEngine = SearchEngine::defaultEngine();
        if (defaultEngine) // check if a default engine is set
        {
            a = new KAction(i18nc("Search selected text with the default search engine", "Search with %1",
                                  defaultEngine->name()), &menu);
            a->setIcon(IconManager::self()->iconForUrl(SearchEngine::buildQuery(defaultEngine, "")));
            a->setData(defaultEngine->entryPath());
            connect(a, SIGNAL(triggered(bool)), this, SLOT(search()));
            menu.addAction(a);
        }

        //All favourite ones
        KActionMenu *searchMenu = new KActionMenu(KIcon("edit-find"), i18nc("@title:menu", "Search"), &menu);

        Q_FOREACH(const KService::Ptr & engine, SearchEngine::favorites())
        {
            a = new KAction(i18nc("@item:inmenu Search, %1 = search engine", "With %1", engine->name()), &menu);
            a->setIcon(IconManager::self()->iconForUrl(SearchEngine::buildQuery(engine, "")));
            a->setData(engine->entryPath());
            connect(a, SIGNAL(triggered(bool)), this, SLOT(search()));
            searchMenu->addAction(a);
        }

        a = new KAction(KIcon("edit-find"), i18n("On Current Page"), &menu);
        connect(a, SIGNAL(triggered()), webwin, SLOT(findSelectedText()));
        searchMenu->addAction(a);

        if (!searchMenu->menu()->isEmpty())
        {
            menu.addAction(searchMenu);
        }
    }

    // DEFAULT ACTIONs (on the bottom) ------------------------------------------------
    menu.addSeparator();
    if (resultHit & WebView::LinkSelection)
    {
        a = new KAction(KIcon("bookmark-new"), i18n("&Bookmark link"), &menu);
        a->setData(m_contextMenuHitResult.linkUrl());
        connect(a, SIGNAL(triggered(bool)), this, SLOT(bookmarkLink()));
        menu.addAction(a);
    }
    else
    {
        if (webwin)
        {
            a = webwin->actionByName(KStandardAction::name(KStandardAction::AddBookmark));
            menu.addAction(a);
        }
    }

    menu.addAction(sendByMailAction);

    if (webwin)
        menu.addAction(webwin->actionByName("web_inspector"));

    // SPELL CHECK Actions
    if (m_contextMenuHitResult.isContentEditable())
    {
        menu.addSeparator();
        a = KStandardAction::spelling(this, SLOT(spellCheck()), &menu);
        menu.addAction(a);
    }

    // finally launch the menu...
    menu.exec(mapToGlobal(event->pos()));
}


void WebView::mousePressEvent(QMouseEvent *event)
{
    if (m_isViewAutoScrolling)
    {
        m_verticalAutoScrollSpeed = 0;
        m_horizontalAutoScrollSpeed = 0;
        m_autoScrollTimer->stop();
        m_isViewAutoScrolling = false;
        update();
        return;
    }

    QWebHitTestResult result = page()->mainFrame()->hitTestContent(event->pos());
    bool weCanDoMiddleClickActions = !result.isContentEditable()  && result.linkUrl().isEmpty();

    switch (event->button())
    {
    case Qt::XButton1:
        triggerPageAction(KWebPage::Back);
        break;

    case Qt::XButton2:
        triggerPageAction(KWebPage::Forward);
        break;

    case Qt::MidButton:
        switch (ReKonfig::middleClickAction())
        {
        case 0: // AutoScroll
            if (weCanDoMiddleClickActions
                    && !m_isViewAutoScrolling
                    && !page()->currentFrame()->scrollBarGeometry(Qt::Horizontal).contains(event->pos())
                    && !page()->currentFrame()->scrollBarGeometry(Qt::Vertical).contains(event->pos()))
            {
                if (!page()->currentFrame()->scrollBarGeometry(Qt::Horizontal).isNull()
                        || !page()->currentFrame()->scrollBarGeometry(Qt::Vertical).isNull())
                {
                    m_clickPos = event->pos();
                    m_isViewAutoScrolling = true;
                    update();
                }
            }
            break;

        case 1: // Load Clipboard URL
            if (weCanDoMiddleClickActions)
            {
                const QString clipboardContent = QApplication::clipboard()->text();

                if (clipboardContent.isEmpty())
                    break;

                if (QUrl::fromUserInput(clipboardContent).isValid())
                    load(KUrl(clipboardContent));
                else // Search with default Engine
                {
                    KService::Ptr defaultEngine = SearchEngine::defaultEngine();
                    if (defaultEngine) // check if a default engine is set
                        load(KUrl(SearchEngine::buildQuery(defaultEngine, clipboardContent)));
                }
            }
            break;

        default: // Do Nothing
            break;
        }
        break;

    default:
        break;
    };

    KWebView::mousePressEvent(event);
}


void WebView::mouseMoveEvent(QMouseEvent *event)
{
    QPoint mousePos = event->pos();

    if (m_isViewAutoScrolling)
    {
        QPoint r = mousePos - m_clickPos;
        m_horizontalAutoScrollSpeed = r.x() / 2;  // you are too fast..
        m_verticalAutoScrollSpeed = r.y() / 2;
        if (!m_autoScrollTimer->isActive())
            m_autoScrollTimer->start();

        return;
    }

    KWebView::mouseMoveEvent(event);
}


void WebView::dropEvent(QDropEvent *event)
{
    bool isEditable = page()->frameAt(event->pos())->hitTestContent(event->pos()).isContentEditable();
    if (event->mimeData()->hasFormat(BookmarkManager::bookmark_mime_type()))
    {
        QByteArray addresses = event->mimeData()->data(BookmarkManager::bookmark_mime_type());
        KBookmark bookmark =  BookmarkManager::self()->findByAddress(QString::fromLatin1(addresses.data()));
        if (bookmark.isGroup())
        {
            BookmarkManager::self()->openFolderinTabs(bookmark.toGroup());
        }
        else
        {
            load(bookmark.url());
        }
    }
    else if (event->mimeData()->hasUrls() && event->source() != this && !isEditable) //dropped links
    {
        Q_FOREACH(const QUrl & url, event->mimeData()->urls())
        {
            emit loadUrl(url, Rekonq::NewFocusedTab);
        }
    }
    else if (event->mimeData()->hasFormat("text/plain") && event->source() != this && !isEditable) //dropped plain text with url format
    {
        QUrl url = QUrl::fromUserInput(event->mimeData()->data("text/plain"));

        if (url.isValid())
            emit loadUrl(url, Rekonq::NewFocusedTab);
    }
    else
    {
        KWebView::dropEvent(event);
    }
}


void WebView::paintEvent(QPaintEvent* event)
{
    KWebView::paintEvent(event);

    if (m_isViewAutoScrolling)
    {
        QPoint centeredPoint = m_clickPos;
        centeredPoint.setX(centeredPoint.x() - m_autoScrollIndicator.width() / 2);
        centeredPoint.setY(centeredPoint.y() - m_autoScrollIndicator.height() / 2);

        QPainter painter(this);
        painter.setOpacity(0.8);
        painter.drawPixmap(centeredPoint, m_autoScrollIndicator);
    }
}


void WebView::search()
{
    KAction *a = qobject_cast<KAction*>(sender());
    KService::Ptr engine = KService::serviceByDesktopPath(a->data().toString());
    KUrl urlSearch = KUrl(SearchEngine::buildQuery(engine, selectedText()));

    emit loadUrl(urlSearch, Rekonq::NewTab);
}


void WebView::viewImage(Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers)
{
    KAction *a = qobject_cast<KAction*>(sender());
    KUrl url(a->data().toUrl());

    if (modifiers & Qt::ControlModifier || buttons == Qt::MidButton)
    {
        emit loadUrl(url, Rekonq::NewTab);
    }
    else
    {
        load(url);
    }
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


void WebView::openLinkInNewWindow()
{
    KAction *a = qobject_cast<KAction*>(sender());
    KUrl url(a->data().toUrl());

    emit loadUrl(url, Rekonq::NewWindow);
}


void WebView::openLinkInNewTab()
{
    KAction *a = qobject_cast<KAction*>(sender());
    KUrl url(a->data().toUrl());

    WebWindow *webwin = m_parentTab->webWindow();

    if (webwin)
        emit loadUrl(url, Rekonq::NewTab);
    else
        emit loadUrl(url, Rekonq::NewFocusedTab);
}


void WebView::openLinkInPrivateWindow()
{
    KAction *a = qobject_cast<KAction*>(sender());
    KUrl url(a->data().toUrl());

    emit loadUrl(url, Rekonq::NewPrivateWindow);
}


void WebView::bookmarkLink()
{
    KAction *a = qobject_cast<KAction*>(sender());
    KUrl url(a->data().toUrl());

    BookmarkManager::self()->rootGroup().addBookmark(url.prettyUrl(), url);
    BookmarkManager::self()->emitChanged();
}


void WebView::keyPressEvent(QKeyEvent *event)
{
    // If CTRL was hit, be prepared for access keys
    if (ReKonfig::accessKeysEnabled()
            && !m_accessKeysActive
            && event->key() == Qt::Key_Control
            && !(event->modifiers() & ~Qt::ControlModifier)
       )
    {
        m_accessKeysPressed = true;
        event->accept();
        return;
    }

    const QString tagName = page()->mainFrame()->evaluateJavaScript("document.activeElement.tagName").toString();

    if (event->modifiers() == Qt::ControlModifier)
    {
        if (event->key() == Qt::Key_C)
        {
            triggerPageAction(KWebPage::Copy);
            event->accept();
            return;
        }

        if (event->key() == Qt::Key_A)
        {
            triggerPageAction(KWebPage::SelectAll);
            event->accept();
            return;
        }

        // CTRL + RETURN: open link into another tab
        if (event->key() == Qt::Key_Return && tagName == QL1S("A"))
        {
            KUrl u = KUrl(page()->mainFrame()->evaluateJavaScript("document.activeElement.attributes[\"href\"].value").toString());
            emit loadUrl(u, Rekonq::NewTab);
            event->accept();
            return;
        }
    }

    // Auto Scrolling
    if (event->modifiers() == Qt::ShiftModifier
            && tagName != QL1S("INPUT")
            && tagName != QL1S("TEXTAREA")
       )
    {
        // NOTE and FIXME
        // This check is doabled because it presents strange behavior: QtWebKit check works well in pages like gmail
        // and fails on sites like g+. The opposite is true for javascript check.
        // Please, help me finding the right way to check this EVERY TIME.
        bool isContentEditableQW = page()->mainFrame()->hitTestContent(QCursor::pos()).isContentEditable();
        bool isContentEditableJS = page()->mainFrame()->evaluateJavaScript("document.activeElement.isContentEditable").toBool();

        if (!isContentEditableQW && !isContentEditableJS)
        {
            if (event->key() == Qt::Key_Up)
            {
                m_verticalAutoScrollSpeed--;
                if (!m_autoScrollTimer->isActive())
                    m_autoScrollTimer->start();

                event->accept();
                return;
            }

            if (event->key() == Qt::Key_Down)
            {
                m_verticalAutoScrollSpeed++;
                if (!m_autoScrollTimer->isActive())
                    m_autoScrollTimer->start();

                event->accept();
                return;
            }

            if (event->key() == Qt::Key_Right)
            {
                m_horizontalAutoScrollSpeed++;
                if (!m_autoScrollTimer->isActive())
                    m_autoScrollTimer->start();

                event->accept();
                return;
            }

            if (event->key() == Qt::Key_Left)
            {
                m_horizontalAutoScrollSpeed--;
                if (!m_autoScrollTimer->isActive())
                    m_autoScrollTimer->start();

                event->accept();
                return;
            }

            if (m_autoScrollTimer->isActive())
            {
                m_autoScrollTimer->stop();
                event->accept();
                return;
            }
            else
            {
                if (m_verticalAutoScrollSpeed || m_horizontalAutoScrollSpeed)
                {
                    m_autoScrollTimer->start();
                    event->accept();
                    return;
                }
            }
        }

        // if you arrived here, I hope it means SHIFT has been pressed NOT for autoscroll management...
    }

    if (ReKonfig::accessKeysEnabled() && m_accessKeysActive)
    {
        hideAccessKeys();
        event->accept();
        return;
    }

    // vi-like navigation
    if (ReKonfig::enableViShortcuts())
    {
        if (event->modifiers() == Qt::NoModifier
                && tagName != QL1S("INPUT")
                && tagName != QL1S("TEXTAREA")
           )
        {
            // See note up!
            bool isContentEditableQW = page()->mainFrame()->hitTestContent(QCursor::pos()).isContentEditable();
            bool isContentEditableJS = page()->mainFrame()->evaluateJavaScript("document.activeElement.isContentEditable").toBool();

            if (!isContentEditableQW && !isContentEditableJS)
            {
                switch (event->key())
                {
                case Qt::Key_J:
                    event->accept();
                    event = new QKeyEvent(QEvent::KeyPress, Qt::Key_Down, Qt::NoModifier);
                    break;
                case Qt::Key_K:
                    event->accept();
                    event = new QKeyEvent(QEvent::KeyPress, Qt::Key_Up, Qt::NoModifier);
                    break;
                case Qt::Key_L:
                    event->accept();
                    event = new QKeyEvent(QEvent::KeyPress, Qt::Key_Right, Qt::NoModifier);
                    break;
                case Qt::Key_H:
                    event->accept();
                    event = new QKeyEvent(QEvent::KeyPress, Qt::Key_Left, Qt::NoModifier);
                    break;
                default:
                    break;
                }
            }
        }
    }

    KWebView::keyPressEvent(event);
}


void WebView::keyReleaseEvent(QKeyEvent *event)
{
    // access keys management
    if (ReKonfig::accessKeysEnabled())
    {
        if (m_accessKeysPressed && event->key() != Qt::Key_Control)
            m_accessKeysPressed = false;

        if (m_accessKeysPressed && !(event->modifiers() & Qt::ControlModifier))
        {
            kDebug() << "Shotting access keys";
            QTimer::singleShot(200, this, SLOT(accessKeyShortcut()));
            event->accept();
            return;
        }
        else
        {
            checkForAccessKey(event);
            kDebug() << "Hiding access keys";
            hideAccessKeys();
            event->accept();
            return;
        }
    }

    KWebView::keyReleaseEvent(event);
}


void WebView::wheelEvent(QWheelEvent *event)
{
    if (event->orientation() == Qt::Vertical || !ReKonfig::hScrollWheelHistory())
    {
        // To let some websites (eg: google maps) to handle wheel events
        int prevPos = page()->currentFrame()->scrollPosition().y();
        KWebView::wheelEvent(event);
        int newPos = page()->currentFrame()->scrollPosition().y();

        // Sync with the zoom slider
        if (event->modifiers() == Qt::ControlModifier)
        {
            // Limits of the slider
            if (zoomFactor() > 1.9)
                setZoomFactor(1.9);
            else if (zoomFactor() < 0.1)
                setZoomFactor(0.1);

            // Round the factor (Fix slider's end value)
            int newFactor = zoomFactor() * 10;
            if ((zoomFactor() * 10 - newFactor) > 0.5)
                newFactor++;

            emit zoomChanged(newFactor);
        }
        else if (ReKonfig::smoothScrolling() && prevPos != newPos)
        {
            page()->currentFrame()->setScrollPosition(QPoint(page()->currentFrame()->scrollPosition().x(), prevPos));

            if ((event->delta() > 0) != !m_smoothScrollBottomReached)
                stopSmoothScrolling();

            if (event->delta() > 0)
                m_smoothScrollBottomReached = false;
            else
                m_smoothScrollBottomReached = true;


            setupSmoothScrolling(abs(newPos - prevPos));
        }
    }
    // use horizontal wheel events to go back and forward in tab history
    else
    {
        // left -> go to previous page
        if (event->delta() > 0)
        {
            emit openPreviousInHistory();
        }
        // right -> go to next page
        if (event->delta() < 0)
        {
            emit openNextInHistory();
        }
    }
}


void WebView::scrollFrameChanged()
{
    // do the scrolling
    page()->currentFrame()->scroll(m_horizontalAutoScrollSpeed, m_verticalAutoScrollSpeed);

    // check if we reached the end
    int y = page()->currentFrame()->scrollPosition().y();
    if (y == 0 || y == page()->currentFrame()->scrollBarMaximum(Qt::Vertical))
        m_verticalAutoScrollSpeed = 0;

    int x = page()->currentFrame()->scrollPosition().x();
    if (x == 0 || x == page()->currentFrame()->scrollBarMaximum(Qt::Horizontal))
        m_horizontalAutoScrollSpeed = 0;
}


void WebView::setupSmoothScrolling(int posY)
{
    int ddy = qMax(m_smoothScrollSteps ? abs(m_dy) / m_smoothScrollSteps : 0, 3);

    m_dy += posY;

    if (m_dy <= 0)
    {
        stopSmoothScrolling();
        return;
    }

    m_smoothScrollSteps = 8;

    if (m_dy / m_smoothScrollSteps < ddy)
    {
        m_smoothScrollSteps = (abs(m_dy) + ddy - 1) / ddy;
        if (m_smoothScrollSteps < 1)
            m_smoothScrollSteps = 1;
    }

    m_smoothScrollTime.start();

    if (!m_isViewSmoothScrolling)
    {
        m_isViewSmoothScrolling = true;
        m_smoothScrollTimer->start();
        scrollTick();
    }
}


void WebView::scrollTick()
{
    if (m_dy == 0)
    {
        stopSmoothScrolling();
        return;
    }

    if (m_smoothScrollSteps < 1)
        m_smoothScrollSteps = 1;

    int takesteps = m_smoothScrollTime.restart() / 16;
    int scroll_y = 0;

    if (takesteps < 1)
        takesteps = 1;

    if (takesteps > m_smoothScrollSteps)
        takesteps = m_smoothScrollSteps;

    for (int i = 0; i < takesteps; i++)
    {
        int ddy = (m_dy / (m_smoothScrollSteps + 1)) * 2;

        // limit step to requested scrolling distance
        if (abs(ddy) > abs(m_dy))
            ddy = m_dy;

        // update remaining scroll
        m_dy -= ddy;
        scroll_y += ddy;
        m_smoothScrollSteps--;
    }

    if (m_smoothScrollBottomReached)
        page()->currentFrame()->scroll(0, scroll_y);
    else
        page()->currentFrame()->scroll(0, -scroll_y);
}


void WebView::stopSmoothScrolling()
{
    m_smoothScrollTimer->stop();
    m_dy = 0;
    m_isViewSmoothScrolling = false;
}


void WebView::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls() || event->mimeData()->hasText())
        event->acceptProposedAction();
    else
        KWebView::dragEnterEvent(event);
}


void WebView::dragMoveEvent(QDragMoveEvent *event)
{
    if (event->mimeData()->hasUrls() || event->mimeData()->hasText())
        event->acceptProposedAction();
    else
        KWebView::dragMoveEvent(event);
}


void WebView::hideAccessKeys()
{
    if (!m_accessKeyLabels.isEmpty())
    {
        for (int i = 0; i < m_accessKeyLabels.count(); ++i)
        {
            QLabel *label = m_accessKeyLabels[i];
            label->hide();
            label->deleteLater();
        }
        m_accessKeyLabels.clear();
        m_accessKeyNodes.clear();
        update();
    }
}


void WebView::showAccessKeys()
{
    QStringList supportedElement;
    supportedElement << QLatin1String("a")
                     << QLatin1String("input")
                     << QLatin1String("area")
                     << QLatin1String("button")
                     << QLatin1String("label")
                     << QLatin1String("legend")
                     << QLatin1String("textarea");

    QList<QChar> unusedKeys;
    for (char c = 'A'; c <= 'Z'; ++c)
        unusedKeys << QLatin1Char(c);
    for (char c = '0'; c <= '9'; ++c)
        unusedKeys << QLatin1Char(c);

    QRect viewport = QRect(page()->mainFrame()->scrollPosition(), page()->viewportSize());
    // Priority first goes to elements with accesskey attributes
    QList<QWebElement> alreadyLabeled;
    Q_FOREACH(const QString & elementType, supportedElement)
    {
        QList<QWebElement> result = page()->mainFrame()->findAllElements(elementType).toList();
        Q_FOREACH(const QWebElement & element, result)
        {
            const QRect geometry = element.geometry();
            if (geometry.size().isEmpty()
                    || !viewport.contains(geometry.topLeft()))
            {
                continue;
            }
            QString accessKeyAttribute = element.attribute(QLatin1String("accesskey")).toUpper();
            if (accessKeyAttribute.isEmpty())
                continue;
            QChar accessKey;
            for (int i = 0; i < accessKeyAttribute.count(); i += 2)
            {
                const QChar &possibleAccessKey = accessKeyAttribute[i];
                if (unusedKeys.contains(possibleAccessKey))
                {
                    accessKey = possibleAccessKey;
                    break;
                }
            }
            if (accessKey.isNull())
            {
                continue;
            }
            unusedKeys.removeOne(accessKey);
            makeAccessKeyLabel(accessKey, element);
            alreadyLabeled.append(element);
        }
    }

    // Pick an access key first from the letters in the text and then from the
    // list of unused access keys
    Q_FOREACH(const QString & elementType, supportedElement)
    {
        QWebElementCollection result = page()->mainFrame()->findAllElements(elementType);
        Q_FOREACH(const QWebElement & element, result)
        {
            const QRect geometry = element.geometry();
            if (unusedKeys.isEmpty()
                    || alreadyLabeled.contains(element)
                    || geometry.size().isEmpty()
                    || !viewport.contains(geometry.topLeft()))
            {
                continue;
            }
            QChar accessKey;
            QString text = element.toPlainText().toUpper();
            for (int i = 0; i < text.count(); ++i)
            {
                const QChar &c = text.at(i);
                if (unusedKeys.contains(c))
                {
                    accessKey = c;
                    break;
                }
            }
            if (accessKey.isNull())
                accessKey = unusedKeys.takeFirst();
            unusedKeys.removeOne(accessKey);
            makeAccessKeyLabel(accessKey, element);
        }
    }
}


void WebView::makeAccessKeyLabel(const QChar &accessKey, const QWebElement &element)
{
    QLabel *label = new QLabel(this);
    label->setText(QString(QLatin1String("<qt><b>%1</b>")).arg(accessKey));

    label->setAutoFillBackground(true);
    label->setFrameStyle(QFrame::Box | QFrame::Plain);
    QPoint point = element.geometry().center();
    point -= page()->mainFrame()->scrollPosition();
    label->move(point);
    label->show();
    point.setX(point.x() - label->width() / 2);
    label->move(point);
    m_accessKeyLabels.append(label);
    m_accessKeyNodes[accessKey] = element;
}


bool WebView::checkForAccessKey(QKeyEvent *event)
{
    if (m_accessKeyLabels.isEmpty())
        return false;

    QString text = event->text();
    if (text.isEmpty())
        return false;
    QChar key = text.at(0).toUpper();
    bool handled = false;
    if (m_accessKeyNodes.contains(key))
    {
        QWebElement element = m_accessKeyNodes[key];
        QPoint p = element.geometry().center();
        QWebFrame *frame = element.webFrame();
        Q_ASSERT(frame);
        do
        {
            p -= frame->scrollPosition();
            frame = frame->parentFrame();
        }
        while (frame && frame != page()->mainFrame());
        QMouseEvent pevent(QEvent::MouseButtonPress, p, Qt::LeftButton, 0, 0);
        QApplication::sendEvent(this, &pevent);
        QMouseEvent revent(QEvent::MouseButtonRelease, p, Qt::LeftButton, 0, 0);
        QApplication::sendEvent(this, &revent);
        handled = true;
    }

    kDebug() << "checking for access keys: " << handled;
    return handled;
}


void WebView::accessKeyShortcut()
{
    if (!hasFocus()
            || !m_accessKeysPressed
            || !ReKonfig::accessKeysEnabled())
        return;
    if (m_accessKeyLabels.isEmpty())
    {
        showAccessKeys();
    }
    else
    {
        hideAccessKeys();
    }
    m_accessKeysPressed = false;
}


void WebView::sendByMail()
{
    KAction *a = qobject_cast<KAction*>(sender());
    QString url = a->data().toString();

    KToolInvocation::invokeMailer("", "", "", "", url);
}


void WebView::blockImage()
{
    QAction *action = qobject_cast<QAction*>(sender());
    if (!action)
        return;

    QString imageUrl = action->data().toString();
    AdBlockManager::self()->addCustomRule(imageUrl);
}


void WebView::mouseReleaseEvent(QMouseEvent *event)
{
    QWebHitTestResult hitTest = page()->mainFrame()->hitTestContent(event->pos());
    const QUrl url = hitTest.linkUrl();

    if (!url.isEmpty())
    {
        if (event->button() & Qt::MidButton)
        {
            // open tab as focused
            if (event->modifiers() & Qt::ShiftModifier)
            {
                emit loadUrl(url, Rekonq::NewFocusedTab);
                event->accept();
                return;
            }

            emit loadUrl(url, Rekonq::NewTab);
            event->accept();
            return;
        }

        if ((event->button() & Qt::LeftButton) && (event->modifiers() & Qt::ControlModifier))
        {
            emit loadUrl(url, Rekonq::NewTab);
            event->accept();
            return;
        }

        if ((event->button() & Qt::LeftButton) && (event->modifiers() & Qt::ShiftModifier))
        {
            page()->downloadUrl(url);
            event->accept();
            return;
        }
    }

    QWebView::mouseReleaseEvent(event);
}


void WebView::spellCheck()
{
    QString text(execJScript(m_contextMenuHitResult, QL1S("this.value")).toString());

    if (m_contextMenuHitResult.isContentSelected())
    {
        m_spellTextSelectionStart = qMax(0, execJScript(m_contextMenuHitResult, QL1S("this.selectionStart")).toInt());
        m_spellTextSelectionEnd = qMax(0, execJScript(m_contextMenuHitResult, QL1S("this.selectionEnd")).toInt());
        text = text.mid(m_spellTextSelectionStart, (m_spellTextSelectionEnd - m_spellTextSelectionStart));
    }
    else
    {
        m_spellTextSelectionStart = 0;
        m_spellTextSelectionEnd = 0;
    }

    if (text.isEmpty())
    {
        return;
    }

    Sonnet::BackgroundChecker *backgroundSpellCheck = new Sonnet::BackgroundChecker;
    Sonnet::Dialog* spellDialog = new Sonnet::Dialog(backgroundSpellCheck, this);
    backgroundSpellCheck->setParent(spellDialog);
    spellDialog->setAttribute(Qt::WA_DeleteOnClose, true);

    connect(spellDialog, SIGNAL(replace(QString,int,QString)), this, SLOT(spellCheckerCorrected(QString,int,QString)));
    connect(spellDialog, SIGNAL(misspelling(QString,int)), this, SLOT(spellCheckerMisspelling(QString,int)));
    if (m_contextMenuHitResult.isContentSelected())
        connect(spellDialog, SIGNAL(done(QString)), this, SLOT(slotSpellCheckDone(QString)));
    spellDialog->setBuffer(text);
    spellDialog->show();
}


void WebView::spellCheckerCorrected(const QString& original, int pos, const QString& replacement)
{
    // Adjust the selection end...
    if (m_spellTextSelectionEnd > 0)
    {
        m_spellTextSelectionEnd += qMax(0, (replacement.length() - original.length()));
    }

    const int index = pos + m_spellTextSelectionStart;
    QString script(QL1S("this.value=this.value.substring(0,"));
    script += QString::number(index);
    script += QL1S(") + \"");
    QString w(replacement);
    script +=  w.replace('\'', "\\\'"); // Escape any Quote marks in replacement word
    script += QL1S("\" + this.value.substring(");
    script += QString::number(index + original.length());
    script += QL1S(")");

    //kDebug() << "**** script:" << script;
    execJScript(m_contextMenuHitResult, script);
}


void WebView::spellCheckerMisspelling(const QString& text, int pos)
{
    // kDebug() << text << pos;
    QString selectionScript(QL1S("this.setSelectionRange("));
    selectionScript += QString::number(pos + m_spellTextSelectionStart);
    selectionScript += QL1C(',');
    selectionScript += QString::number(pos + text.length() + m_spellTextSelectionStart);
    selectionScript += QL1C(')');
    execJScript(m_contextMenuHitResult, selectionScript);
}


void WebView::slotSpellCheckDone(const QString&)
{
    // Restore the text selection if one was present before we started the
    // spell check.
    if (m_spellTextSelectionStart > 0 || m_spellTextSelectionEnd > 0)
    {
        QString script(QL1S("; this.setSelectionRange("));
        script += QString::number(m_spellTextSelectionStart);
        script += QL1C(',');
        script += QString::number(m_spellTextSelectionEnd);
        script += QL1C(')');
        execJScript(m_contextMenuHitResult, script);
    }
}


WebTab *WebView::parentTab()
{
    return m_parentTab;
}


void WebView::saveImage()
{
    KAction *a = qobject_cast<KAction*>(sender());
    KUrl imageUrl(a->data().toUrl());

    DownloadManager::self()->downloadResource(imageUrl,
            KIO::MetaData(),
            this,
            true,
            QString(),
            !settings()->testAttribute(QWebSettings::PrivateBrowsingEnabled));
}
