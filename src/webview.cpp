/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2007-2008 Trolltech ASA. All rights reserved
* Copyright (C) 2008-2009 by Andrea Diamantini <adjam7 at gmail dot com>
*
*
* This program is free software; you can redistribute it
* and/or modify it under the terms of the GNU General
* Public License as published by the Free Software Foundation;
* either version 3, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
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
#include "cookiejar.h"
#include "networkaccessmanager.h"
#include "history.h"
#include "webpage.h"

// KDE Includes
#include <KStandardDirs>
#include <KUrl>
#include <KActionCollection>
#include <KDebug>
#include <KToolInvocation>

// Qt Includes
#include <QtGui/QContextMenuEvent>
#include <QtGui/QWheelEvent>
#include <QtGui/QMouseEvent>
#include <QtGui/QClipboard>
#include <QtGui/QKeyEvent>
#include <QtGui/QAction>

#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>

#include <QtWebKit/QWebFrame>
#include <QtWebKit/QWebHitTestResult>
#include <QtWebKit/QWebPage>
#include <QtWebKit/QWebSettings>
#include <QtWebKit/QWebView>


WebView::WebView(QWidget* parent)
        : QWebView(parent)
        , m_page(new WebPage(this))
        , m_progress(0)
{
    setPage(m_page);
    
    connect(page(), SIGNAL(statusBarMessage(const QString&)), this, SLOT(setStatusBarText(const QString&)));
    connect(this, SIGNAL(loadProgress(int)), this, SLOT(setProgress(int)));
    connect(this, SIGNAL(loadFinished(bool)), this, SLOT(loadFinished()));
}


WebPage *WebView::page() const
{
    return qobject_cast<WebPage *>(QWebView::page());
}


KUrl WebView::url() const 
{ 
    return KUrl(QWebView::url()); 
}


QString WebView::lastStatusBarText() const
{ 
    return m_statusBarText; 
}


int WebView::progress() const
{ 
    return m_progress; 
}


void WebView::setProgress(int progress) 
{ 
    m_progress = progress; 
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
    
    // link actions
    bool linkIsEmpty = result.linkUrl().isEmpty();
    if (!linkIsEmpty)
    {
        a = pageAction(QWebPage::OpenLinkInNewWindow);
        a->setText(i18n("Open Link in New &Tab"));
        a->setIcon(KIcon("window-new"));
        menu.addAction(a);
    }
    else
    {
        menu.addAction(mainwindow->actionByName("new_tab"));
    }
    menu.addAction(mainwindow->actionByName("view_redisplay"));
    menu.addSeparator();

    // Developer Extras actions
    if (page()->settings()->testAttribute(QWebSettings::DeveloperExtrasEnabled))
    {
        a = pageAction(QWebPage::InspectElement);  
        a->setIcon(KIcon("view-process-all"));
        menu.addAction(a);
        menu.addSeparator();
    }

    // cut - copy - paste Actions. 
    bool b = false;

    if (result.isContentSelected() && result.isContentEditable())
    {
        a = pageAction(QWebPage::Cut);
        a->setIcon(KIcon("edit-cut"));
        a->setShortcut(KStandardShortcut::cut().primary());
        menu.addAction(a);
        b = true;
    }

    if (result.isContentSelected())
    {
        a = pageAction(QWebPage::Copy);
        a->setIcon(KIcon("edit-copy"));
        a->setShortcut(KStandardShortcut::copy().primary());
        menu.addAction(a);
        b = true;
    }

    if (result.isContentEditable())
    {
        a = pageAction(QWebPage::Paste);
        a->setIcon(KIcon("edit-paste"));
        a->setShortcut(KStandardShortcut::paste().primary());
        menu.addAction(a);
        b = true;
    }

    if(b)
    {
        menu.addSeparator();
    }

    // save/copy link actions
    if (!linkIsEmpty)
    {
        a = pageAction(QWebPage::DownloadLinkToDisk);
        a->setIcon(KIcon("document-save"));
        menu.addAction(a);
        
        a = pageAction(QWebPage::CopyLinkToClipboard);
        a->setIcon(KIcon("edit-copy"));
        menu.addAction(a);

        menu.addSeparator();

        if (!result.pixmap().isNull())
        {
            // TODO Add "View Image" && remove copy_this_image action
            a = pageAction(QWebPage::DownloadImageToDisk); 
            a->setIcon(KIcon("document-save"));
            menu.addAction(a);

            a = pageAction(QWebPage::CopyImageToClipboard);
            a->setIcon(KIcon("edit-copy"));
            menu.addAction(a);
            
            menu.addSeparator();
        }
    }

    // history actions
    menu.addAction(mainwindow->actionByName("history_back"));
    menu.addAction(mainwindow->actionByName("history_forward"));

    // bookmark link action
    if (!linkIsEmpty)
    {
        menu.addSeparator();
        QAction *addBookmarkAction = Application::bookmarkProvider()->actionByName("rekonq_add_bookmark");
        menu.addAction(addBookmarkAction);
    }

    if(mainwindow->isFullScreen())
    {
        menu.addAction(mainwindow->actionByName("fullscreen"));
    }

    menu.exec(mapToGlobal(event->pos()));
}


void WebView::loadFinished()
{
    if (m_progress != 100)
    {
        kWarning() << "Received finished signal while progress is still:" << progress()
        << "Url:" << url();
    }
    m_progress = 0;
}


void WebView::keyPressEvent(QKeyEvent *event)
{
    if ((event->modifiers() == Qt::ControlModifier) && (event->key() == Qt::Key_Tab))
    {
        emit ctrlTabPressed();
        return;
    }

    if ((event->modifiers() == Qt::ControlModifier + Qt::ShiftModifier) && (event->key() == Qt::Key_Backtab))
    {
        emit shiftCtrlTabPressed();
        return;
    }

    QWebView::keyPressEvent(event);
}


void WebView::mousePressEvent(QMouseEvent *event)
{
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
