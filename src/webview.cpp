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

// Local Includes
#include "application.h"
#include "adblockmanager.h"
#include "bookmarkmanager.h"
#include "iconmanager.h"
#include "mainview.h"
#include "mainwindow.h"
#include "searchengine.h"
#include "urlbar.h"
#include "webpage.h"
#include "webtab.h"

// KDE Includes
#include <KAction>
#include <KActionMenu>
#include <KLocalizedString>
#include <KMenu>
#include <KStandardAction>
#include <KStandardDirs>
#include <KToolInvocation>

// Qt Includes
#include <QFile>
#include <QTimer>

#include <QClipboard>
#include <QContextMenuEvent>
#include <QLabel>

#include <QWebFrame>
#include <QWebHistory>
#include <QNetworkRequest>


WebView::WebView(QWidget* parent)
    : KWebView(parent, false)
    , m_page(0)
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
{
    // loadUrl signal
    connect(this, SIGNAL(loadUrl(KUrl, Rekonq::OpenType)), rApp, SLOT(loadUrl(KUrl, Rekonq::OpenType)));

    // Auto scroll timer
    connect(m_autoScrollTimer, SIGNAL(timeout()), this, SLOT(scrollFrameChanged()));
    m_autoScrollTimer->setInterval(100);

    // Smooth scroll timer
    connect(m_smoothScrollTimer, SIGNAL(timeout()), this, SLOT(scrollTick()));
    m_smoothScrollTimer->setInterval(16);

    connect(this, SIGNAL(iconChanged()), this, SLOT(changeWindowIcon()));
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


void WebView::changeWindowIcon()
{
    if (ReKonfig::useFavicon())
    {
        MainWindow *const mainWindow = rApp->mainWindow();
        if (url() == mainWindow->currentTab()->url())
        {
            const int index = mainWindow->mainView()->currentIndex();
            mainWindow->changeWindowIcon(index);
        }
    }
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


void WebView::contextMenuEvent(QContextMenuEvent *event)
{
    QWebHitTestResult result = page()->mainFrame()->hitTestContent(event->pos());
    MainWindow *mainwindow = rApp->mainWindow();

    KMenu menu(this);
    QAction *a;

    KAction *inspectAction = new KAction(KIcon("layer-visible-on"), i18n("Inspect Element"), this);
    connect(inspectAction, SIGNAL(triggered(bool)), this, SLOT(inspect()));

    KAction *sendByMailAction = new KAction(this);
    sendByMailAction->setIcon(KIcon("mail-send"));
    connect(sendByMailAction, SIGNAL(triggered(bool)), this, SLOT(sendByMail()));

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

    // --------------------------------------------------------------------------------
    // Ok, let's start filling up the menu...

    // is content editable? Add PASTE
    if (result.isContentEditable())
    {
        menu.addAction(pageAction(KWebPage::Paste));
        menu.addSeparator();
    }

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

        menu.addAction(mainwindow->actionByName("view_redisplay"));

        menu.addSeparator();

        //Frame
        KActionMenu *frameMenu = new KActionMenu(i18n("Current Frame"), this);
        frameMenu->addAction(pageAction(KWebPage::OpenFrameInNewWindow));

        a = new KAction(KIcon("document-print-frame"), i18n("Print Frame"), this);
        connect(a, SIGNAL(triggered()), this, SLOT(printFrame()));
        frameMenu->addAction(a);

        menu.addAction(frameMenu);

        menu.addSeparator();

        // Page Actions
        menu.addAction(pageAction(KWebPage::SelectAll));

        menu.addAction(mainwindow->actionByName(KStandardAction::name(KStandardAction::SaveAs)));

        if (!KStandardDirs::findExe("kget").isNull() && ReKonfig::kgetList())
        {
            a = new KAction(KIcon("kget"), i18n("List All Links"), this);
            connect(a, SIGNAL(triggered(bool)), page(), SLOT(downloadAllContentsWithKGet()));
            menu.addAction(a);
        }

        menu.addAction(mainwindow->actionByName("page_source"));
        menu.addAction(inspectAction);

        if (mainwindow->isFullScreen())
        {
            menu.addSeparator();
            menu.addAction(mainwindow->actionByName("fullscreen"));
        }
    }

    // LINK ACTIONS -------------------------------------------------------------------
    if (resultHit & WebView::LinkSelection)
    {
        // send by mail: link url
        sendByMailAction->setData(result.linkUrl());
        sendByMailAction->setText(i18n("Share link"));

        a = new KAction(KIcon("tab-new"), i18n("Open in New &Tab"), this);
        a->setData(result.linkUrl());
        connect(a, SIGNAL(triggered(bool)), this, SLOT(openLinkInNewTab()));
        menu.addAction(a);

        a = new KAction(KIcon("window-new"), i18n("Open in New &Window"), this);
        a->setData(result.linkUrl());
        connect(a, SIGNAL(triggered(bool)), this, SLOT(openLinkInNewWindow()));
        menu.addAction(a);

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

        if (rApp->adblockManager()->isEnabled())
        {
            a = new KAction(KIcon("preferences-web-browser-adblock"), i18n("Block image"), this);
            a->setData(result.imageUrl());
            connect(a, SIGNAL(triggered(Qt::MouseButtons, Qt::KeyboardModifiers)), this, SLOT(blockImage()));
            menu.addAction(a);
        }
    }

    // ACTIONS FOR TEXT SELECTION -----------------------------------------------------
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
                //open selected text url in a new tab
                QAction * const openInNewTabAction = new KAction(KIcon("tab-new"), i18n("Open '%1' in New Tab", truncatedUrl), this);
                openInNewTabAction->setData(QUrl(urlLikeText));
                connect(openInNewTabAction, SIGNAL(triggered(bool)), this, SLOT(openLinkInNewTab()));
                menu.addAction(openInNewTabAction);
                //open selected text url in a new window
                QAction * const openInNewWindowAction = new KAction(KIcon("window-new"), i18n("Open '%1' in New Window", truncatedUrl), this);
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
            a = new KAction(i18nc("Search selected text with the default search engine", "Search with %1", defaultEngine->name()), this);
            a->setIcon(rApp->iconManager()->iconForUrl(SearchEngine::buildQuery(defaultEngine, "")));
            a->setData(defaultEngine->entryPath());
            connect(a, SIGNAL(triggered(bool)), this, SLOT(search()));
            menu.addAction(a);
        }

        //All favourite ones
        KActionMenu *searchMenu = new KActionMenu(KIcon("edit-find"), i18nc("@title:menu", "Search"), this);

        Q_FOREACH(const KService::Ptr & engine, SearchEngine::favorites())
        {
            a = new KAction(i18nc("@item:inmenu Search, %1 = search engine", "With %1", engine->name()), this);
            a->setIcon(rApp->iconManager()->iconForUrl(SearchEngine::buildQuery(engine, "")));
            a->setData(engine->entryPath());
            connect(a, SIGNAL(triggered(bool)), this, SLOT(search()));
            searchMenu->addAction(a);
        }

        a = new KAction(KIcon("edit-find"), i18n("On Current Page"), this);
        connect(a, SIGNAL(triggered()), rApp->mainWindow(), SLOT(findSelectedText()));
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
        a = new KAction(KIcon("bookmark-new"), i18n("&Bookmark link"), this);
        a->setData(result.linkUrl());
        connect(a, SIGNAL(triggered(bool)), this, SLOT(bookmarkLink()));
        menu.addAction(a);
    }
    else
    {
        a = rApp->bookmarkManager()->actionByName("rekonq_add_bookmark");
        menu.addAction(a);
    }
    menu.addAction(sendByMailAction);
    menu.addAction(inspectAction);

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
                const QString clipboardContent = rApp->clipboard()->text();

                if (clipboardContent.isEmpty())
                    break;

                if (QUrl::fromUserInput(clipboardContent).isValid())
                    loadUrl(clipboardContent, Rekonq::CurrentTab);
                else // Search with default Engine
                {
                    KService::Ptr defaultEngine = SearchEngine::defaultEngine();
                    if (defaultEngine) // check if a default engine is set
                        loadUrl(KUrl(SearchEngine::buildQuery(defaultEngine, clipboardContent)), Rekonq::CurrentTab);
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

    MainWindow *w = rApp->mainWindow();
    if (w->isFullScreen())
    {
        if (event->pos().y() >= 0 && event->pos().y() <= 4)
        {
            w->setWidgetsVisible(true);
        }
        else
        {
            if (!w->mainView()->currentUrlBar()->hasFocus())
                w->setWidgetsVisible(false);
        }
    }
    KWebView::mouseMoveEvent(event);
}


void WebView::dropEvent(QDropEvent *event)
{
    bool isEditable = page()->frameAt(event->pos())->hitTestContent(event->pos()).isContentEditable();
    if (event->mimeData()->hasFormat(BookmarkManager::bookmark_mime_type()))
    {
        QByteArray addresses = event->mimeData()->data(BookmarkManager::bookmark_mime_type());
        KBookmark bookmark =  rApp->bookmarkManager()->findByAddress(QString::fromLatin1(addresses.data()));
        if (bookmark.isGroup())
        {
            rApp->bookmarkManager()->openFolderinTabs(bookmark.toGroup());
        }
        else
        {
            emit loadUrl(bookmark.url(), Rekonq::CurrentTab);
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


void WebView::printFrame()
{
    rApp->mainWindow()->printRequested(page()->currentFrame());
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
        emit loadUrl(url, Rekonq::CurrentTab);
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

    emit loadUrl(url, Rekonq::NewTab);
}


void WebView::bookmarkLink()
{
    KAction *a = qobject_cast<KAction*>(sender());
    KUrl url(a->data().toUrl());

    rApp->bookmarkManager()->rootGroup().addBookmark(url.prettyUrl(), url);
    rApp->bookmarkManager()->emitChanged();
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


void WebView::inspect()
{
    QAction *a = rApp->mainWindow()->actionByName("web_inspector");
    if (a && !a->isChecked())
        a->trigger();
    pageAction(QWebPage::InspectElement)->trigger();
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
        rApp->sendEvent(this, &pevent);
        QMouseEvent revent(QEvent::MouseButtonRelease, p, Qt::LeftButton, 0, 0);
        rApp->sendEvent(this, &revent);
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
    rApp->adblockManager()->addCustomRule(imageUrl);
}


void WebView::mouseReleaseEvent(QMouseEvent *event)
{
    QWebHitTestResult hitTest = page()->mainFrame()->hitTestContent(event->pos());
    const QUrl url = hitTest.linkUrl();

    if (!url.isEmpty())
    {
        if (event->button() & Qt::MidButton)
        {
            if (event->modifiers() & Qt::ShiftModifier)
            {
                if (ReKonfig::openNewTabsInBackground())
                    emit loadUrl(url, Rekonq::NewFocusedTab);
                else
                    emit loadUrl(url, Rekonq::NewBackGroundTab);
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
