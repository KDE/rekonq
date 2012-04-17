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
#include "webtab.h"
#include "webtab.moc"

// Auto Includes
#include "rekonq.h"

// Local Includes
#include "application.h"
#include "historymanager.h"
#include "messagebar.h"
#include "opensearchmanager.h"
#include "previewselectorbar.h"
#include "rsswidget.h"
#include "searchenginebar.h"
#include "sessionmanager.h"
#include "syncmanager.h"
#include "urlbar.h"
#include "walletbar.h"
#include "webpage.h"
#include "websnap.h"
#include "webshortcutwidget.h"

// KDE Includes
#include <KWebWallet>
#include <KStandardShortcut>
#include <KMenu>
#include <KActionMenu>
#include <KWebView>
#include <KDebug>
#include <KBuildSycocaProgressDialog>

// Qt Includes
#include <QVBoxLayout>


WebTab::WebTab(QWidget *parent)
    : QWidget(parent)
    , m_webView(0)
    , m_urlBar(new UrlBar(this))
    , m_progress(0)
    , m_part(0)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QVBoxLayout *l = new QVBoxLayout(this);
    l->setMargin(0);
    l->setSpacing(0);

    l->addWidget(view());
    view()->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // fix focus handling
    setFocusProxy(view());

    KWebWallet *wallet = page()->wallet();

    if (wallet)
    {
        connect(wallet, SIGNAL(saveFormDataRequested(QString,QUrl)),
                this, SLOT(createWalletBar(QString,QUrl)));
    }

    connect(view(), SIGNAL(loadProgress(int)), this, SLOT(updateProgress(int)));
    connect(view(), SIGNAL(loadStarted()), this, SLOT(resetProgress()));
    connect(view(), SIGNAL(titleChanged(QString)), this, SIGNAL(titleChanged(QString)));
    connect(view(), SIGNAL(loadFinished(bool)), this, SLOT(loadFinished()));

    // Session Manager
    connect(view(), SIGNAL(loadFinished(bool)), rApp->sessionManager(), SLOT(saveSession()));
}


WebTab::~WebTab()
{
    m_walletBar.clear();
    m_previewSelectorBar.clear();

    delete m_part;
    delete m_urlBar;
    delete m_webView;
}


WebView *WebTab::view()
{
    if (!m_webView)
    {
        m_webView = new WebView(this);
    }
    return m_webView;
}


WebPage *WebTab::page()
{
    return view()->page();
}


KUrl WebTab::url()
{
    if (page() && page()->isOnRekonqPage())
    {
        return page()->loadingUrl();
    }

    return view()->url();
}


void WebTab::updateProgress(int p)
{
    m_progress = p;
    emit loadProgressing();
}


void WebTab::resetProgress()
{
    m_progress = 1;
}


bool WebTab::isPageLoading()
{
    return m_progress != 0 && m_progress != 100;
}


void WebTab::createWalletBar(const QString &key, const QUrl &url)
{
    // check if the url is in the wallet blacklist
    QString urlString = url.toString();
    QStringList blackList = ReKonfig::walletBlackList();
    if (blackList.contains(urlString))
        return;

    KWebWallet *wallet = page()->wallet();

    if (!ReKonfig::passwordSavingEnabled())
    {
        wallet->rejectSaveFormDataRequest(key);
        return;
    }

    if (m_walletBar.isNull())
    {
        m_walletBar = new WalletBar(this);
        m_walletBar.data()->onSaveFormData(key, url);
        qobject_cast<QVBoxLayout *>(layout())->insertWidget(0, m_walletBar.data());
        m_walletBar.data()->animatedShow();
    }
    else
    {
        disconnect(wallet);
        qobject_cast<QVBoxLayout *>(layout())->insertWidget(0, m_walletBar.data());
        m_walletBar.data()->animatedShow();
    }

    connect(m_walletBar.data(), SIGNAL(saveFormDataAccepted(QString)),
            wallet, SLOT(acceptSaveFormDataRequest(QString)), Qt::UniqueConnection);
    connect(m_walletBar.data(), SIGNAL(saveFormDataRejected(QString)),
            wallet, SLOT(rejectSaveFormDataRequest(QString)), Qt::UniqueConnection);

    // sync passwords
    connect(m_walletBar.data(), SIGNAL(saveFormDataAccepted(QString)),
            rApp->syncManager(), SLOT(syncPasswords()), Qt::UniqueConnection);
}


void WebTab::createPreviewSelectorBar(int index)
{
    if (m_previewSelectorBar.isNull())
    {
        m_previewSelectorBar = new PreviewSelectorBar(index, this);
        qobject_cast<QVBoxLayout *>(layout())->insertWidget(0, m_previewSelectorBar.data());
        m_previewSelectorBar.data()->animatedShow();
    }
    else
    {
        disconnect(m_previewSelectorBar.data());
        m_previewSelectorBar.data()->setIndex(index);
        m_previewSelectorBar.data()->animatedHide();
    }

    connect(page(),             SIGNAL(loadStarted()),      m_previewSelectorBar.data(), SLOT(loadProgress()), Qt::UniqueConnection);
    connect(page(),             SIGNAL(loadProgress(int)),  m_previewSelectorBar.data(), SLOT(loadProgress()), Qt::UniqueConnection);
    connect(page(),             SIGNAL(loadFinished(bool)), m_previewSelectorBar.data(), SLOT(loadFinished()), Qt::UniqueConnection);
    connect(page()->mainFrame(), SIGNAL(urlChanged(QUrl)),  m_previewSelectorBar.data(), SLOT(verifyUrl()),    Qt::UniqueConnection);
}


bool WebTab::hasRSSInfo()
{
    QWebElementCollection col = page()->mainFrame()->findAllElements("link[type=\"application/rss+xml\"]");
    col.append(page()->mainFrame()->findAllElements("link[type=\"application/atom+xml\"]"));
    if (col.count() != 0)
        return true;

    return false;
}


void WebTab::showRSSInfo(const QPoint &pos)
{
    QWebElementCollection col = page()->mainFrame()->findAllElements("link[type=\"application/rss+xml\"]");
    col.append(page()->mainFrame()->findAllElements("link[type=\"application/atom+xml\"]"));

    QMap<KUrl, QString> map;

    Q_FOREACH(const QWebElement & el, col)
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
    }

    RSSWidget *widget = new RSSWidget(map, window());
    widget->showAt(pos);
}


void WebTab::hideSelectorBar()
{
    m_previewSelectorBar.data()->animatedHide();
}


void WebTab::setPart(KParts::ReadOnlyPart *p, const KUrl &u)
{
    if (p)
    {
        // Ok, part exists. Insert & show it..
        m_part = p;
        qobject_cast<QVBoxLayout *>(layout())->insertWidget(1, p->widget());
        p->openUrl(u);
        m_webView->hide();

        emit titleChanged(u.url());
        return;
    }

    if (!m_part)
        return;

    // Part NO more exists. Let's clean up from webtab
    m_webView->show();
    qobject_cast<QVBoxLayout *>(layout())->removeWidget(m_part->widget());
    delete m_part;
    m_part = 0;
}


KUrl WebTab::extractOpensearchUrl(QWebElement e)
{
    QString href = e.attribute(QL1S("href"));
    KUrl url = KUrl(href);
    if (!href.contains(":"))
    {
        KUrl docUrl = m_webView->url();
        QString host = docUrl.scheme() + "://" + docUrl.host();
        if (docUrl.port() != -1)
        {
            host += QL1C(':') + QString::number(docUrl.port());
        }
        url = KUrl(docUrl, href);
    }
    return url;
}


bool WebTab::hasNewSearchEngine()
{
    QWebElement e = page()->mainFrame()->findFirstElement(QL1S("head >link[rel=\"search\"][ type=\"application/opensearchdescription+xml\"]"));
    return !e.isNull() && !rApp->opensearchManager()->engineExists(extractOpensearchUrl(e));
}


void WebTab::showSearchEngine(const QPoint &pos)
{
    QWebElement e = page()->mainFrame()->findFirstElement(QL1S("head >link[rel=\"search\"][ type=\"application/opensearchdescription+xml\"]"));
    QString title = e.attribute(QL1S("title"));
    if (!title.isEmpty())
    {
        WebShortcutWidget *widget = new WebShortcutWidget(window());
        widget->setWindowFlags(Qt::Popup);

        connect(widget, SIGNAL(webShortcutSet(KUrl,QString,QString)),
                rApp->opensearchManager(), SLOT(addOpenSearchEngine(KUrl,QString,QString)));
        connect(rApp->opensearchManager(), SIGNAL(openSearchEngineAdded(QString,QString,QString)),
                this, SLOT(openSearchEngineAdded()));

        widget->show(extractOpensearchUrl(e), title, pos);
    }
}


void WebTab::openSearchEngineAdded()
{
    // If the providers changed, tell sycoca to rebuild its database...
    KBuildSycocaProgressDialog::rebuildKSycoca(this);

    disconnect(rApp->opensearchManager(), SIGNAL(openSearchEngineAdded(QString,QString,QString)),
               this, SLOT(openSearchEngineAdded()));
}


void WebTab::showMessageBar()
{
    MessageBar *msgBar = new MessageBar(i18n("It seems rekonq was not closed properly. Do you want "
                                        "to restore the last saved session?"), this);

    qobject_cast<QVBoxLayout *>(layout())->insertWidget(0, msgBar);
    msgBar->animatedShow();

    connect(msgBar, SIGNAL(accepted()), rApp->sessionManager(), SLOT(restoreCrashedSession()));
}


bool WebTab::hasAdBlockedElements()
{
    return page()->hasAdBlockedElements();
}


QPixmap WebTab::tabPreview(int width, int height)
{
    if (isPageLoading())
    {
        // no previews during load
        return QPixmap();
    }

    if (!part())
    {
        return WebSnap::renderPagePreview(*page(), width, height);
    }
    else
    {
        QWidget *partWidget = part()->widget();
        QPixmap partThumb(partWidget->size());

        partWidget->render(&partThumb);

        return partThumb.scaled(width, height, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    }
}


void WebTab::loadFinished()
{
    // add page to history
    QString pageTitle = (page() && page()->isOnRekonqPage()) ? url().url() : m_webView->title();
    rApp->historyManager()->addHistoryEntry(url(), pageTitle);
}


void WebTab::showSearchEngineBar()
{
    SearchEngineBar *seBar = new SearchEngineBar(this);

    qobject_cast<QVBoxLayout *>(layout())->insertWidget(0, seBar);
    seBar->animatedShow();
}
