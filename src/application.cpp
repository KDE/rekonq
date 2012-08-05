/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2008-2012 by Andrea Diamantini <adjam7 at gmail dot com>
* Copyright (C) 2009 by Paweł Prażak <pawelprazak at gmail dot com>
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
#include "application.h"
#include "application.moc"

// Auto Includes
#include "rekonq.h"

// Ui Includes
#include "ui_webappcreation.h"

// Local Includes
#include "adblockmanager.h"
#include "bookmarkmanager.h"
#include "downloadmanager.h"
#include "historymanager.h"
#include "iconmanager.h"
#include "mainview.h"
#include "mainwindow.h"
#include "opensearchmanager.h"
#include "searchengine.h"
#include "sessionmanager.h"
#include "syncmanager.h"
#include "stackedurlbar.h"
#include "tabbar.h"
#include "urlbar.h"
#include "urlresolver.h"
#include "useragentmanager.h"
#include "webtab.h"

// KDE Includes
#include <KCmdLineArgs>
#include <KIcon>
#include <KMessageBox>
#include <KStandardDirs>
#include <KStartupInfo>
#include <ThreadWeaver/Weaver>
#include <KAction>
#include <KWindowSystem>
#include <KWindowInfo>
#include <KGlobal>
#include <KCharsets>
#include <KPushButton>
#include <KMimeType>

// Qt Includes
#include <QVBoxLayout>
#include <QTimer>


using namespace ThreadWeaver;


Application::Application()
    : KUniqueApplication()
    , _privateBrowsingAction(0)
{
    _privateBrowsingAction = new KAction(KIcon("view-media-artist"), i18n("Private &Browsing"), this);
    _privateBrowsingAction->setCheckable(true);
    connect(_privateBrowsingAction, SIGNAL(triggered(bool)), this, SLOT(setPrivateBrowsingMode(bool)));
}


Application::~Application()
{
    // ok, we are closing well.
    // Don't recover on next load..
    ReKonfig::setRecoverOnCrash(0);
    saveConfiguration();

    Q_FOREACH(QWeakPointer<MainWindow> window, m_mainWindows)
    {
        kDebug() << "deleting windows...";
        delete window.data();
        window.clear();
    }

    if (!m_historyManager.isNull())
    {
        kDebug() << "deleting history manager";
        delete m_historyManager.data();
        m_historyManager.clear();
    }

    if (!m_bookmarkManager.isNull())
    {
        kDebug() << "deleting bookmark Manager";
        delete m_bookmarkManager.data();
        m_bookmarkManager.clear();
    }

    if (!m_sessionManager.isNull())
    {
        kDebug() << "deleting session manager";
        delete m_sessionManager.data();
        m_sessionManager.clear();
    }

    if (!m_opensearchManager.isNull())
    {
        kDebug() << "deleting opensearch manager";
        delete m_opensearchManager.data();
        m_opensearchManager.clear();
    }

    if (!m_iconManager.isNull())
    {
        kDebug() << "deleting icon manager";
        delete m_iconManager.data();
        m_iconManager.clear();
    }

    if (!m_adblockManager.isNull())
    {
        kDebug() << "deleting adblock manager";
        delete m_adblockManager.data();
        m_adblockManager.clear();
    }

    if (!m_userAgentManager.isNull())
    {
        kDebug() << "deleting user agent manager";
        delete m_userAgentManager.data();
        m_userAgentManager.clear();
    }

    if (!m_syncManager.isNull())
    {
        kDebug() << "deleting sync manager";
        delete m_syncManager.data();
        m_syncManager.clear();
    }

    // TODO:
    // add a check to NOT close rekonq
    // until last download is finished
    if (!m_downloadManager.isNull())
    {
        kDebug() << "deleting download manager";
        delete m_downloadManager.data();
        m_downloadManager.clear();
    }

    kDebug() << "Bye bye...";
}


int Application::newInstance()
{
    KCmdLineArgs* args = KCmdLineArgs::parsedArgs();

    // not that easy, indeed
    // We have to consider 3 variables here:
    // 1) Is first load?
    // 2) Are there arguments?
    // 3) Is rekonq recovering from crash?
    // so, we have 8 possible cases...
    static bool isFirstLoad = true;
    bool areThereArguments = (args->count() > 0);
    bool hasToBeRecoveredFromCrash = (ReKonfig::recoverOnCrash() > 0);
    // note that hasToBeRecoveredFromCrash is always true if it is not the first load
    // !isFirstLoad -> hasToBeRecovered

    kDebug() << "is first load? " << isFirstLoad;
    kDebug() << "are there arguments? " << areThereArguments;
    kDebug() << "is rekonq crashed? " << hasToBeRecoveredFromCrash;

    if (areThereArguments)
    {
        // prepare URLS to load
        KUrl::List urlList;
        for (int i = 0; i < args->count(); ++i)
        {
            const KUrl u = args->url(i);

            if (u.isLocalFile() && QFile::exists(u.toLocalFile())) // "rekonq somefile.html" case
                urlList += u;
            else
            {
                // "rekonq kde.org" || "rekonq kde:kdialog" case
                UrlResolver res(args->arg(i));
                UrlSearchList list = res.orderedSearchItems();
                if (list.isEmpty())
                {
                    urlList += u;
                }
                else
                {
                    urlList += list.first().url;
                }
            }
        }

        if (isFirstLoad && (ReKonfig::startupBehaviour() == 2) && sessionManager()->restoreSessionFromScratch())
        {
            isFirstLoad = false;
        }

        // first argument: 99% of the time we have just that...
        if (isFirstLoad)
        {
            // No windows in the current desktop? No windows at all?
            // Create a new one and load there sites...
            loadUrl(urlList.at(0), Rekonq::NewWindow);
        }
        else
        {
            if (!ReKonfig::openExternalLinksInNewWindow())
            {
                loadUrl(urlList.at(0), Rekonq::NewFocusedTab);
            }
            else
            {
                loadUrl(urlList.at(0), Rekonq::NewWindow);
            }

            if (!mainWindow()->isActiveWindow())
                KWindowSystem::demandAttention(mainWindow()->winId(), true);
        }

        // following arguments: what's best behavior here?
        // I'm pretty sure no one has real opinion about...
        if (!ReKonfig::openExternalLinksInNewWindow())
        {
            for (int i = 1; i < urlList.count(); ++i)
                loadUrl(urlList.at(i), Rekonq::NewFocusedTab);
        }
        else
        {
            for (int i = 1; i < urlList.count(); ++i)
                loadUrl(urlList.at(i), Rekonq::NewWindow);
        }
    }
    else
    {
        if (isFirstLoad)
        {
            if (hasToBeRecoveredFromCrash)
            {
                loadUrl(KUrl("about:closedTabs"), Rekonq::NewWindow);
            }
            else
            {
                // NOTE: just load new tabs/windows without arguments
                // if NOT is Session restored...
                if (!isSessionRestored())
                {
                    switch (ReKonfig::startupBehaviour())
                    {
                    case 0: // open home page
                        newMainWindow()->homePage();
                        break;
                    case 1: // open new tab page
                        loadUrl(KUrl("about:home"), Rekonq::NewWindow);
                        break;
                    case 2: // restore session
                        if (sessionManager()->restoreSessionFromScratch())
                        {
                            break;
                        }
                    default:
                        newMainWindow()->homePage();
                        break;
                    }
                }
            }
        }
        else
        {
            switch (ReKonfig::newTabsBehaviour())
            {
            case 0: // new tab page
                loadUrl(KUrl("about:home") , Rekonq::NewWindow);
                break;
            case 2: // homepage
                loadUrl(KUrl(ReKonfig::homePage()) , Rekonq::NewWindow);
                break;
            case 1: // blank page
            default:
                loadUrl(KUrl("about:blank") , Rekonq::NewWindow);
                break;
            }
        }
    }

    if (isFirstLoad)
    {
        if (hasToBeRecoveredFromCrash)
        {
            QTimer::singleShot(1000, mainWindow()->currentTab(), SLOT(showMessageBar()));
        }
        else
        {
            sessionManager()->setSessionManagementEnabled(true);
        }

        if (ReKonfig::checkDefaultSearchEngine()
                && !hasToBeRecoveredFromCrash
                && SearchEngine::defaultEngine().isNull())
            QTimer::singleShot(2000, mainWindow()->currentTab(), SLOT(showSearchEngineBar()));

        // updating rekonq configuration
        updateConfiguration();

        setWindowIcon(KIcon("rekonq"));

        historyManager();

        ReKonfig::setRecoverOnCrash(ReKonfig::recoverOnCrash() + 1);
        saveConfiguration();
    }

    KStartupInfo::appStarted();
    isFirstLoad = false;

    return 0;
}


Application *Application::instance()
{
    return (qobject_cast<Application *>(QCoreApplication::instance()));
}


void Application::saveConfiguration() const
{
    ReKonfig::self()->writeConfig();
}


MainWindow *Application::mainWindow()
{
    MainWindow *active = qobject_cast<MainWindow*>(QApplication::activeWindow());

    if (!active)
    {
        if (m_mainWindows.isEmpty())
            return 0;

        Q_FOREACH(const QWeakPointer<MainWindow> &pointer, m_mainWindows)
        {
            if (KWindowInfo(pointer.data()->effectiveWinId(), NET::WMDesktop, 0).isOnCurrentDesktop())
                return pointer.data();
        }
        return m_mainWindows.at(0).data();
    }
    return active;
}


HistoryManager *Application::historyManager()
{
    if (m_historyManager.isNull())
    {
        m_historyManager = new HistoryManager;
    }
    return m_historyManager.data();
}


BookmarkManager *Application::bookmarkManager()
{
    if (m_bookmarkManager.isNull())
    {
        m_bookmarkManager = new BookmarkManager;
    }
    return m_bookmarkManager.data();
}


SessionManager *Application::sessionManager()
{
    if (m_sessionManager.isNull())
    {
        m_sessionManager = new SessionManager;
    }
    return m_sessionManager.data();
}


OpenSearchManager *Application::opensearchManager()
{
    if (m_opensearchManager.isNull())
    {
        m_opensearchManager = new OpenSearchManager;
    }
    return m_opensearchManager.data();
}


IconManager *Application::iconManager()
{
    if (m_iconManager.isNull())
    {
        m_iconManager = new IconManager;
    }
    return m_iconManager.data();
}


DownloadManager *Application::downloadManager()
{
    if (m_downloadManager.isNull())
    {
        m_downloadManager = new DownloadManager(instance());
    }
    return m_downloadManager.data();
}


UserAgentManager *Application::userAgentManager()
{
    if (m_userAgentManager.isNull())
    {
        m_userAgentManager = new UserAgentManager(instance());
    }
    return m_userAgentManager.data();
}


SyncManager *Application::syncManager()
{
    if (m_syncManager.isNull())
    {
        m_syncManager = new SyncManager(instance());
    }
    return m_syncManager.data();
}


void Application::loadUrl(const KUrl& url, const Rekonq::OpenType& type)
{
    if (url.isEmpty())
        return;

    if (!url.isValid())
    {
        KMessageBox::error(0, i18n("Malformed URL:\n%1", url.url(KUrl::RemoveTrailingSlash)));
        return;
    }

    Rekonq::OpenType newType = type;
    // Don't open useless tabs or windows for actions in about: pages
    if (url.url().contains("about:") && url.url().contains("/"))
        newType = Rekonq::CurrentTab;

    MainWindow *w = 0;
    if (newType == Rekonq::NewWindow
            || (newType == Rekonq::NewTab && ReKonfig::openLinksInNewWindow()))
    {
        w = newMainWindow();
        newType = Rekonq::CurrentTab;
    }
    else
    {
        w = mainWindow();
    }

    // be SURE window exists
    if (!w)
        w = newMainWindow();

    w->loadCheckedUrl(url, newType);
}


MainWindow *Application::newMainWindow(bool withTab)
{
    MainWindow *w = new MainWindow();
    // This is used to track which window was activated most recently
    w->installEventFilter(this);

    if (withTab)
        w->mainView()->newWebTab();    // remember using newWebTab and NOT newTab here!!

    m_mainWindows.prepend(w);
    w->show();

    return w;
}


void Application::removeMainWindow(MainWindow *window)
{
    m_mainWindows.removeOne(window);
    kDebug() << "Removing Window from app window list...";

    // bye bye...
    if (m_mainWindows.count() == 0)
        quit();
}


MainWindowList Application::mainWindowList()
{
    return m_mainWindows;
}


AdBlockManager *Application::adblockManager()
{
    if (m_adblockManager.isNull())
    {
        m_adblockManager = new AdBlockManager;
    }
    return m_adblockManager.data();
}


void Application::newWindow()
{
    loadUrl(KUrl("about:home"), Rekonq::NewWindow);
    mainWindow()->mainView()->currentUrlBar()->setFocus();
}


bool Application::eventFilter(QObject* watched, QEvent* event)
{
    // Track which window was activated most recently to prefer it on window choosing
    // (e.g. when another application opens a link)
    if (event->type() == QEvent::WindowActivate)
    {
        MainWindow *window = qobject_cast<MainWindow*>(watched);
        if (window)
        {
            if (m_mainWindows.at(0).data() != window)
            {
                int index = m_mainWindows.indexOf(QWeakPointer<MainWindow>(window));
                Q_ASSERT(index != -1);
                m_mainWindows.prepend(m_mainWindows.takeAt(index));
            }
        }
    }

    return QObject::eventFilter(watched, event);
}


void Application::updateConfiguration()
{
    // ============== Tabs ==================
    bool b = ReKonfig::closeTabSelectPrevious();
    Q_FOREACH(const QWeakPointer<MainWindow> &w, m_mainWindows)
    {
        MainView *mv = w.data()->mainView();
        mv->updateTabBarVisibility();

        mv->tabBar()->setAnimatedTabHighlighting(ReKonfig::animatedTabHighlighting());

        if (b)
            mv->tabBar()->setSelectionBehaviorOnRemove(QTabBar::SelectPreviousTab);
        else
            mv->tabBar()->setSelectionBehaviorOnRemove(QTabBar::SelectRightTab);
    }

    QWebSettings *defaultSettings = QWebSettings::globalSettings();

    // =========== Fonts ==============
    defaultSettings->setFontFamily(QWebSettings::StandardFont, ReKonfig::standardFontFamily());
    defaultSettings->setFontFamily(QWebSettings::FixedFont, ReKonfig::fixedFontFamily());
    defaultSettings->setFontFamily(QWebSettings::SerifFont, ReKonfig::serifFontFamily());
    defaultSettings->setFontFamily(QWebSettings::SansSerifFont, ReKonfig::sansSerifFontFamily());
    defaultSettings->setFontFamily(QWebSettings::CursiveFont, ReKonfig::cursiveFontFamily());
    defaultSettings->setFontFamily(QWebSettings::FantasyFont, ReKonfig::fantasyFontFamily());

    // compute font size
    // (I have to admit I know nothing about these DPI questions..: copied from kwebkitpart, as someone suggested)
    // font size in pixels =  font size in inches × screen dpi
    if (mainWindow() && mainWindow()->currentTab())
    {
        int logDpiY = mainWindow()->currentTab()->view()->logicalDpiY();
        float toPix = (logDpiY < 96.0)
                      ? 96.0 / 72.0
                      : logDpiY / 72.0 ;

        int defaultFontSize = ReKonfig::defaultFontSize();
        int minimumFontSize = ReKonfig::minFontSize();

        defaultSettings->setFontSize(QWebSettings::DefaultFontSize, qRound(defaultFontSize * toPix));
        defaultSettings->setFontSize(QWebSettings::MinimumFontSize, qRound(minimumFontSize * toPix));
    }

    // encodings
    QString enc = ReKonfig::defaultEncoding();
    defaultSettings->setDefaultTextEncoding(enc);

    // ================ WebKit ============================
    defaultSettings->setAttribute(QWebSettings::DnsPrefetchEnabled, ReKonfig::dnsPrefetch());
    defaultSettings->setAttribute(QWebSettings::PrintElementBackgrounds, ReKonfig::printElementBackgrounds());

    defaultSettings->setAttribute(QWebSettings::JavascriptEnabled, ReKonfig::javascriptEnabled());
    defaultSettings->setAttribute(QWebSettings::JavascriptCanOpenWindows, ReKonfig::javascriptCanOpenWindows());
    defaultSettings->setAttribute(QWebSettings::JavascriptCanAccessClipboard, ReKonfig::javascriptCanAccessClipboard());

    defaultSettings->setAttribute(QWebSettings::JavaEnabled, ReKonfig::javaEnabled());

    if (ReKonfig::pluginsEnabled() == 2)
        defaultSettings->setAttribute(QWebSettings::PluginsEnabled, false);
    else
        defaultSettings->setAttribute(QWebSettings::PluginsEnabled, true);

    // Enabling WebKit "Page Cache" feature: http://webkit.org/blog/427/webkit-page-cache-i-the-basics/
    defaultSettings->setMaximumPagesInCache(3);

    // ===== HTML 5 features WebKit support ======
    defaultSettings->setAttribute(QWebSettings::OfflineStorageDatabaseEnabled, ReKonfig::offlineStorageDatabaseEnabled());
    defaultSettings->setAttribute(QWebSettings::OfflineWebApplicationCacheEnabled, ReKonfig::offlineWebApplicationCacheEnabled());
    defaultSettings->setAttribute(QWebSettings::LocalStorageEnabled, ReKonfig::localStorageEnabled());
    if (ReKonfig::localStorageEnabled())
    {
        QString path = KStandardDirs::locateLocal("cache", QString("WebkitLocalStorage/rekonq"), true);
        path.remove("rekonq");
        QWebSettings::setOfflineStoragePath(path);
        QWebSettings::setOfflineStorageDefaultQuota(50000);
    }

    // ================= WebGl ===================
    defaultSettings->setAttribute(QWebSettings::WebGLEnabled, ReKonfig::webGL());
    defaultSettings->setAttribute(QWebSettings::AcceleratedCompositingEnabled, ReKonfig::webGL());

    // Applies user defined CSS to all open webpages.
    defaultSettings->setUserStyleSheetUrl(ReKonfig::userCSS());

    // ====== load Settings on main classes
    historyManager()->loadSettings();

    defaultSettings = 0;

    if (!mainWindow())
        return;

    ReKonfig::useFavicon()
    ? mainWindow()->changeWindowIcon(mainWindow()->mainView()->currentIndex())
    : mainWindow()->setWindowIcon(KIcon("rekonq"))
    ;

    // hovering unfocused tabs options
    switch (ReKonfig::hoveringTabOption())
    {
    case 0: // tab previews
    case 3: // nothing
        for (int i = 0; i < mainWindow()->mainView()->tabBar()->count(); i++)
        {
            mainWindow()->mainView()->tabBar()->setTabToolTip(i, QL1S(""));
        }
        break;

    case 1: // title previews
        for (int i = 0; i < mainWindow()->mainView()->tabBar()->count(); i++)
        {
            mainWindow()->mainView()->tabBar()->setTabToolTip(i, mainWindow()->mainView()->tabText(i).remove('&'));
        }
        break;

    case 2: // url previews
        for (int i = 0; i < mainWindow()->mainView()->tabBar()->count(); i++)
        {
            mainWindow()->mainView()->tabBar()->setTabToolTip(i, mainWindow()->mainView()->webTab(i)->url().toMimeDataString());
        }
        break;

    default: // non extant case
        ASSERT_NOT_REACHED(unknown hoveringTabOption);
        break;
    }

}


void Application::setPrivateBrowsingMode(bool b)
{
// NOTE
// to let work nicely Private Browsing, we need the following:
// - enable WebKit Private Browsing mode :)
// - treat all cookies as session cookies
//  (so that they do not get saved to a persistent storage). Available from KDE SC 4.5.72, see BUG: 250122
// - favicons (fixed in rekonq 0.5.87)
// - save actual session (to restore it when Private Mode is closed) and stop storing it
// - disable history saving

    QWebSettings *settings = QWebSettings::globalSettings();
    bool isJustEnabled = settings->testAttribute(QWebSettings::PrivateBrowsingEnabled);
    if (isJustEnabled == b)
        return;     // uhm... something goes wrong...

    if (b)
    {
        QString caption = i18n("Are you sure you want to turn on private browsing?");
        QString text = i18n("<b>%1</b>"
                            "<p>rekonq will save your current tabs for when you'll stop private browsing the net.</p>", caption);

        int button = KMessageBox::warningContinueCancel(mainWindow(),
                     text, caption, KStandardGuiItem::cont(), KStandardGuiItem::cancel(),
                     i18n("don't ask again"));
        if (button != KMessageBox::Continue)
        {
            // The user canceled so we should uncheck the box
            _privateBrowsingAction->setChecked(false);
            return;
        }

        sessionManager()->setSessionManagementEnabled(false);
        settings->setAttribute(QWebSettings::PrivateBrowsingEnabled, true);
        _privateBrowsingAction->setChecked(true);

        loadUrl(KUrl("about:home"), Rekonq::NewWindow);

        MainWindow *activeOne = m_mainWindows.at(0).data();

        Q_FOREACH(const QWeakPointer<MainWindow> &w, m_mainWindows)
        {
            if (w.data() != activeOne)
                w.data()->close();
        }
    }
    else
    {
        settings->setAttribute(QWebSettings::PrivateBrowsingEnabled, false);
        _privateBrowsingAction->setChecked(false);

        int newWindows = sessionManager()->restoreSavedSession();
        if (newWindows == 0)
        {
            loadUrl(KUrl("about:home"), Rekonq::NewWindow);
            newWindows++;
        }

        for (int i = newWindows; i < m_mainWindows.count(); ++i)
        {
            m_mainWindows.at(i).data()->close();
        }

        sessionManager()->setSessionManagementEnabled(true);
    }
}


void Application::queryQuit()
{
    if (mainWindowList().count() > 1)
    {
        int answer = KMessageBox::questionYesNoCancel(
                         mainWindow(),
                         i18n("Do you want to close the window or the whole application?"),
                         i18n("Application/Window closing..."),
                         KGuiItem(i18n("C&lose Current Window"),
                                  KIcon("window-close")),
                         KStandardGuiItem::quit(),
                         KStandardGuiItem::cancel(),
                         "confirmClosingMultipleWindows"
                     );

        switch (answer)
        {
        case KMessageBox::Yes:
            mainWindow()->close();
            return;

        case KMessageBox::No:
            break;

        default:
            return;
        }
    }

    // in case of just one window...
    quit();
}


void Application::createWebAppShortcut()
{
    KUrl u = mainWindow()->currentTab()->url();
    QString h = u.host();

    QPointer<KDialog> dialog = new KDialog(mainWindow());
    dialog->setCaption(i18nc("@title:window", "Create Application Shortcut"));
    dialog->setButtons(KDialog::Ok | KDialog::Cancel);
    dialog->button(KDialog::Ok)->setText(i18n("Create"));
    dialog->setMinimumSize(400, 50);
    dialog->setWindowIcon(QIcon(iconManager()->iconForUrl(u).pixmap(16)));

    Ui::webAppCreation wAppWidget;
    QWidget widget;
    wAppWidget.setupUi(&widget);

    const QString title = mainWindow()->currentTab()->view()->title().remove('&');
    wAppWidget.nameLineEdit->setText(title);
    wAppWidget.kcfg_createDesktopAppShortcut->setChecked(ReKonfig::createDesktopAppShortcut());
    wAppWidget.kcfg_createMenuAppShortcut->setChecked(ReKonfig::createMenuAppShortcut());

    dialog->setMainWidget(&widget);
    dialog->exec();

    if (dialog->result() == QDialog::Accepted)
    {
        ReKonfig::setCreateDesktopAppShortcut(wAppWidget.kcfg_createDesktopAppShortcut->isChecked());
        ReKonfig::setCreateMenuAppShortcut(wAppWidget.kcfg_createMenuAppShortcut->isChecked());

        iconManager()->saveDesktopIconForUrl(u);
        QString iconPath = KStandardDirs::locateLocal("cache" , "favicons/" , true) + h + QL1S("_WEBAPPICON.png");

        QString shortcutString = QL1S("#!/usr/bin/env xdg-open\n")
                                 + QL1S("[Desktop Entry]\n")
                                 + QL1S("Name=") + (wAppWidget.nameLineEdit->text().isEmpty() ? QL1S("kwebapp") : wAppWidget.nameLineEdit->text()) + QL1S("\n")
                                 + QL1S("GenericName=") + (wAppWidget.descriptionLineEdit->text().isEmpty() ? QL1S("") : wAppWidget.descriptionLineEdit->text()) + QL1S("\n")
                                 + QL1S("Icon=") + iconPath + QL1S("\n")
                                 + QL1S("Exec=kwebapp ") + u.url() + QL1S("\n")
                                 + QL1S("Type=Application\n")
                                 + QL1S("Categories=Application;Network\n")
                                 ;

        if (ReKonfig::createDesktopAppShortcut())
        {
            QString desktop = KGlobalSettings::desktopPath();
            QFile wAppFile(desktop + QL1C('/') + title);

            if (!wAppFile.open(QIODevice::WriteOnly | QIODevice::Text))
            {
                kDebug() << "Unable to open file: " << wAppFile.errorString();
                return;
            }

            QTextStream out(&wAppFile);
            out.setCodec("UTF-8");
            out << shortcutString;

            wAppFile.setPermissions(QFile::ReadUser | QFile::WriteUser | QFile::ExeUser | QFile::ReadGroup | QFile::ReadOther);
            wAppFile.close();
        }

        if (ReKonfig::createMenuAppShortcut())
        {
            QString appMenuDir = KStandardDirs::locateLocal("xdgdata-apps", QString());
            QFile wAppFile(appMenuDir + QL1C('/') + title + QL1S(".desktop"));

            if (!wAppFile.open(QIODevice::WriteOnly | QIODevice::Text))
            {
                kDebug() << "Unable to open file: " << wAppFile.errorString();
                return;
            }

            QTextStream out(&wAppFile);
            out.setCodec("UTF-8");
            out << shortcutString;

            wAppFile.close();
        }

    }

    dialog->deleteLater();
}
