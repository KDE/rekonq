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
#include <KService>
#include <KUriFilterData>
#include <KStandardShortcut>

// Qt Includes
#include <QContextMenuEvent>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QClipboard>
#include <QKeyEvent>
#include <QAction>


WebView::WebView(QWidget* parent)
        : KWebView(parent, false)
        , m_page(new WebPage(this))
        , m_progress(0)
        , m_mousePos(QPoint(0,0))

{
    setPage(m_page);
    
    connect(page(), SIGNAL(statusBarMessage(const QString&)), this, SLOT(setStatusBarText(const QString&)));
    connect(this, SIGNAL(loadProgress(int)), this, SLOT(updateProgress(int)));
    connect(this, SIGNAL(loadFinished(bool)), this, SLOT(loadFinished(bool)));

    connect(this, SIGNAL(linkMiddleOrCtrlClicked(const KUrl &)), this, SLOT(loadInNewTab(const KUrl &)) );

    connect(this, SIGNAL(linkShiftClicked(const KUrl &)), this, SLOT(downloadRequest(const KUrl &)));
    connect(page(), SIGNAL(downloadRequested(const QNetworkRequest &)), this, SLOT(downloadRequest(const QNetworkRequest &r)));
}


WebView::~WebView()
{
}


WebPage *WebView::page()
{
    if(!m_page)
    {
        m_page = new WebPage(this);
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

        menu.addAction(pageAction(KWebPage::DownloadLinkToDisk));
        menu.addAction(pageAction(KWebPage::CopyLinkToClipboard));
        menu.addSeparator();
    }

    // is content editable && selected? Add CUT
    if (result.isContentEditable() && result.isContentSelected())
    {
        // actions for text selected in field
        menu.addAction(pageAction(KWebPage::Cut));
    }

    // is content selected) Add COPY
    if(result.isContentSelected())
    {
        a = pageAction(KWebPage::Copy);
        if(!result.linkUrl().isEmpty())
            a->setText(i18n("Copy Text")); //for link
        else
            a->setText(i18n("Copy"));
        menu.addAction(a);
    }

    // is content editable? Add PASTE
    if(result.isContentEditable())
    {
        menu.addAction(pageAction(KWebPage::Paste));
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
                    connect(a, SIGNAL(triggered(bool)), this, SLOT(search()));
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

        menu.addAction(pageAction(KWebPage::DownloadImageToDisk));
        menu.addAction(pageAction(KWebPage::CopyImageToClipboard));
        menu.addSeparator();
    }

    // Open url text in new tab/window
    if(result.linkUrl().isEmpty())
    {

        QString text = selectedText(); 
        if (text.startsWith( QLatin1String("http://") ) 
            || text.startsWith( QLatin1String("https://") ) 
            || text.startsWith( QLatin1String("www.") ) 
           )
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

            menu.addSeparator();
        }

    }

    // page actions
    if (!result.isContentSelected() && result.linkUrl().isEmpty())
    {

        // navigation
        QWebHistory *history = page()->history();
        if(history->canGoBack())
        {
            menu.addAction(pageAction(KWebPage::Back));
        }

        if(history->canGoForward())
        {
            menu.addAction(pageAction(KWebPage::Forward));
        }

        menu.addAction(mainwindow->actionByName("view_redisplay"));

        menu.addSeparator();

        menu.addAction(mainwindow->actionByName("new_tab"));    
        menu.addAction(mainwindow->actionByName("new_window"));

        menu.addSeparator();

        //Frame
        KActionMenu *frameMenu = new KActionMenu(i18n("Current Frame"), this);

        frameMenu->addAction(pageAction(KWebPage::OpenFrameInNewWindow));

        a = new KAction( KIcon("document-print-frame"), i18n("Print Frame"), this);
        connect(a, SIGNAL(triggered()), this, SLOT(printFrame()));
        frameMenu->addAction(a);

        menu.addAction(frameMenu);
        
        menu.addSeparator();

        // Page Actions
        menu.addAction(pageAction(KWebPage::SelectAll));

        menu.addAction(mainwindow->actionByName(KStandardAction::name(KStandardAction::SaveAs)));
        menu.addAction(mainwindow->actionByName("page_source"));

        QAction *addBookmarkAction = Application::bookmarkProvider()->actionByName("rekonq_add_bookmark");
        menu.addAction(addBookmarkAction);

        if (page()->settings()->testAttribute(QWebSettings::DeveloperExtrasEnabled))
        {
            // Developer Extras actions
            menu.addAction(pageAction(KWebPage::InspectElement));
        }

        if(mainwindow->isFullScreen())
        {
            menu.addSeparator();
            menu.addAction(mainwindow->actionByName("fullscreen"));
        }
    }

    menu.exec(mapToGlobal(event->pos()));
}

void WebView::mousePressEvent(QMouseEvent *event)
{
    m_page->m_pressedButtons = event->buttons();
    m_page->m_keyboardModifiers = event->modifiers();

    switch(event->button())
    {
      case Qt::XButton1:
        triggerPageAction(KWebPage::Back);
        break;
      case Qt::XButton2:
        triggerPageAction(KWebPage::Forward);
        break;
      default:
        QWebView::mousePressEvent(event);
    };
}


void WebView::mouseMoveEvent(QMouseEvent *event)
{
    m_mousePos = event->pos();
    QWebView::mouseMoveEvent(event);
}


QPoint WebView::mousePos()
{
    return m_mousePos;
}


void WebView::search()
{
    KAction *a = qobject_cast<KAction*>(sender());
    QString search = a->data().toString() + selectedText();
    KUrl urlSearch = KUrl::fromEncoded(search.toUtf8());
    Application::instance()->loadUrl(urlSearch, Rekonq::NewCurrentTab);
}


void WebView::updateProgress(int p)
{
    m_progress = p;
}


void WebView::loadFinished(bool)
{
    m_progress = 0;
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
        triggerPageAction(KWebPage::Copy);
        return;
    }

    if ((event->modifiers() == Qt::ControlModifier) && (event->key() == Qt::Key_A))
    {
        triggerPageAction(KWebPage::SelectAll);
        return;
    }

    QWebView::keyPressEvent(event);
}


void WebView::loadInNewTab(const KUrl &url)
{
    Application::instance()->loadUrl(url, Rekonq::NewCurrentTab);
}
    
    
void WebView::downloadRequest(const KUrl &url)
{
    m_page->downloadRequest(QNetworkRequest(url));
}


void WebView::downloadRequest(const QNetworkRequest &request)
{
    m_page->downloadRequest(request);
}
