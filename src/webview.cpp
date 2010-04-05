/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2008-2010 by Andrea Diamantini <adjam7 at gmail dot com>
* Copyright (C) 2009-2010 by Lionel Chauvin <megabigbug@yahoo.fr>
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
#include "mainwindow.h"
#include "mainview.h"
#include "webpage.h"
#include "bookmarksmanager.h"

// KDE Includes
#include <KService>
#include <KUriFilterData>
#include <KStandardShortcut>
#include <KMenu>
#include <KActionMenu>
#include <ktoolinvocation.h>

// Qt Includes
#include <QContextMenuEvent>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QClipboard>
#include <QKeyEvent>
#include <QAction>
#include <QLayout>
#include <QtDBus/QDBusConnectionInterface>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusReply>
#include <QDir>


WebView::WebView(QWidget* parent)
    : KWebView(parent, false)
    , _mousePos( QPoint(0,0) )
    , _scrollTimer( new QTimer(this) )
    , _VScrollSpeed(0)
    , _HScrollSpeed(0)
    , _disableAutoScroll(false)
{
    WebPage *page = new WebPage(this);
    setPage(page);

    // download system
    connect(this, SIGNAL(linkShiftClicked(const KUrl &)), 
            page, SLOT(downloadUrl(const KUrl &)));
    connect(page, SIGNAL(downloadRequested(const QNetworkRequest &)), 
            page, SLOT(downloadRequest(const QNetworkRequest &)));
            
    // middle click || ctrl + click signal
    connect(this, SIGNAL(linkMiddleOrCtrlClicked(const KUrl &)), 
            this, SLOT(loadUrlInNewTab(const KUrl &)) );

    // loadUrl signal
    connect(this, SIGNAL(loadUrl(const KUrl &, const Rekonq::OpenType &)), 
            Application::instance(), SLOT(loadUrl(const KUrl &, const Rekonq::OpenType &)));

    // scrolling timer
    connect(_scrollTimer, SIGNAL(timeout()), this, SLOT(scrollFrameChanged()));
    _scrollTimer->setInterval(100);
}


WebView::~WebView()
{
     disconnect();
}


WebPage *WebView::page()
{
    WebPage *page = qobject_cast<WebPage *>( KWebView::page() );
    return page;
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
                    a->setIcon( Application::icon( data.uri() ) );
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

        if( result.pixmap().isNull() )
        {
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

            if (ReKonfig::kgetList())
            {
                a = new KAction(KIcon("kget"), i18n("List All Links"), this);
                connect(a, SIGNAL(triggered(bool)), page(), SLOT(downloadAllContentsWithKGet()));
                menu.addAction(a);
            }

            menu.addAction(mainwindow->actionByName("page_source"));
            
            a = new KAction( KIcon("layer-visible-on"), i18n("Inspect Element"), this);
            connect(a, SIGNAL(triggered(bool)), this, SLOT(inspect()));
            menu.addAction(a);
            
            a = Application::bookmarkProvider()->actionByName("rekonq_add_bookmark");
            menu.addAction(a);
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
    QWebHitTestResult result = page()->mainFrame()->hitTestContent( event->pos() );
    _disableAutoScroll = result.isContentEditable();
    
    switch(event->button())
    {
      case Qt::XButton1:
        triggerPageAction(KWebPage::Back);
        break;
      case Qt::XButton2:
        triggerPageAction(KWebPage::Forward);
        break;
      default:
        KWebView::mousePressEvent(event);
        break;
    };
}


void WebView::mouseMoveEvent(QMouseEvent *event)
{
    _mousePos = event->pos();
    if (Application::instance()->mainWindow()->isFullScreen())
    {        
        if (event->pos().y()>=0 && event->pos().y()<=4)
        {
            Application::instance()->mainWindow()->setWidgetsVisible(true); 
        }
        else
        {
            Application::instance()->mainWindow()->setWidgetsVisible(false); 
        }
    }
    KWebView::mouseMoveEvent(event);
}


QPoint WebView::mousePos()
{
    return _mousePos;
}


void WebView::search()
{
    KAction *a = qobject_cast<KAction*>(sender());
    QString search = a->data().toString() + selectedText();
    KUrl urlSearch = KUrl::fromEncoded(search.toUtf8());
    
    emit loadUrl(urlSearch, Rekonq::NewCurrentTab);
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
        emit loadUrl(url, Rekonq::SettingOpenTab);
    }
    else
    {
        emit loadUrl(url, Rekonq::CurrentTab);
    }
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
    
    emit loadUrl(url, Rekonq::SettingOpenTab);
}


void WebView::keyPressEvent(QKeyEvent *event)
{
    if ( event->modifiers() == Qt::ControlModifier )
    {
        if ( event->key() == Qt::Key_C )
        {
            triggerPageAction(KWebPage::Copy);
            return;
        }

        if ( event->key() == Qt::Key_A )
        {
            triggerPageAction(KWebPage::SelectAll);
            return;
        }
    }
 
    if(_disableAutoScroll)
    {
        KWebView::keyPressEvent(event);
        return;
    }
    
    // Auto Scrolling
    if ( event->modifiers() == Qt::ShiftModifier )
    {
        if( event->key() == Qt::Key_Up )
        {
            _VScrollSpeed--;
            if( !_scrollTimer->isActive() )
                _scrollTimer->start();
            return;
        }
        
        if( event->key() == Qt::Key_Down )
        {
            _VScrollSpeed++;
            if( !_scrollTimer->isActive() )
                _scrollTimer->start();
            return;
        }
        
        if( event->key() == Qt::Key_Right )
        {
            _HScrollSpeed++;
            if( !_scrollTimer->isActive() )
                _scrollTimer->start();
            return;
        }
        
        if( event->key() == Qt::Key_Left )
        {
            _HScrollSpeed--;
            if( !_scrollTimer->isActive() )
                _scrollTimer->start();
            return;
        }
        
        if(_scrollTimer->isActive())
        {
            _scrollTimer->stop();
        }
        else
        {
            if(_VScrollSpeed || _HScrollSpeed)
                _scrollTimer->start();
        }
    }
    
    KWebView::keyPressEvent(event);
}


void WebView::inspect()
{
    QAction *a = Application::instance()->mainWindow()->actionByName("web_inspector");
    if(a && !a->isChecked())
        a->trigger();
    pageAction(QWebPage::InspectElement)->trigger();
}


void WebView::loadUrlInNewTab(const KUrl &url)
{
    emit loadUrl(url, Rekonq::SettingOpenTab);
}


void WebView::scrollFrameChanged()
{
    // do the scrolling
    page()->currentFrame()->scroll( _HScrollSpeed, _VScrollSpeed );
    
    // check if we reached the end
    int y = page()->currentFrame()->scrollPosition().y();
    if (y == 0 || y == page()->currentFrame()->scrollBarMaximum(Qt::Vertical))
        _VScrollSpeed = 0;

    int x = page()->currentFrame()->scrollPosition().x();
    if (x == 0 || x == page()->currentFrame()->scrollBarMaximum(Qt::Horizontal))
        _HScrollSpeed = 0;
}
