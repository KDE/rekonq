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
#include "historymanager.h"
#include "iconmanager.h"
#include "sessionmanager.h"
#include "syncmanager.h"

#include "crashmessagebar.h"
#include "previewselectorbar.h"
#include "searchenginebar.h"
#include "walletbar.h"
#include "webpage.h"
#include "websnap.h"

#include "webwindow.h"

// KDE Includes
#include <KWebWallet>
#include <KStandardShortcut>
#include <KMenu>
#include <KActionMenu>
#include <kdeprintdialog.h>
#include <KLocalizedString>

#include <KParts/Part>
#include <KParts/BrowserExtension>

#ifdef HAVE_KACTIVITIES
#include <KActivities/ResourceInstance>
#endif

// Qt Includes
#include <QVBoxLayout>
#include <QPrintDialog>
#include <QPrinter>
#include <QSplitter>

#include <QWebSettings>


WebTab::WebTab(QWidget *parent, bool isPrivateBrowsing)
    : QWidget(parent)
    , m_webView(0)
    , m_progress(0)
    , m_part(0)
    , m_zoomFactor(10)
    , m_isPrivateBrowsing(isPrivateBrowsing)
    , m_isWebApp(false)
    , m_splitter(new QSplitter(this))
#ifdef HAVE_KACTIVITIES
    , m_activityResourceInstance(0)
#endif
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QVBoxLayout *l = new QVBoxLayout(this);
    l->setMargin(0);
    l->setSpacing(0);

    m_splitter->addWidget(view());
    view()->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // NOTE: setting web inspector vertical/horizontal
    m_splitter->setOrientation(Qt::Vertical);

    l->addWidget(m_splitter);

    // fix focus handling
    setFocusProxy(view());


    KWebWallet *wallet = page()->wallet();

    if (wallet)
    {
        connect(wallet, SIGNAL(saveFormDataRequested(QString,QUrl)),
                this, SLOT(createWalletBar(QString,QUrl)));
    }

    // Connect webview signals with related webtab ones
    connect(view(), SIGNAL(loadFinished(bool)),     this, SIGNAL(loadFinished(bool)));
    connect(view(), SIGNAL(loadProgress(int)),      this, SIGNAL(loadProgress(int)));
    connect(view(), SIGNAL(loadStarted()),          this, SIGNAL(loadStarted()));
    connect(view(), SIGNAL(urlChanged(QUrl)),       this, SIGNAL(urlChanged(QUrl)));
    connect(view(), SIGNAL(titleChanged(QString)),  this, SIGNAL(titleChanged(QString)));
    connect(view(), SIGNAL(iconChanged()),          this, SIGNAL(iconChanged()));

    if (!parent)
    {
        m_isWebApp = true;
        connect(this, SIGNAL(titleChanged(QString)), this, SLOT(webAppTitleChanged(QString)));
        connect(this, SIGNAL(iconChanged()), this, SLOT(webAppIconChanged()));
    }

    connect(view(), SIGNAL(loadProgress(int)), this, SLOT(updateProgress(int)));
    connect(view(), SIGNAL(loadStarted()), this, SLOT(resetProgress()));
    connect(view(), SIGNAL(loadFinished(bool)), this, SLOT(loadFinished()));

    // Session Manager
    connect(view(), SIGNAL(loadFinished(bool)), SessionManager::self(), SLOT(saveSession()));
    
#ifdef HAVE_KACTIVITIES
    if (m_isPrivateBrowsing)
        return;
    
    m_activityResourceInstance = new KActivities::ResourceInstance(window()->winId(), this);
    
    connect(this, SIGNAL(urlChanged(QUrl)), m_activityResourceInstance, SLOT(setUri(QUrl)));
    connect(this, SIGNAL(titleChanged(QString)), m_activityResourceInstance, SLOT(setTitle(QString)));
#endif
}


WebTab::~WebTab()
{
    m_walletBar.clear();
    m_previewSelectorBar.clear();

    // Get sure m_part will be deleted
    delete m_part;
}


WebView *WebTab::view()
{
    if (!m_webView)
    {
        m_webView = new WebView(this, m_isPrivateBrowsing);
    }
    return m_webView;
}


WebPage *WebTab::page()
{
    if (view())
        return view()->page();

    return 0;
}


WebWindow *WebTab::webWindow()
{
    WebWindow *w = qobject_cast<WebWindow *>(parent());
    return w;
}


KUrl WebTab::url()
{
    if (page() && page()->isOnRekonqPage())
    {
        return page()->loadingUrl();
    }

    if (view())
        return view()->url();

    kDebug() << "OOPS... NO web classes survived! Returning an empty url...";
    return KUrl();
}


QString WebTab::title()
{
    if (view() && url().protocol() == QL1S("rekonq"))
        return view()->title();
    
    if (page() && page()->isOnRekonqPage())
    {
        return url().url();
    }

    if (view())
        return view()->title();

    kDebug() << "OOPS... NO web classes survived! Returning an empty title...";
    return QString();

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


bool WebTab::hasRSSInfo()
{
    QWebElementCollection col = page()->mainFrame()->findAllElements("link[type=\"application/rss+xml\"]");
    col.append(page()->mainFrame()->findAllElements("link[type=\"application/atom+xml\"]"));
    if (col.count() != 0)
        return true;

    return false;
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
            SyncManager::self(), SLOT(syncPasswords()), Qt::UniqueConnection);
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

    connect(this, SIGNAL(loadStarted()),      m_previewSelectorBar.data(), SLOT(loadProgress()), Qt::UniqueConnection);
    connect(this, SIGNAL(loadProgress(int)),  m_previewSelectorBar.data(), SLOT(loadProgress()), Qt::UniqueConnection);
    connect(this, SIGNAL(loadFinished(bool)), m_previewSelectorBar.data(), SLOT(loadFinished()), Qt::UniqueConnection);
    connect(this, SIGNAL(urlChanged(QUrl)),   m_previewSelectorBar.data(), SLOT(verifyUrl()),    Qt::UniqueConnection);
}


void WebTab::hideSelectorBar()
{
    m_previewSelectorBar.data()->animatedHide();
}


KParts::ReadOnlyPart *WebTab::part()
{
    return m_part;
}


void WebTab::setPart(KParts::ReadOnlyPart *p, const KUrl &u)
{
    if (p)
    {
        // Ok, part exists. Insert & show it..
        m_part = p;
        qobject_cast<QVBoxLayout *>(layout())->insertWidget(0, p->widget());
        p->openUrl(u);
        view()->hide();
        m_splitter->hide();
        emit titleChanged(u.url());
        emit urlChanged(u.url());
        return;
    }

    if (!m_part)
        return;

    // Part NO more exists. Let's clean up from webtab
    view()->show();
    m_splitter->show();
    qobject_cast<QVBoxLayout *>(layout())->removeWidget(m_part->widget());
    delete m_part;
    m_part = 0;
}


void WebTab::showCrashMessageBar()
{
    CrashMessageBar *msgBar = new CrashMessageBar(i18n("It seems rekonq was not closed properly. Do you want "
            "to restore the last saved session?"), this);

    qobject_cast<QVBoxLayout *>(layout())->insertWidget(0, msgBar);
    msgBar->animatedShow();

    connect(msgBar, SIGNAL(accepted()), SessionManager::self(), SLOT(restoreCrashedSession()));
}


void WebTab::loadFinished()
{
    // add page to history, if not in private browsing mode
    if (page()->settings()->testAttribute(QWebSettings::PrivateBrowsingEnabled))
        return;

    QString pageTitle = (page() && page()->isOnRekonqPage()) ? url().prettyUrl() : view()->title();
    HistoryManager::self()->addHistoryEntry(url(), pageTitle);
}


void WebTab::showSearchEngineBar()
{
    SearchEngineBar *seBar = new SearchEngineBar(this);

    qobject_cast<QVBoxLayout *>(layout())->insertWidget(0, seBar);
    seBar->animatedShow();
}


void WebTab::printFrame()
{
    if (page()->isOnRekonqPage())
    {
        // trigger print part action instead of ours..
        KParts::ReadOnlyPart *p = part();
        if (p)
        {
            KParts::BrowserExtension *ext = p->browserExtension();
            if (ext)
            {
                KParts::BrowserExtension::ActionSlotMap *actionSlotMap = KParts::BrowserExtension::actionSlotMapPtr();

                connect(this, SIGNAL(triggerPartPrint()), ext, actionSlotMap->value("print"));
                emit triggerPartPrint();

                return;
            }
        }
    }

    QWebFrame *printFrame = page()->currentFrame();
    if (printFrame == 0)
    {
        printFrame = page()->mainFrame();
    }

    QPrinter printer;
    printer.setDocName(printFrame->title());
    QPrintDialog *printDialog = KdePrint::createPrintDialog(&printer, this);

    if (printDialog) //check if the Dialog was created
    {
        if (printDialog->exec())
            printFrame->print(&printer);

        delete printDialog;
    }
}


void WebTab::zoomIn()
{
    if (m_zoomFactor == 50)
    {
        emit infoToShow(i18n("Max zoom reached: ") + QString::number(m_zoomFactor * 10) + QL1S("%"));
        return;
    }

    if (m_zoomFactor >= 20)
        m_zoomFactor += 5;
    else
        m_zoomFactor++;

    view()->setZoomFactor(QVariant(m_zoomFactor).toReal() / 10);

    // set zoom factor
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group(config, "Zoom");
    group.writeEntry(url().host(), m_zoomFactor);

    emit infoToShow(i18n("Zooming: ") + QString::number(m_zoomFactor * 10) + QL1S("%"));
}


void WebTab::zoomOut()
{
    if (m_zoomFactor == 1)
    {
        emit infoToShow(i18n("Min zoom reached: ") + QString::number(m_zoomFactor * 10) + QL1S("%"));
        return;
    }

    m_zoomFactor--;
    view()->setZoomFactor(QVariant(m_zoomFactor).toReal() / 10);

    // set zoom factor
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group(config, "Zoom");
    group.writeEntry(url().host(), m_zoomFactor);

    emit infoToShow(i18n("Zooming: ") + QString::number(m_zoomFactor * 10) + QL1S("%"));
}


void WebTab::zoomDefault()
{
    m_zoomFactor = 10;
    view()->setZoomFactor(QVariant(m_zoomFactor).toReal() / 10);

    // set zoom factor
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group(config, "Zoom");
    group.writeEntry(url().host(), m_zoomFactor);

    emit infoToShow(i18n("Default zoom: ") + QString::number(m_zoomFactor * 10) + QL1S("%"));
}


void WebTab::webAppTitleChanged(QString title)
{
    if (title.isEmpty())
        setWindowTitle(i18n("rekonq"));
    else
        setWindowTitle(title);
}


void WebTab::webAppIconChanged()
{
    setWindowIcon(IconManager::self()->iconForUrl(url()));
}


void WebTab::toggleInspector(bool on)
{
    if (on)
    {
        page()->settings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, on);

        if (m_inspector.isNull())
        {
            m_inspector = new QWebInspector(this);
            m_inspector.data()->setPage(page());

            m_splitter->addWidget(m_inspector.data());
        }

        m_inspector.data()->show();

        return;
    }
    // else

    m_inspector.data()->hide();

    page()->settings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, on);
}


void WebTab::focusIn()
{
#ifdef HAVE_KACTIVITIES
    if (m_isPrivateBrowsing || !m_activityResourceInstance)
        return;
    
    m_activityResourceInstance->notifyFocusedIn();
#endif
}


void WebTab::focusOut()
{
#ifdef HAVE_KACTIVITIES
    if (m_isPrivateBrowsing || !m_activityResourceInstance)
        return;
    
    m_activityResourceInstance->notifyFocusedOut();    
#endif
}


bool WebTab::isWebApp()
{
    return m_isWebApp;
}
