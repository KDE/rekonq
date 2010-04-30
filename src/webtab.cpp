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
#include "webtab.h"
#include "webtab.moc"

// Auto Includes
#include "rekonq.h"

// Local Includes
#include "application.h"
#include "mainwindow.h"
#include "mainview.h"
#include "webpage.h"
#include "bookmarksmanager.h"
#include "walletbar.h"
#include "previewselectorbar.h"
#include "rsswidget.h"

// KDE Includes
#include <KService>
#include <KUriFilterData>
#include <KStandardShortcut>
#include <KMenu>
#include <KActionMenu>
#include <KWebView>
#include <kwebwallet.h>
#include <KDE/KMessageBox>

// Qt Includes
#include <QContextMenuEvent>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QClipboard>
#include <QKeyEvent>
#include <QAction>
#include <QVBoxLayout>


WebTab::WebTab(QWidget *parent)
        : QWidget(parent)
        , m_progress(0)
{
    QVBoxLayout *l = new QVBoxLayout(this);
    l->setMargin(0);
    l->setSpacing(0);

    QWidget *messageBar = new QWidget(this);
    l->addWidget(messageBar);
    messageBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    QVBoxLayout *l2 = new QVBoxLayout(messageBar);
    l2->setMargin(0);
    l2->setSpacing(0);

    WebView *view = new WebView(this);
    l->addWidget(view);
    view->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // fix focus handling
    setFocusProxy(view);

    KWebWallet *wallet = view->page()->wallet();

    if (wallet)
    {
        connect(wallet, SIGNAL(saveFormDataRequested(const QString &, const QUrl &)),
                this, SLOT(createWalletBar(const QString &, const QUrl &)));
    }

    connect(view, SIGNAL(loadProgress(int)), this, SLOT(updateProgress(int)));
    connect(view, SIGNAL(loadFinished(bool)), this, SLOT(loadFinished(bool)));
}


WebTab::~WebTab()
{
}


WebView *WebTab::view()
{
    WebView *view = qobject_cast<WebView *>(layout()->itemAt(1)->widget());
    return view;
}


WebPage *WebTab::page()
{
    return view()->page();
}


// TODO:
// Import the "about" check and the one in protocolhandler
// in some (static?) methods in NewTabPage
KUrl WebTab::url()
{
    KUrl u = KUrl(view()->url());
    if (u.scheme() == QL1S("about"))
    {
        QWebElement rootElement = page()->mainFrame()->documentElement();
        if (rootElement.document().findAll("#rekonq-newtabpage").count() == 0)
            return u;
        if (rootElement.findAll(".favorites").count() > 0)
            return KUrl("about:favorites");
        if (rootElement.findAll(".closedTabs").count() > 0)
            return KUrl("about:closedTabs");
        if (rootElement.findAll(".history").count() > 0)
            return KUrl("about:history");
        if (rootElement.findAll(".bookmarks").count() > 0)
            return KUrl("about:bookmarks");
        if (rootElement.findAll(".downloads").count() > 0)
            return KUrl("about:downloads");
    }
    return u;
}


int WebTab::progress()
{
    return m_progress;
}


void WebTab::updateProgress(int p)
{
    m_progress = p;
}


void WebTab::loadFinished(bool)
{
    m_progress = 0;
}


void WebTab::createWalletBar(const QString &key, const QUrl &url)
{
    // check if the url is in the wallet blacklist
    QString urlString = url.toString();
    QStringList blackList = ReKonfig::walletBlackList();
    if (blackList.contains(urlString))
        return;

    KWebWallet *wallet = page()->wallet();
    QWidget *messageBar = layout()->itemAt(0)->widget();

    WalletBar *walletBar = new WalletBar(messageBar);
    walletBar->onSaveFormData(key, url);
    messageBar->layout()->addWidget(walletBar);

    connect(walletBar, SIGNAL(saveFormDataAccepted(const QString &)),
            wallet, SLOT(acceptSaveFormDataRequest(const QString &)));
    connect(walletBar, SIGNAL(saveFormDataRejected(const QString &)),
            wallet, SLOT(rejectSaveFormDataRequest(const QString &)));
}


void WebTab::createPreviewSelectorBar(int index)
{
    QWidget *messageBar = layout()->itemAt(0)->widget();
    PreviewSelectorBar *bar = new PreviewSelectorBar(index, messageBar);
    messageBar->layout()->addWidget(bar);

    connect(page(), SIGNAL(loadStarted()), bar, SLOT(loadProgress()));
    connect(page(), SIGNAL(loadProgress(int)), bar, SLOT(loadProgress()));
    connect(page(), SIGNAL(loadFinished(bool)), bar, SLOT(loadFinished()));
    connect(page()->mainFrame(), SIGNAL(urlChanged(QUrl)), bar, SLOT(verifyUrl()));
}


bool WebTab::hasRSSInfo()
{
    QWebElementCollection col = page()->mainFrame()->findAllElements("link[type=\"application/rss+xml\"]");
    col.append(page()->mainFrame()->findAllElements("link[type=\"application/atom+xml\"]"));
    if (col.count() != 0)
        return true;

    return false;
}


void WebTab::showRSSInfo(QPoint pos)
{
    QWebElementCollection col = page()->mainFrame()->findAllElements("link[type=\"application/rss+xml\"]");
    col.append(page()->mainFrame()->findAllElements("link[type=\"application/atom+xml\"]"));

    QMap<KUrl, QString> map;
    int i = 0;
    foreach(QWebElement el, col)
    {
        QString urlString;
        if (el.attribute("href").startsWith(QL1S("http")))
            urlString = el.attribute("href");
        else
        {
            KUrl u = url();
            // NOTE
            // cd() is probably better than setPath() here,
            // for all those url sites just having a path
            if (u.cd(el.attribute("href")))
                urlString = u.toMimeDataString();
        }

        QString title = el.attribute("title");
        if (title.isEmpty())
            title = el.attribute("href");

        map.insert(KUrl(urlString), title);

        i++;
    }

    RSSWidget *widget = new RSSWidget(map, window());
    widget->showAt(pos);
}
