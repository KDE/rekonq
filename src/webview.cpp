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


WebView::WebView(QWidget* parent)
        : QWebView(parent)
        , m_page(new WebPage(this))
        , m_progress(0)
{
    setPage(m_page);
    
    connect(page(), SIGNAL(statusBarMessage(const QString&)), this, SLOT(setStatusBarText(const QString&)));
    connect(this, SIGNAL(loadProgress(int)), this, SLOT(slotUpdateProgress(int)));
    connect(this, SIGNAL(loadFinished(bool)), this, SLOT(slotLoadFinished(bool)));
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

    if (!result.linkUrl().isEmpty())
    {
        // link actions
        a = pageAction(QWebPage::OpenLinkInNewWindow);
        a->setText(i18n("Open Link in New &Tab"));
        a->setIcon(KIcon("window-new"));
        menu.addAction(a);

        a = pageAction(QWebPage::DownloadLinkToDisk);
        a->setIcon(KIcon("document-save"));
        menu.addAction(a);

        a = pageAction(QWebPage::CopyLinkToClipboard);
        a->setIcon(KIcon("edit-copy"));
        menu.addAction(a);

        if (!result.pixmap().isNull())
        {
            menu.addSeparator();

            // TODO Add "View Image" && remove copy_this_image action
            a = pageAction(QWebPage::DownloadImageToDisk);
            a->setIcon(KIcon("document-save"));
            menu.addAction(a);

            a = pageAction(QWebPage::CopyImageToClipboard);
            a->setIcon(KIcon("edit-copy"));
            menu.addAction(a);
        }
    }
    else if (result.isContentEditable() && result.isContentSelected())
    {
        // actions for text selected in field
        a = pageAction(QWebPage::Cut);
        a->setIcon(KIcon("edit-cut"));
        a->setShortcut(KStandardShortcut::cut().primary());
        menu.addAction(a);

        a = pageAction(QWebPage::Copy);
        a->setIcon(KIcon("edit-copy"));
        a->setShortcut(KStandardShortcut::copy().primary());
        menu.addAction(a);

        a = pageAction(QWebPage::Paste);
        a->setIcon(KIcon("edit-paste"));
        a->setShortcut(KStandardShortcut::paste().primary());
        menu.addAction(a);

        menu.addSeparator();

        KConfig config("kuriikwsfilterrc"); //Share with konqueror
        KConfigGroup cg = config.group("General");
        QStringList favoriteEngines;
        favoriteEngines << "wikipedia" << "google"; //defaults
        favoriteEngines = cg.readEntry("FavoriteSearchEngines", favoriteEngines);
        QString keywordDelimiter = cg.readEntry("KeywordDelimiter", ":");
        KService::Ptr service;
        KUriFilterData data;
        foreach (const QString &engine, favoriteEngines)
        {
            service = KService::serviceByDesktopPath(QString("searchproviders/%1.desktop").arg(engine));
            const QString searchProviderPrefix = *(service->property("Keys").toStringList().begin()) + keywordDelimiter;
            data.setData(searchProviderPrefix + "some keyword");
            a = new KAction(i18n("Search with ")+service->name(), this);
            a->setIcon(Application::icon(KUrl(data.uri())));
            a->setData(searchProviderPrefix);
            connect(a, SIGNAL(triggered(bool)), this, SLOT(slotSearch()));
            menu.addAction(a);
        }


        // TODO Add translate, show translation
    }
    else if (result.isContentEditable())
    {
        // actions for a not selected field or a void field
        // WARNING: why it doesn't automatically select a field?
        // Why the paste action is disabled?

        a = pageAction(QWebPage::Paste);
        a->setIcon(KIcon("edit-paste"));
        a->setShortcut(KStandardShortcut::paste().primary());
        menu.addAction(a);
    }
    else if (result.isContentSelected())
    {
        // actions for text selected in page
        a = pageAction(QWebPage::Copy);
        a->setIcon(KIcon("edit-copy"));
        a->setShortcut(KStandardShortcut::copy().primary());
        menu.addAction(a);

        menu.addSeparator();

        KConfig config("kuriikwsfilterrc"); //Share with konqueror
        KConfigGroup cg = config.group("General");
        QStringList favoriteEngines;
        favoriteEngines << "wikipedia" << "google"; //defaults
        favoriteEngines = cg.readEntry("FavoriteSearchEngines", favoriteEngines);
        QString keywordDelimiter = cg.readEntry("KeywordDelimiter", ":");
        KService::Ptr service;
        KUriFilterData data;
        foreach (const QString &engine, favoriteEngines)
        {
            service = KService::serviceByDesktopPath(QString("searchproviders/%1.desktop").arg(engine));
            const QString searchProviderPrefix = *(service->property("Keys").toStringList().begin()) + keywordDelimiter;
            data.setData(searchProviderPrefix + "some keyword");
            a = new KAction(i18n("Search with %1", service->name()), this);
            a->setIcon(Application::icon(KUrl(data.uri())));
            a->setData(searchProviderPrefix);
            connect(a, SIGNAL(triggered(bool)), this, SLOT(slotSearch()));
            menu.addAction(a);
        }

        // TODO Add translate, show translation
    }
    else if (!result.pixmap().isNull())
    {
        menu.addSeparator();

        // TODO Add "View Image" && remove copy_this_image action
        a = pageAction(QWebPage::DownloadImageToDisk);
        a->setIcon(KIcon("document-save"));
        menu.addAction(a);

        a = pageAction(QWebPage::CopyImageToClipboard);
        a->setIcon(KIcon("edit-copy"));
        menu.addAction(a);
    }
    else
    {
        //page actions
        menu.addAction(mainwindow->actionByName("new_tab"));
        if(mainwindow->isFullScreen())
        {
            menu.addAction(mainwindow->actionByName("fullscreen"));
        }

        menu.addSeparator();

        menu.addAction(mainwindow->actionByName("history_back"));
        menu.addAction(mainwindow->actionByName("history_forward"));
        menu.addAction(mainwindow->actionByName("view_redisplay"));
        
        menu.addSeparator();

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

    menu.exec(mapToGlobal(event->pos()));
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


void WebView::wheelEvent(QWheelEvent *event)
{
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
