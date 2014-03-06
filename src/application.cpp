/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2012-2014 by Andrea Diamantini <adjam7 at gmail dot com>
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
#include <KMessageBox>
#include <KProcess>
#include <KStartupInfo>
#include <KWindowInfo>
#include <KWindowSystem>

// Qt Includes
#include <QAction>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDialog>
#include <QDir>
#include <QIcon>
#include <QStandardPaths>
#include <QTimer>


Application::Application(int &argc, char **argv)
    : QApplication(argc, argv)
{
    // NOTE
    // This is needed to ensure rekonq directory path has been created...
    QString rekonqDataDir = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
    QDir d;
    d.mkpath(rekonqDataDir);
    // ------------------------------------------------------------------

    // updating rekonq configuration
    updateConfiguration();

    setWindowIcon(QIcon::fromTheme(QL1S("rekonq")));

    newInstance();
}


Application::~Application()
{
    // ok, we are closing well: don't recover on next load..
    ReKonfig::setRecoverOnCrash(0);
    saveConfiguration();

    // Destroy all windows...
    Q_FOREACH(QPointer<RekonqWindow> pointer, m_rekonqWindows)
    {
        delete pointer.data();
        pointer.clear();
    }

    // Destroy all web apps
    Q_FOREACH(WebTab *tab, m_webApps)
    {
        delete tab;
    }

    qDebug() << "Bye bye (k)baby...";
}


int Application::newInstance()
{
    QStringList args = arguments();

    // not that easy, indeed
    // We have to consider 3 variables here:
    // 1) Is first load?
    // 2) Are there arguments?
    // 3) Is rekonq recovering from crash?
    // so, we have 8 possible cases...
    static bool isFirstLoad = true;
    bool areThereArguments = (args.count() > 1);
    bool hasToBeRecoveredFromCrash = (ReKonfig::recoverOnCrash() > 0);
    // note that hasToBeRecoveredFromCrash is always true if it is not the first load
    // !isFirstLoad -> hasToBeRecoveredFromCrash

    qDebug() << "is first load? " << isFirstLoad;
    qDebug() << "are there arguments? " << areThereArguments;
    qDebug() << "is rekonq crashed? " << hasToBeRecoveredFromCrash;

    bool incognito = false; // FIXMEargs->isSet("incognito");
    bool webapp = false; // FIXME args->isSet("webapp");

    if (webapp)
    {
        qDebug() << "WEBAPP MODE...";
        if (!areThereArguments)
        {
            KMessageBox::error(0, i18n("Error"), i18n("Cannot launch webapp mode without an URL to load"));
            return 1;
        }

        if (args.count() > 1)
        {
            KMessageBox::error(0, i18n("Error"), i18n("Cannot load more than one URL in webapp mode"));
            return 1;
        }

        QUrl u = QUrl(args.at(0));
        if (!u.isLocalFile() || !QFile::exists(u.toLocalFile()))
        {
            u = UrlResolver::urlFromTextTyped(args.at(0));
        }
        qDebug() << "URL: " << u;

        loadUrl(u, Rekonq::WebApp);

        KStartupInfo::appStarted();
        isFirstLoad = false;
        return 0;
    }

    if (areThereArguments)
    {
        qDebug() << "DEFAULT MODE, WITH ARGUMENTS...";

        // prepare URLS to load
        QList<QUrl> urlList;
        for (int i = 1; i < args.count(); ++i)
        {
            const QUrl u = QUrl(args.at(i));

            if (u.isLocalFile() && QFile::exists(u.toLocalFile())) // "rekonq somefile.html" case
            {
                urlList << u;
            }
            else
            {
                // "rekonq kde.org" || "rekonq kde:kdialog" cases
                urlList << UrlResolver::urlFromTextTyped(args.at(i));
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
        qDebug() << "DEFAULT MODE, NO ARGUMENTS...";
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
                        loadUrl(QUrl(ReKonfig::homePage()), Rekonq::NewPrivateWindow);
                        break;
                    }
                    if (SessionManager::self()->restoreJustThePinnedTabs())
                        loadUrl(QUrl(ReKonfig::homePage()) , Rekonq::NewTab);
                    else
                        loadUrl(QUrl(ReKonfig::homePage()) , Rekonq::NewWindow);
                    break;
                case 1: // open new tab page
                    if (incognito)
                    {
                        loadUrl(QUrl(QL1S("rekonq:home")), Rekonq::NewPrivateWindow);
                        break;
                    }
                    if (SessionManager::self()->restoreJustThePinnedTabs())
                        loadUrl(QUrl(QL1S("rekonq:home")) , Rekonq::NewTab);
                    else
                        loadUrl(QUrl(QL1S("rekonq:home")), Rekonq::NewWindow);
                    break;
                case 2: // restore session
                    if (incognito)
                    {
                        loadUrl(QUrl(QL1S("rekonq:home")), Rekonq::NewPrivateWindow);
                        break;
                    }
                    if (hasToBeRecoveredFromCrash || !SessionManager::self()->restoreSessionFromScratch())
                    {
                        loadUrl(QUrl(QL1S("rekonq:home")) , Rekonq::NewTab);
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
                loadUrl(QUrl(QL1S("rekonq:home")) , type);
                break;
            case 2: // homepage
                loadUrl(QUrl(ReKonfig::homePage()) , type);
                break;
            case 1: // blank page
            default:
                loadUrl(QUrl(QL1S("about:blank")) , type);
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

        if (ReKonfig::checkDefaultSearchEngine() && !hasToBeRecoveredFromCrash && !SearchEngine::defaultEngine())
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


RekonqWindow *Application::rekonqWindow()
{
    RekonqWindow *active = qobject_cast<RekonqWindow*>(QApplication::activeWindow());

    if (active)
        return active;

    RekonqWindowList wList = m_rekonqWindows;

    if (wList.isEmpty())
        return 0;

    Q_FOREACH(const QPointer<RekonqWindow> &pointer, wList)
    {
        if (KWindowInfo(pointer.data()->effectiveWinId(), NET::WMDesktop, 0).isOnCurrentDesktop())
            return pointer.data();
    }
    return wList.at(0).data();
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
                int index = m_rekonqWindows.indexOf(QPointer<RekonqWindow>(window));
                Q_ASSERT(index != -1);
                m_rekonqWindows.prepend(m_rekonqWindows.takeAt(index));
            }
        }
    }

    // As we are filtering the events occurred to the tabwindows, check also
    // when we close one of them, remove from tab window list and check if it was last...
    if ((event->type() == QEvent::Close) /* FIXME && !rApp->sessionSaving() */)
    {
        RekonqWindow *window = qobject_cast<RekonqWindow*>(watched);

        if (window)
        {
            SessionManager::self()->saveSession();
            m_rekonqWindows.removeOne(window);
        }
        
        WebTab *webApp = qobject_cast<WebTab*>(watched);
        m_webApps.removeOne(webApp);

        if (m_rekonqWindows.count() == 0 && m_webApps.count() == 0)
            quit();
    }

    return QObject::eventFilter(watched, event);
}


void Application::loadUrl(const QUrl& url, const Rekonq::OpenType& type)
{
    if (url.isEmpty())
        return;

    if (!url.isValid())
    {
        KMessageBox::error(0, i18n("Malformed URL:\n%1", url.url(QUrl::StripTrailingSlash)));
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
    if (url.url().contains(QL1S("rekonq:")) && url.url().contains(QL1C('/')))
        newType = Rekonq::CurrentTab;

    RekonqWindow *w = 0;
    if (newType == Rekonq::NewPrivateWindow)
    {
        w = newWindow(true, true);
        newType = Rekonq::CurrentTab;
    }
    else if (newType == Rekonq::NewWindow || (newType == Rekonq::NewTab || newType == Rekonq::NewFocusedTab))
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
        QString path = QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + QL1S("/WebkitLocalStorage/");
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
    Q_FOREACH(const QPointer<RekonqWindow> &w, m_rekonqWindows)
    {
        if (b)
            w.data()->tabBar()->setSelectionBehaviorOnRemove(QTabBar::SelectPreviousTab);
        else
            w.data()->tabBar()->setSelectionBehaviorOnRemove(QTabBar::SelectRightTab);
    }

    // FIXME What about this?
//     ReKonfig::useFavicon()
//     ? rekonqWindow()->changeWindowIcon(rekonqWindow()->mainView()->currentIndex())
//     : rekonqWindow()->setWindowIcon(QIcon::fromTheme("rekonq"))
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
            rekonqWindow()->tabBar()->setTabToolTip(i, rekonqWindow()->tabWidget()->tabText(i).remove(QL1C('&')));
        }
        break;

    case 2: // url previews
        for (int i = 0; i < rekonqWindow()->tabBar()->count(); i++)
        {
            rekonqWindow()->tabBar()->setTabToolTip(i, rekonqWindow()->tabWidget()->webWindow(i)->url().url());
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
                                  QIcon::fromTheme( QL1S("window-close") )),
                         KStandardGuiItem::quit(),
                         KStandardGuiItem::cancel(),
                         QL1S("confirmClosingMultipleWindows")
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
    QPointer<QDialog> dialog = new QDialog(rekonqWindow());
    // FIXME
//     dialog->setCaption(i18nc("@title:window", "Clear Private Data"));
//     dialog->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
// 
//     dialog->button(QDialogButtonBox::Ok)->setIcon(QIcon::fromTheme(QL1S("edit-clear")));
//     dialog->button(QDialogButtonBox::Ok)->setText(i18n("Clear"));
// 
    Ui::ClearDataWidget clearWidget;
    QWidget widget;
    clearWidget.setupUi(&widget);
    clearWidget.clearHistory->setChecked(ReKonfig::clearHistory());
    clearWidget.clearDownloads->setChecked(ReKonfig::clearDownloads());
    clearWidget.clearCookies->setChecked(ReKonfig::clearCookies());
    clearWidget.clearCachedPages->setChecked(ReKonfig::clearCachedPages());
    clearWidget.clearWebIcons->setChecked(ReKonfig::clearWebIcons());
    clearWidget.homePageThumbs->setChecked(ReKonfig::clearHomePageThumbs());
// 
//     dialog->setMainWidget(&widget);
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
            QDBusInterface kcookiejar(QL1S("org.kde.kded"), QL1S("/modules/kcookiejar"), QL1S("org.kde.KCookieServer"));
            QDBusReply<void> reply = kcookiejar.call(QL1S("deleteAllCookies"));
        }

        if (clearWidget.clearCachedPages->isChecked())
        {
            KProcess::startDetached(QStandardPaths::findExecutable(QL1S("kio_http_cache_cleaner")),
                                     QStringList(QL1S("--clear-all")));
        }

        if (clearWidget.clearWebIcons->isChecked())
        {
            IconManager::self()->clearIconCache();
        }

        if (clearWidget.homePageThumbs->isChecked())
        {
            QString path = QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + QL1S("/thumbs/");
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
    QUrl u;
    if (urlString.isEmpty())
    {
        u = rekonqWindow()->currentWebWindow()->url();
    }
    else
    {
        u = QUrl(urlString);
    }
    QString h = u.host();

    QPointer<QDialog> dialog = new QDialog(rekonqWindow());
    // FIXME
//     dialog->setCaption(i18nc("@title:window", "Create Application Shortcut"));
//     dialog->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
//     dialog->button(QDialogButtonBox::Ok)->setText(i18n("Create"));
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
    webAppTitle = webAppTitle.remove(QL1C('&'));
    
    wAppWidget.nameLineEdit->setText(webAppTitle);
    wAppWidget.kcfg_createDesktopAppShortcut->setChecked(ReKonfig::createDesktopAppShortcut());
    wAppWidget.kcfg_createMenuAppShortcut->setChecked(ReKonfig::createMenuAppShortcut());

//     dialog->setMainWidget(&widget);
    dialog->exec();

    if (dialog->result() == QDialog::Accepted)
    {
        ReKonfig::setCreateDesktopAppShortcut(wAppWidget.kcfg_createDesktopAppShortcut->isChecked());
        ReKonfig::setCreateMenuAppShortcut(wAppWidget.kcfg_createMenuAppShortcut->isChecked());

        IconManager::self()->saveDesktopIconForUrl(u);
        QString iconPath = QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + QL1S("/favicons/") + h + QL1S("_WEBAPPICON.png");

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
            QString desktop = QStandardPaths::displayName(QStandardPaths::DesktopLocation);
            QFile wAppFile(desktop + QL1C('/') + webAppTitle);

            if (!wAppFile.open(QIODevice::WriteOnly | QIODevice::Text))
            {
                qDebug() << "Unable to open file: " << wAppFile.errorString();
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
            QString appMenuDir = QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation);
            QFile wAppFile(appMenuDir + QL1C('/') + webAppTitle + QL1S(".desktop"));

            if (!wAppFile.open(QIODevice::WriteOnly | QIODevice::Text))
            {
                qDebug() << "Unable to open file: " << wAppFile.errorString();
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
    loadUrl(QUrl(QL1S("rekonq:home")), Rekonq::NewPrivateWindow);
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
