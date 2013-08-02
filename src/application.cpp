/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2012 by Andrea Diamantini <adjam7 at gmail dot com>
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
#include "searchengine.h"

#include "tabbar.h"
#include "rekonqwindow.h"

#include "webwindow.h"
#include "webtab.h"
#include "webpage.h"

#include "urlresolver.h"

// Local Manager(s) Includes
#include "adblockmanager.h"
#include "downloadmanager.h"
#include "historymanager.h"
#include "iconmanager.h"
#include "sessionmanager.h"

// Ui Includes
#include "ui_cleardata.h"

// KDE Includes
#include <KCmdLineArgs>

#include <KDialog>
#include <KProcess>
#include <KPushButton>
#include <KStandardDirs>
#include <KWindowSystem>
#include <KWindowInfo>
#include <KStartupInfo>

#include <KMessageBox>

#include <config-kactivities.h>
#ifdef HAVE_KACTIVITIES
#include <KActivities/Consumer>
#endif

// Qt Includes
#include <QDBusInterface>
#include <QDBusReply>
#include <QDir>
#include <QTimer>


Application::Application()
    : KUniqueApplication()
{
#ifdef HAVE_KACTIVITIES
    m_activityConsumer = new KActivities::Consumer();
#endif
    
    // updating rekonq configuration
    updateConfiguration();

    setWindowIcon(KIcon("rekonq"));

    // just create History Manager...
    HistoryManager::self();
}


Application::~Application()
{
    // ok, we are closing well: don't recover on next load..
    ReKonfig::setRecoverOnCrash(0);
    saveConfiguration();

#ifdef HAVE_KACTIVITIES    
    delete m_activityConsumer;
#endif

    kDebug() << "Bye bye (k)baby...";
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
    // !isFirstLoad -> hasToBeRecoveredFromCrash

    kDebug() << "is first load? " << isFirstLoad;
    kDebug() << "are there arguments? " << areThereArguments;
    kDebug() << "is rekonq crashed? " << hasToBeRecoveredFromCrash;

    bool incognito = args->isSet("incognito");
    bool webapp = args->isSet("webapp");

    if (webapp)
    {
        kDebug() << "WEBAPP MODE...";
        if (args->count() == 0)
        {
            KMessageBox::error(0, i18n("Error"), i18n("Cannot launch webapp mode without an URL to load"));
            return 1;
        }

        if (args->count() > 1)
        {
            KMessageBox::error(0, i18n("Error"), i18n("Cannot load more than one URL in webapp mode"));
            return 1;
        }

        KUrl u = args->url(0);
        if (!u.isLocalFile() || !QFile::exists(u.toLocalFile()))
        {
            u = UrlResolver::urlFromTextTyped(args->arg(0));
        }
        kDebug() << "URL: " << u;

        loadUrl(u, Rekonq::WebApp);

        KStartupInfo::appStarted();
        isFirstLoad = false;
        return 0;
    }

    if (areThereArguments)
    {
        kDebug() << "DEFAULT MODE, WITH ARGUMENTS...";

        // prepare URLS to load
        KUrl::List urlList;
        for (int i = 0; i < args->count(); ++i)
        {
            const KUrl u = args->url(i);

            if (u.isLocalFile() && QFile::exists(u.toLocalFile())) // "rekonq somefile.html" case
            {
                urlList += u;
            }
            else
            {
                // "rekonq kde.org" || "rekonq kde:kdialog" cases
                urlList += UrlResolver::urlFromTextTyped(args->arg(i));
            }
        }

        if (isFirstLoad)
        {
            bool restoreOk = false;
            
            switch(ReKonfig::startupBehaviour())
            {
            case 2:
                if (hasToBeRecoveredFromCrash)
                    restoreOk = false;
                else
                    restoreOk = SessionManager::self()->restoreSessionFromScratch();
                break;
                
            case 3:
                SessionManager::self()->manageSessions();
                break;
                
            default:
                restoreOk = SessionManager::self()->restoreJustThePinnedTabs();
                break;
            }

            isFirstLoad = !restoreOk;
        }

        // first argument: 99% of the time we have just that...
        if (isFirstLoad || m_rekonqWindows.count() == 0)
        {
            // No windows in the current desktop? No windows at all?
            // Create a new one and load there sites...
            if (incognito)
                loadUrl(urlList.at(0), Rekonq::NewPrivateWindow);
            else
                loadUrl(urlList.at(0), Rekonq::NewWindow);
        }
        else
        {
            if (incognito)
            {
                loadUrl(urlList.at(0), Rekonq::NewPrivateWindow);
            }
            else if (!ReKonfig::openExternalLinksInNewWindow())
            {
                loadUrl(urlList.at(0), Rekonq::NewFocusedTab);
            }
            else
            {
                loadUrl(urlList.at(0), Rekonq::NewWindow);
            }

            if (!rekonqWindow()->isActiveWindow())
                KWindowSystem::demandAttention(rekonqWindow()->winId(), true);
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
                if (incognito)
                    loadUrl(urlList.at(i), Rekonq::NewPrivateWindow);
                else
                    loadUrl(urlList.at(i), Rekonq::NewWindow);
        }
    }
    else    // ok, NO arguments
    {
        kDebug() << "DEFAULT MODE, NO ARGUMENTS...";
        if (isFirstLoad)
        {
            // NOTE: just load new tabs/windows without arguments
            // if NOT is Session restored...
            if (!isSessionRestored())
            {
                switch (ReKonfig::startupBehaviour())
                {
                case 0: // open home page
                    if (incognito)
                    {
                        loadUrl(KUrl(ReKonfig::homePage()), Rekonq::NewPrivateWindow);
                        break;
                    }
                    if (SessionManager::self()->restoreJustThePinnedTabs())
                        loadUrl(KUrl(ReKonfig::homePage()) , Rekonq::NewTab);
                    else
                        loadUrl(KUrl(ReKonfig::homePage()) , Rekonq::NewWindow);
                    break;
                case 1: // open new tab page
                    if (incognito)
                    {
                        loadUrl(KUrl("rekonq:home"), Rekonq::NewPrivateWindow);
                        break;
                    }
                    if (SessionManager::self()->restoreJustThePinnedTabs())
                        loadUrl(KUrl("rekonq:home") , Rekonq::NewTab);
                    else
                        loadUrl(KUrl("rekonq:home"), Rekonq::NewWindow);
                    break;
                case 2: // restore session
                    if (incognito)
                    {
                        loadUrl(KUrl("rekonq:home"), Rekonq::NewPrivateWindow);
                        break;
                    }
                    if (hasToBeRecoveredFromCrash || !SessionManager::self()->restoreSessionFromScratch())
                    {
                        loadUrl(KUrl("rekonq:home") , Rekonq::NewTab);
                    }
                    break;
                case 3:
                    SessionManager::self()->manageSessions();
                    break;
                default:
                    ASSERT_NOT_REACHED(unknown startup behavior);
                    break;
                }
            }
        }
        else
        {
            Rekonq::OpenType type = incognito ? Rekonq::NewPrivateWindow : Rekonq::NewWindow;

            switch (ReKonfig::newTabsBehaviour())
            {
            case 0: // new tab page
                loadUrl(KUrl("rekonq:home") , type);
                break;
            case 2: // homepage
                loadUrl(KUrl(ReKonfig::homePage()) , type);
                break;
            case 1: // blank page
            default:
                loadUrl(KUrl("about:blank") , type);
                break;
            }
        }
    }

    // ok, now last stuffs...
    if (isFirstLoad)
    {
        if (hasToBeRecoveredFromCrash && !incognito)
        {
            if (rekonqWindow() && rekonqWindow()->currentWebWindow())
                QTimer::singleShot(1000, rekonqWindow()->currentWebWindow(), SLOT(showCrashMessageBar()));
        }
        else
        {
            SessionManager::self()->setSessionManagementEnabled(true);
        }

        if (ReKonfig::checkDefaultSearchEngine() && !hasToBeRecoveredFromCrash && SearchEngine::defaultEngine().isNull())
            QTimer::singleShot(2000, rekonqWindow()->currentWebWindow()->tabView(), SLOT(showSearchEngineBar()));

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


// --------------------------------------------------------------------------------------------------------------------


RekonqWindow *Application::rekonqWindow(const QString & activityID)
{
    RekonqWindow *active = qobject_cast<RekonqWindow*>(QApplication::activeWindow());

    if (!active)
    {
        RekonqWindowList wList = m_rekonqWindows;
#ifdef HAVE_KACTIVITIES
        wList = tabsForActivity(activityID);
#endif
        
        if (wList.isEmpty())
            return 0;
        
        Q_FOREACH(const QWeakPointer<RekonqWindow> &pointer, wList)
        {
            if (KWindowInfo(pointer.data()->effectiveWinId(), NET::WMDesktop, 0).isOnCurrentDesktop())
                return pointer.data();
        }
        return wList.at(0).data();
    }
    return active;
}


RekonqWindowList Application::tabsForActivity(const QString & activityID)
{
#ifdef HAVE_KACTIVITIES
    QString id = activityID;
    if ( id.isEmpty() )
        id = m_activityConsumer->currentActivity();
    
    return m_activityRekonqWindowsMap[id];
#else
    return m_rekonqWindows;
#endif
}


bool Application::haveWindowsForActivity(const QString & activityID)
{
    return (!tabsForActivity(activityID).isEmpty());
}


// -----------------------------------------------------------------------------------------------------------


RekonqWindow *Application::newWindow(bool withTab, bool PrivateBrowsingMode)
{
    RekonqWindow *w = new RekonqWindow(withTab, PrivateBrowsingMode);
    setWindowInfo(w);
    w->show();

    return w;
}


RekonqWindow *Application::newWindow(WebPage *pg)
{
    RekonqWindow *w = new RekonqWindow(pg);
    setWindowInfo(w);
    w->show();

    return w;    
}


void Application::setWindowInfo(RekonqWindow *w)
{
    // set object name
    int n = m_rekonqWindows.count() + 1;
    w->setObjectName(QL1S("win") + QString::number(n));

    // This is used to track which window was activated most recently
    w->installEventFilter(this);

    m_rekonqWindows.prepend(w);
    
#ifdef HAVE_KACTIVITIES
    QString currentActivity = m_activityConsumer->currentActivity();
    m_activityRekonqWindowsMap[currentActivity].prepend(w);
#endif    
}


// -----------------------------------------------------------------------------------------------------------


WebTab *Application::newWebApp()
{
    WebTab *tab = new WebTab;

    tab->installEventFilter(this);
    m_webApps.prepend(tab);

    tab->show();

    return tab;
}


RekonqWindowList Application::rekonqWindowList()
{
    return m_rekonqWindows;
}


WebAppList Application::webAppList()
{
    return m_webApps;
}


// ------------------------------------------------------------------------------------------------------------------


bool Application::eventFilter(QObject* watched, QEvent* event)
{
    // Track which window was activated most recently to prefer it on window choosing
    // (e.g. when another application opens a link)
    if (event->type() == QEvent::WindowActivate)
    {
        RekonqWindow *window = qobject_cast<RekonqWindow*>(watched);
        if (window)
        {
            if (!m_rekonqWindows.isEmpty() 
                && m_rekonqWindows.at(0) 
                && m_rekonqWindows.at(0).data() != window)
            {
                int index = m_rekonqWindows.indexOf(QWeakPointer<RekonqWindow>(window));
                Q_ASSERT(index != -1);
                m_rekonqWindows.prepend(m_rekonqWindows.takeAt(index));
            }
        }
    }

    // As we are filtering the events occurred to the tabwindows, check also
    // when we close one of them, remove from tab window list and check if it was last...
    if ((event->type() == QEvent::Close) && !rApp->sessionSaving())
    {
        RekonqWindow *window = qobject_cast<RekonqWindow*>(watched);

        if (window)
        {
            SessionManager::self()->saveSession();
            m_rekonqWindows.removeOne(window);
#ifdef HAVE_KACTIVITIES
            QString currentActivity = m_activityConsumer->currentActivity();
            m_activityRekonqWindowsMap[currentActivity].removeOne(window);
#endif
        }
        
        WebTab *webApp = qobject_cast<WebTab*>(watched);
        m_webApps.removeOne(webApp);

        if (m_rekonqWindows.count() == 0 && m_webApps.count() == 0)
            quit();
    }

    return QObject::eventFilter(watched, event);
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

    if (type == Rekonq::WebApp)
    {
        WebTab *tab = newWebApp();
        connect(tab->page(), SIGNAL(pageCreated(WebPage*)), this, SLOT(pageCreated(WebPage*)));
        tab->view()->load(url);
        return;
    }
    
    Rekonq::OpenType newType = type;
    // Don't open useless tabs or windows for actions in rekonq: pages
    if (url.url().contains("rekonq:") && url.url().contains("/"))
        newType = Rekonq::CurrentTab;

    RekonqWindow *w = 0;
    if (newType == Rekonq::NewPrivateWindow)
    {
        w = newWindow(true, true);
        newType = Rekonq::CurrentTab;
    }
    else if (newType == Rekonq::NewWindow
             || ((newType == Rekonq::NewTab || newType == Rekonq::NewFocusedTab) && !haveWindowsForActivity()))
    {
        w = newWindow();
        newType = Rekonq::CurrentTab;
    }
    else
    {
        w = rekonqWindow();
        if (!w)
        {
            w = newWindow();
            newType = Rekonq::CurrentTab;
        }
    }

    w->loadUrl(url, newType);
}


void Application::updateConfiguration()
{
    QWebSettings *defaultSettings = QWebSettings::globalSettings();

    // =========== Fonts ==============
    defaultSettings->setFontFamily(QWebSettings::StandardFont, ReKonfig::standardFontFamily());
    defaultSettings->setFontFamily(QWebSettings::FixedFont, ReKonfig::fixedFontFamily());
    defaultSettings->setFontFamily(QWebSettings::SerifFont, ReKonfig::serifFontFamily());
    defaultSettings->setFontFamily(QWebSettings::SansSerifFont, ReKonfig::sansSerifFontFamily());
    defaultSettings->setFontFamily(QWebSettings::CursiveFont, ReKonfig::cursiveFontFamily());
    defaultSettings->setFontFamily(QWebSettings::FantasyFont, ReKonfig::fantasyFontFamily());

    defaultSettings->setFontSize(QWebSettings::DefaultFontSize, ReKonfig::defaultFontSize());
    defaultSettings->setFontSize(QWebSettings::MinimumFontSize, ReKonfig::minFontSize());
    
    // encodings
    QString enc = ReKonfig::defaultEncoding();
    defaultSettings->setDefaultTextEncoding(enc);

    // ================ WebKit ============================
    defaultSettings->setAttribute(QWebSettings::DnsPrefetchEnabled, ReKonfig::dnsPrefetch());
    defaultSettings->setAttribute(QWebSettings::PrintElementBackgrounds, ReKonfig::printElementBackgrounds());
    defaultSettings->setAttribute(QWebSettings::ZoomTextOnly, ReKonfig::zoomTextOnly());

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
        QWebSettings::setOfflineStorageDefaultQuota(ReKonfig::offlineWebApplicationCacheQuota() * 1024);
    }

    // ================= WebGl ===================
    defaultSettings->setAttribute(QWebSettings::WebGLEnabled, ReKonfig::webGL());
    defaultSettings->setAttribute(QWebSettings::AcceleratedCompositingEnabled, ReKonfig::webGL());

    // Applies user defined CSS to all open webpages.
    defaultSettings->setUserStyleSheetUrl(ReKonfig::userCSS());

    // ====== load Settings on main classes
    HistoryManager::self()->loadSettings();

    defaultSettings = 0;
    
    if (!rekonqWindow())
        return;
    
    // ============== Tabs ==================
    bool b = ReKonfig::closeTabSelectPrevious();
    Q_FOREACH(const QWeakPointer<RekonqWindow> &w, m_rekonqWindows)
    {
        if (b)
            w.data()->tabBar()->setSelectionBehaviorOnRemove(QTabBar::SelectPreviousTab);
        else
            w.data()->tabBar()->setSelectionBehaviorOnRemove(QTabBar::SelectRightTab);
    }

    // FIXME What about this?
//     ReKonfig::useFavicon()
//     ? rekonqWindow()->changeWindowIcon(rekonqWindow()->mainView()->currentIndex())
//     : rekonqWindow()->setWindowIcon(KIcon("rekonq"))
//     ;
//
    // hovering unfocused tabs options
    switch (ReKonfig::hoveringTabOption())
    {
    case 0: // tab previews
    case 3: // nothing
        for (int i = 0; i < rekonqWindow()->tabBar()->count(); i++)
        {
            rekonqWindow()->tabBar()->setTabToolTip(i, QL1S(""));
        }
        break;

    case 1: // title previews
        for (int i = 0; i < rekonqWindow()->tabBar()->count(); i++)
        {
            rekonqWindow()->tabBar()->setTabToolTip(i, rekonqWindow()->tabWidget()->tabText(i).remove('&'));
        }
        break;

    case 2: // url previews
        for (int i = 0; i < rekonqWindow()->tabBar()->count(); i++)
        {
            rekonqWindow()->tabBar()->setTabToolTip(i, rekonqWindow()->tabWidget()->webWindow(i)->url().toMimeDataString());
        }
        break;

    default: // non extant case
        ASSERT_NOT_REACHED(unknown hoveringTabOption);
        break;
    }
}


void Application::queryQuit()
{
    if (m_webApps.count() > 0)
    {
        rekonqWindow()->close();
        return;
    }

    if (rekonqWindowList().count() > 1)
    {
        int answer = KMessageBox::questionYesNoCancel(
                         rekonqWindow(),
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
            rekonqWindow()->close();
            return;

        case KMessageBox::No:
            break;

        default:
            return;
        }
    }

    SessionManager::self()->setSessionManagementEnabled(false);    
    quit();
}


void Application::clearPrivateData()
{
    QPointer<KDialog> dialog = new KDialog(rekonqWindow());
    dialog->setCaption(i18nc("@title:window", "Clear Private Data"));
    dialog->setButtons(KDialog::Ok | KDialog::Cancel);

    dialog->button(KDialog::Ok)->setIcon(KIcon("edit-clear"));
    dialog->button(KDialog::Ok)->setText(i18n("Clear"));

    Ui::ClearDataWidget clearWidget;
    QWidget widget;
    clearWidget.setupUi(&widget);
    clearWidget.clearHistory->setChecked(ReKonfig::clearHistory());
    clearWidget.clearDownloads->setChecked(ReKonfig::clearDownloads());
    clearWidget.clearCookies->setChecked(ReKonfig::clearCookies());
    clearWidget.clearCachedPages->setChecked(ReKonfig::clearCachedPages());
    clearWidget.clearWebIcons->setChecked(ReKonfig::clearWebIcons());
    clearWidget.homePageThumbs->setChecked(ReKonfig::clearHomePageThumbs());

    dialog->setMainWidget(&widget);
    dialog->exec();

    if (dialog->result() == QDialog::Accepted)
    {
        //Save current state
        ReKonfig::setClearHistory(clearWidget.clearHistory->isChecked());
        ReKonfig::setClearDownloads(clearWidget.clearDownloads->isChecked());
        ReKonfig::setClearCookies(clearWidget.clearDownloads->isChecked());
        ReKonfig::setClearCachedPages(clearWidget.clearCachedPages->isChecked());
        ReKonfig::setClearWebIcons(clearWidget.clearWebIcons->isChecked());
        ReKonfig::setClearHomePageThumbs(clearWidget.homePageThumbs->isChecked());

        if (clearWidget.clearHistory->isChecked())
        {
            HistoryManager::self()->clear();
        }

        if (clearWidget.clearDownloads->isChecked())
        {
            DownloadManager::self()->clearDownloadsHistory();
        }

        if (clearWidget.clearCookies->isChecked())
        {
            QDBusInterface kcookiejar("org.kde.kded", "/modules/kcookiejar", "org.kde.KCookieServer");
            QDBusReply<void> reply = kcookiejar.call("deleteAllCookies");
        }

        if (clearWidget.clearCachedPages->isChecked())
        {
            KProcess::startDetached(KStandardDirs::findExe("kio_http_cache_cleaner"),
                                    QStringList(QL1S("--clear-all")));
        }

        if (clearWidget.clearWebIcons->isChecked())
        {
            IconManager::self()->clearIconCache();
        }

        if (clearWidget.homePageThumbs->isChecked())
        {
            QString path = KStandardDirs::locateLocal("cache", QString("thumbs/rekonq"), true);
            path.remove("rekonq");
            QDir cacheDir(path);
            QStringList fileList = cacheDir.entryList();
            Q_FOREACH(const QString & str, fileList)
            {
                QFile file(path + str);
                file.remove();
            }
        }
    }

    dialog->deleteLater();
}


void Application::createWebAppShortcut(const QString & urlString, const QString & titleString)
{
    KUrl u;
    if (urlString.isEmpty())
    {
        u = rekonqWindow()->currentWebWindow()->url();
    }
    else
    {
        u = KUrl(urlString);
    }
    QString h = u.host();

    QPointer<KDialog> dialog = new KDialog(rekonqWindow());
    dialog->setCaption(i18nc("@title:window", "Create Application Shortcut"));
    dialog->setButtons(KDialog::Ok | KDialog::Cancel);
    dialog->button(KDialog::Ok)->setText(i18n("Create"));
    dialog->setMinimumSize(400, 50);
    dialog->setWindowIcon(QIcon(IconManager::self()->iconForUrl(u).pixmap(16)));

    Ui::webAppCreation wAppWidget;
    QWidget widget;
    wAppWidget.setupUi(&widget);

    QString webAppTitle;
    if (titleString.isEmpty())
    {
        webAppTitle = rekonqWindow()->currentWebWindow()->title();
    }
    else
    {
        webAppTitle = titleString;
    }
    webAppTitle = webAppTitle.remove('&');
    
    wAppWidget.nameLineEdit->setText(webAppTitle);
    wAppWidget.kcfg_createDesktopAppShortcut->setChecked(ReKonfig::createDesktopAppShortcut());
    wAppWidget.kcfg_createMenuAppShortcut->setChecked(ReKonfig::createMenuAppShortcut());

    dialog->setMainWidget(&widget);
    dialog->exec();

    if (dialog->result() == QDialog::Accepted)
    {
        ReKonfig::setCreateDesktopAppShortcut(wAppWidget.kcfg_createDesktopAppShortcut->isChecked());
        ReKonfig::setCreateMenuAppShortcut(wAppWidget.kcfg_createMenuAppShortcut->isChecked());

        IconManager::self()->saveDesktopIconForUrl(u);
        QString iconPath = KStandardDirs::locateLocal("cache" , "favicons/" , true) + h + QL1S("_WEBAPPICON.png");

        if (!wAppWidget.nameLineEdit->text().isEmpty())
            webAppTitle = wAppWidget.nameLineEdit->text();

        QString webAppDescription;
        if (!wAppWidget.descriptionLineEdit->text().isEmpty())
            webAppDescription = wAppWidget.descriptionLineEdit->text();

        QString shortcutString = QL1S("#!/usr/bin/env xdg-open\n")
                                 + QL1S("[Desktop Entry]\n")
                                 + QL1S("Name=") + webAppTitle
                                 + QL1S("\n")
                                 + QL1S("GenericName=") + webAppDescription
                                 + QL1S("\n")
                                 + QL1S("Icon=") + iconPath + QL1S("\n")
                                 + QL1S("Exec=rekonq --webapp ") + u.url() + QL1S("\n")
                                 + QL1S("Type=Application\n")
                                 + QL1S("Categories=Application;Network\n")
                                 ;

        if (ReKonfig::createDesktopAppShortcut())
        {
            QString desktop = KGlobalSettings::desktopPath();
            QFile wAppFile(desktop + QL1C('/') + webAppTitle);

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
            QFile wAppFile(appMenuDir + QL1C('/') + webAppTitle + QL1S(".desktop"));

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


void Application::bookmarksToolbarToggled(bool b)
{
    emit toggleBookmarksToolbar(b);
}


void Application::newPrivateBrowsingWindow()
{
    // NOTE: what about a "rekonq:incognito" page?
    loadUrl(KUrl("rekonq:home"), Rekonq::NewPrivateWindow);
}


void Application::pageCreated(WebPage *pg)
{
    if (m_rekonqWindows.isEmpty())
    {
        newWindow(pg);
        return;
    }

    RekonqWindow *tw = rekonqWindow();
    tw->tabWidget()->newTab(pg);

    tw->activateWindow();
    tw->raise();
}
