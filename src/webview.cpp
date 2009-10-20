/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2008-2009 by Andrea Diamantini <adjam7 at gmail dot com>
* Copyright (C) 2009 by Lionel Chauvin <megabigbug@yahoo.fr>
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
#include "mainwindow.h"
#include "mainview.h"
#include "webpage.h"

// KDE Includes
#include <KUrl>
#include <KDebug>
#include <KService>
#include <KUriFilterData>
#include <KStandardShortcut>

// Qt Includes
#include <QtGui/QContextMenuEvent>
#include <QtGui/QWheelEvent>
#include <QtGui/QMouseEvent>
#include <QtGui/QClipboard>
#include <QtGui/QKeyEvent>
#include <QtGui/QAction>
#include <QtCore/QTimer>


WebView::WebView(QWidget* parent)
        : QWebView(parent)
        , m_page(new WebPage(this))
        , m_progress(0)
        , m_scrollTimer(new QTimer(this))
        , m_scrollDirection(WebView::NoScroll)
        , m_scrollSpeedVertical(0)
        , m_scrollSpeedHorizontal(0)

{
    setPage(m_page);
    
    connect(page(), SIGNAL(statusBarMessage(const QString&)), this, SLOT(setStatusBarText(const QString&)));
    connect(this, SIGNAL(loadProgress(int)), this, SLOT(slotUpdateProgress(int)));
    connect(this, SIGNAL(loadFinished(bool)), this, SLOT(slotLoadFinished(bool)));
    connect(this, SIGNAL(selectionChanged()), this, SLOT(selectionChanged()));

    connect(m_scrollTimer, SIGNAL(timeout()), this, SLOT(scrollFrameChanged()));
    m_scrollTimer->setInterval(50);

}


WebView::~WebView()
{
}


WebPage *WebView::page()
{
    if(!m_page)
    {
        m_page = new WebPage();
        setPage(m_page);
    }
    return m_page;
}


KUrl WebView::url() const 
{ 
    return KUrl(QWebView::url()); 
}


int WebView::progress()
{
    return m_progress;
}


QString WebView::lastStatusBarText() const
{ 
    return m_statusBarText; 
}


void WebView::setStatusBarText(const QString &string) 
{ 
    m_statusBarText = string; 
}


void WebView::contextMenuEvent(QContextMenuEvent *event)
{
    QWebHitTestResult result = page()->mainFrame()->hitTestContent(event->pos());
    MainWindow *mainwindow = Application::instance()->mainWindow();

    KMenu menu(this);
    QAction *a;

    // is a link?
    if (!result.linkUrl().isEmpty())
    {
        // link actions
        a = new KAction(KIcon("tab-new"), i18n("Open in New &Tab"), this);
        a->setData(result.linkUrl());
        connect(a, SIGNAL(triggered(bool)), this, SLOT(openLinkInNewTab()));
        menu.addAction(a);

        a = new KAction(KIcon("window-new"), i18n("Open in New &Window"), this);
        a->setData(result.linkUrl());
        connect(a, SIGNAL(triggered(bool)), this, SLOT(openLinkInNewWindow()));
        menu.addAction(a);

        a = pageAction(QWebPage::DownloadLinkToDisk);
        a->setIcon(KIcon("document-save"));
        menu.addAction(a);

        a = pageAction(QWebPage::CopyLinkToClipboard);
        a->setIcon(KIcon("edit-copy"));
        menu.addAction(a);
        
        menu.addSeparator();
    }

    // is content editable && selected? Add CUT
    if (result.isContentEditable() && result.isContentSelected())
    {
        // actions for text selected in field
        a = pageAction(QWebPage::Cut);
        a->setIcon(KIcon("edit-cut"));
        a->setShortcut(KStandardShortcut::cut().primary());
        menu.addAction(a);
    }

    // is content selected) Add COPY
    if(result.isContentSelected())
    {
        a = pageAction(QWebPage::Copy);
        a->setIcon(KIcon("edit-copy"));
        a->setShortcut(KStandardShortcut::copy().primary());
        if(!result.isContentEditable()) // "Cut" "Copy Text" "Paste" is ugly. Don't add "text" with cut/paste
            a->setText(i18n("Copy Text"));
        else
            a->setText(i18n("Copy"));
        menu.addAction(a);
    }

    // is content editable? Add PASTE
    if(result.isContentEditable())
    {
        a = pageAction(QWebPage::Paste);
        a->setIcon(KIcon("edit-paste"));
        a->setShortcut(KStandardShortcut::paste().primary());
        menu.addAction(a);
    }

    // is content selected? Add SEARCH actions
    if(result.isContentSelected())
    {
        KActionMenu *searchMenu = new KActionMenu(KIcon("edit-find"), i18n("Search with"), this);

        KConfig config("kuriikwsfilterrc"); //Share with konqueror
        KConfigGroup cg = config.group("General");
        QStringList favoriteEngines;
        favoriteEngines << "wikipedia" << "google"; //defaults
        favoriteEngines = cg.readEntry("FavoriteSearchEngines", favoriteEngines);
        QString keywordDelimiter = cg.readEntry("KeywordDelimiter", ":");
        KService::Ptr service;
        KUriFilterData data;

        Q_FOREACH(const QString &engine, favoriteEngines)
        {
            if(!engine.isEmpty())
            {
                service = KService::serviceByDesktopPath(QString("searchproviders/%1.desktop").arg(engine));
                if(service)
                {
                    const QString searchProviderPrefix = *(service->property("Keys").toStringList().begin()) + keywordDelimiter;
                    data.setData(searchProviderPrefix + "some keyword");
                    a = new KAction(service->name(), this);
                    a->setIcon(Application::icon(KUrl(data.uri())));
                    a->setData(searchProviderPrefix);
                    connect(a, SIGNAL(triggered(bool)), this, SLOT(slotSearch()));
                    searchMenu->addAction(a);
                }
            }
        }

        if (!searchMenu->menu()->isEmpty())
        {
            menu.addAction(searchMenu);
        }

        menu.addSeparator();
        // TODO Add translate, show translation   
    }

    // is an image?
    if (!result.pixmap().isNull())
    {
        menu.addSeparator();

        // TODO remove copy_this_image action      
        a = new KAction(KIcon("view-media-visualization"), i18n("&View Image"), this);
        a->setData(result.imageUrl());
        connect(a, SIGNAL(triggered(Qt::MouseButtons, Qt::KeyboardModifiers)), this, SLOT(viewImage(Qt::MouseButtons, Qt::KeyboardModifiers)));
        menu.addAction(a);
        
        a = pageAction(QWebPage::DownloadImageToDisk);
        a->setIcon(KIcon("document-save"));
        menu.addAction(a);

        a = pageAction(QWebPage::CopyImageToClipboard);
        a->setIcon(KIcon("edit-copy"));
        menu.addAction(a);

        menu.addSeparator();
    }

    // last (but not less) actions..
    if(result.linkUrl().isEmpty())
    {
        // page action
        QString text = selectedText(); 
        if (text.startsWith( QLatin1String("http://") ) || text.startsWith( QLatin1String("https://") ) || text.startsWith( QLatin1String("www.") ) )
        {
            QString truncatedURL = text;
            if (text.length() > 18)
            {
                truncatedURL.truncate(15);
                truncatedURL += "...";
            }

            //open selected text url in a new tab
            a = new KAction(KIcon("tab-new"), i18n("Open '%1' in New Tab", truncatedURL), this);
            a->setData(QUrl(text));
            connect(a, SIGNAL(triggered(bool)), this, SLOT(openLinkInNewTab()));
            menu.addAction(a);

            //open selected text url in a new window
            a = new KAction(KIcon("window-new"), i18n("Open '%1' in New Window", truncatedURL), this);
            a->setData(QUrl(text));
            connect(a, SIGNAL(triggered(bool)), this, SLOT(openLinkInNewWindow()));
            menu.addAction(a);
        }
        else
        {
            menu.addAction(mainwindow->actionByName("new_tab"));    
            menu.addAction(mainwindow->actionByName("new_window"));
        }
        menu.addSeparator();
    }

    QWebHistory *history = page()->history();
    if(history->canGoBack())
    {
        a = pageAction(QWebPage::Back);
        a->setIcon(KIcon("go-previous"));
        menu.addAction(a);
    }

    if(history->canGoForward())
    {
        a = pageAction(QWebPage::Forward);
        a->setIcon(KIcon("go-next"));
        menu.addAction(a);
    }

    menu.addAction(mainwindow->actionByName("view_redisplay"));

    KActionMenu *frameMenu = new KActionMenu(i18n("Current Frame"), this);

    a = pageAction(QWebPage::OpenFrameInNewWindow);
    a->setText(i18n("Open in New Tab"));
    a->setIcon(KIcon("view-right-new"));
    frameMenu->addAction(a);

    a = new KAction( KIcon("document-print-frame"), i18n("Print Frame"), this);
    connect(a, SIGNAL(triggered()), this, SLOT(printFrame()));
    frameMenu->addAction(a);
    menu.addAction(frameMenu);

    // empty space actions
    if(result.linkUrl().isEmpty())
    {
        menu.addSeparator();

        menu.addAction(mainwindow->actionByName(KStandardAction::name(KStandardAction::SaveAs)));

        menu.addAction(mainwindow->actionByName("page_source"));
        menu.addAction(mainwindow->actionByName("add_to_favorites"));
        QAction *addBookmarkAction = Application::bookmarkProvider()->actionByName("rekonq_add_bookmark");
        menu.addAction(addBookmarkAction);

        if (page()->settings()->testAttribute(QWebSettings::DeveloperExtrasEnabled))
        {
            // Developer Extras actions
            a = pageAction(QWebPage::InspectElement);
            a->setIcon(KIcon("view-process-all"));
            menu.addAction(a);
        }
    }

    if(mainwindow->isFullScreen())
    {
        menu.addSeparator();
        menu.addAction(mainwindow->actionByName("fullscreen"));
    }

    menu.exec(mapToGlobal(event->pos()));
}


void WebView::stopScrollAnimation()
{
    m_scrollTimer->stop();
    m_scrollSpeedVertical = 0;
    m_scrollSpeedHorizontal = 0;
    m_scrollDirection = WebView::NoScroll;
}


void WebView::startScrollAnimation(ScrollDirection direction)
{
    // if no scrollspeed, set the requested direction, otherwise it's just a slowdown or speedup
    if (m_scrollSpeedVertical == 0 && (direction == WebView::Up || direction == WebView::Down))
        m_scrollDirection |= direction;
    if (m_scrollSpeedHorizontal == 0 && (direction == WebView::Left || direction == WebView::Right))
        m_scrollDirection |= direction;

    // update scrollspeed
    switch (direction)
    {
        case WebView::Up:
            --m_scrollSpeedVertical;
            break;
        case WebView::Down:
            ++m_scrollSpeedVertical;
            break;
        case WebView::Left:
            --m_scrollSpeedHorizontal;
            break;
        case WebView::Right:
            ++m_scrollSpeedHorizontal;
            break;
        default:
            break;
    }

    if (!m_scrollTimer->isActive())
        m_scrollTimer->start();

    return;
}


void WebView::scrollFrameChanged()
{
    // clear finished scrolling
    if (m_scrollSpeedVertical == 0)
        m_scrollDirection &= ~WebView::Up | ~WebView::Down;
    if (m_scrollSpeedHorizontal == 0)
        m_scrollDirection &= ~WebView::Left | ~WebView::Right;

    // all scrolling finished
    if (m_scrollDirection == WebView::NoScroll)
    {
        m_scrollTimer->stop();
        return;
    }

    // do the scrolling
    page()->currentFrame()->scroll(m_scrollSpeedHorizontal, m_scrollSpeedVertical);

    // check if we reached the end
    int y = page()->currentFrame()->scrollPosition().y();
    int x = page()->currentFrame()->scrollPosition().x();

    if (y == 0 || y == page()->currentFrame()->scrollBarMaximum(Qt::Vertical))
        m_scrollSpeedVertical = 0;
    if (x == 0 || x == page()->currentFrame()->scrollBarMaximum(Qt::Horizontal))
        m_scrollSpeedHorizontal = 0;
}


void WebView::mousePressEvent(QMouseEvent *event)
{
    stopScrollAnimation();

    m_page->m_pressedButtons = event->buttons();
    m_page->m_keyboardModifiers = event->modifiers();

    switch(event->button()) 
    {
      case Qt::XButton1:
        triggerPageAction(QWebPage::Back);
        break;
      case Qt::XButton2:
        triggerPageAction(QWebPage::Forward);
        break;
      default:
        QWebView::mousePressEvent(event);
    };
}


void WebView::mouseMoveEvent(QMouseEvent *event)
{
    if( url().protocol() != "rekonq" )
    {
        QWebView::mouseMoveEvent(event);
    }
}


void WebView::wheelEvent(QWheelEvent *event)
{
    stopScrollAnimation();

    if (QApplication::keyboardModifiers() & Qt::ControlModifier)
    {
        int numDegrees = event->delta() / 8;
        int numSteps = numDegrees / 15;
        setTextSizeMultiplier(textSizeMultiplier() + numSteps * 0.1);
        event->accept();
        return;
    }
    QWebView::wheelEvent(event);
}


void WebView::slotSearch()
{
    KAction *a = qobject_cast<KAction*>(sender());
    QString search = a->data().toString() + selectedText();
    KUrl urlSearch = KUrl::fromEncoded(search.toUtf8());
    Application::instance()->loadUrl(urlSearch, Rekonq::NewCurrentTab);
}


void WebView::slotUpdateProgress(int p)
{
    m_progress=p;
}


void WebView::slotLoadFinished(bool)
{
    m_progress=0;
}


void WebView::printFrame()
{
    Application::instance()->mainWindow()->printRequested(page()->currentFrame());
}


void WebView::viewImage(Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers)
{
    KAction *a = qobject_cast<KAction*>(sender());
    KUrl url(a->data().toUrl());

    if (modifiers & Qt::ControlModifier || buttons == Qt::MidButton)
    {
        Application::instance()->loadUrl(url, Rekonq::SettingOpenTab);
    }
    else
    {
        Application::instance()->loadUrl(url, Rekonq::CurrentTab);
    }
}


void WebView::openLinkInNewWindow()
{
    KAction *a = qobject_cast<KAction*>(sender());
    KUrl url(a->data().toUrl());
    Application::instance()->loadUrl(url, Rekonq::NewWindow);
}


void WebView::openLinkInNewTab()
{
    KAction *a = qobject_cast<KAction*>(sender());
    KUrl url(a->data().toUrl());
    Application::instance()->loadUrl(url, Rekonq::SettingOpenTab);
}


void WebView::keyPressEvent(QKeyEvent *event)
{
    if ((event->modifiers() == Qt::ControlModifier) && (event->key() == Qt::Key_C))
    {
        triggerPageAction(QWebPage::Copy);
        return;
    }

    if (event->modifiers() == Qt::ShiftModifier)
    {
        switch (event->key())
        {
        case Qt::Key_Down:
            startScrollAnimation(WebView::Down);
            return;
        case Qt::Key_Up:
            startScrollAnimation(WebView::Up);
            return;
        case Qt::Key_Left:
            startScrollAnimation(WebView::Left);
            return;
        case Qt::Key_Right:
            startScrollAnimation(WebView::Right);
            return;
        default:
            break;
        }
    }

    QWebView::keyPressEvent(event);
}
