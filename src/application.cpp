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

// Local Includes
#include "searchengine.h"
#include "tabwindow.h"
#include "webwindow.h"
#include "urlresolver.h"

// Local Manager(s) Includes
#include "historymanager.h"
#include "sessionmanager.h"

// // KDE Includes
#include <KCmdLineArgs>

#include <KWindowSystem>
#include <KWindowInfo>
#include <KStartupInfo>

#include <KMessageBox>
// #include <KIcon>
// #include <KStandardDirs>
// #include <KAction>

// #include <KGlobal>
// #include <KCharsets>
// #include <KPushButton>
// #include <KMimeType>
// 
// // Qt Includes
// #include <QVBoxLayout>
// #include <QDir>
// #include <QTimer>


Application::Application()
    : KUniqueApplication()
{
}


Application::~Application()
{
    // ok, we are closing well.
    // Don't recover on next load..
    ReKonfig::setRecoverOnCrash(0);
    saveConfiguration();

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
    bool hasToBeRecovered = (ReKonfig::recoverOnCrash() > 0);
    // note that hasToBeRecovered is always true if it is not the first load
    // !isFirstLoad -> hasToBeRecovered

    kDebug() << "is first load? " << isFirstLoad;
    kDebug() << "are there arguments? " << areThereArguments;
    kDebug() << "is rekonq crashed? " << hasToBeRecovered;

    if (!isSessionRestored())
    {
        if (areThereArguments)
        {
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

            if (isFirstLoad && (ReKonfig::startupBehaviour() == 2) && SessionManager::self()->restoreSessionFromScratch())
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

                if (!tabWindow()->isActiveWindow())
                    KWindowSystem::demandAttention(tabWindow()->winId(), true);
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
                if (hasToBeRecovered)
                {
                    loadUrl(KUrl("about:closedTabs"), Rekonq::NewWindow);
                }
                else
                {
                    switch (ReKonfig::startupBehaviour())
                    {
                    case 0: // open home page
                        newTabWindow()->newCleanTab();
                        break;
                    case 1: // open new tab page
                        loadUrl(KUrl("about:home"), Rekonq::NewWindow);
                        break;
                    case 2: // restore session
                        if (SessionManager::self()->restoreSessionFromScratch())
                        {
                            break;
                        }
                    default:
                        newTabWindow()->newCleanTab();
                        break;
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
    }   // !isSessionRestored()

    if (isFirstLoad)
    {
        if (hasToBeRecovered)
        {
            QTimer::singleShot(1000, tabWindow()->currentWebWindow(), SLOT(showMessageBar()));
        }
        else
        {
            SessionManager::self()->setSessionManagementEnabled(true);
        }

        if (ReKonfig::checkDefaultSearchEngine() && !hasToBeRecovered && SearchEngine::defaultEngine().isNull())
            QTimer::singleShot(2000, tabWindow()->currentWebWindow(), SLOT(showSearchEngineBar()));

        // updating rekonq configuration
        updateConfiguration();

        setWindowIcon(KIcon("rekonq"));

        // just create History Manager...
        HistoryManager::self();

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


TabWindow *Application::tabWindow()
{
    TabWindow *active = qobject_cast<TabWindow*>(QApplication::activeWindow());

    if (!active)
    {
        if (m_tabWindows.isEmpty())
            return 0;

        Q_FOREACH(const QWeakPointer<TabWindow> &pointer, m_tabWindows)
        {
            if (KWindowInfo(pointer.data()->effectiveWinId(), NET::WMDesktop, 0).isOnCurrentDesktop())
                return pointer.data();
        }
        return m_tabWindows.at(0).data();
    }
    return active;
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

    TabWindow *w = 0;
    if (newType == Rekonq::NewWindow
            || (newType == Rekonq::NewTab && ReKonfig::openLinksInNewWindow()))
    {
        w = newTabWindow();
        newType = Rekonq::CurrentTab;
    }
    else
    {
        w = tabWindow();
    }

    w->loadUrl(url, newType);
}


TabWindow *Application::newTabWindow()
{
    TabWindow *w = new TabWindow;
    // This is used to track which window was activated most recently
    w->installEventFilter(this);

    m_tabWindows.prepend(w);
    w->show();

    return w;
}


void Application::removeTabWindow(TabWindow *window)
{
    m_tabWindows.removeOne(window);
    kDebug() << "Removing Window from app window list...";

    // bye bye...
    if (m_tabWindows.count() == 0)
        quit();
}


TabWindowList Application::tabWindowList()
{
    return m_tabWindows;
}


bool Application::eventFilter(QObject* watched, QEvent* event)
{
    // Track which window was activated most recently to prefer it on window choosing
    // (e.g. when another application opens a link)
    if (event->type() == QEvent::WindowActivate)
    {
        TabWindow *window = qobject_cast<TabWindow*>(watched);
        if (window)
        {
            if (m_tabWindows.at(0).data() != window)
            {
                int index = m_tabWindows.indexOf(QWeakPointer<TabWindow>(window));
                Q_ASSERT(index != -1);
                m_tabWindows.prepend(m_tabWindows.takeAt(index));
            }
        }
    }

    return QObject::eventFilter(watched, event);
}


void Application::updateConfiguration()
{
    kDebug() << "Updating... NOTHING!!!";
//     // ============== Tabs ==================
//     bool b = ReKonfig::closeTabSelectPrevious();
//     Q_FOREACH(const QWeakPointer<TabWindow> &w, m_tabWindows)
//     {
//         MainView *mv = w.data()->mainView();
//         mv->updateTabBar();
// 
//         mv->tabBar()->setAnimatedTabHighlighting(ReKonfig::animatedTabHighlighting());
// 
//         if (b)
//             mv->tabBar()->setSelectionBehaviorOnRemove(QTabBar::SelectPreviousTab);
//         else
//             mv->tabBar()->setSelectionBehaviorOnRemove(QTabBar::SelectRightTab);
//     }
// 
//     QWebSettings *defaultSettings = QWebSettings::globalSettings();
// 
//     // =========== Fonts ==============
//     defaultSettings->setFontFamily(QWebSettings::StandardFont, ReKonfig::standardFontFamily());
//     defaultSettings->setFontFamily(QWebSettings::FixedFont, ReKonfig::fixedFontFamily());
//     defaultSettings->setFontFamily(QWebSettings::SerifFont, ReKonfig::serifFontFamily());
//     defaultSettings->setFontFamily(QWebSettings::SansSerifFont, ReKonfig::sansSerifFontFamily());
//     defaultSettings->setFontFamily(QWebSettings::CursiveFont, ReKonfig::cursiveFontFamily());
//     defaultSettings->setFontFamily(QWebSettings::FantasyFont, ReKonfig::fantasyFontFamily());
// 
//     // compute font size
//     // (I have to admit I know nothing about these DPI questions..: copied from kwebkitpart, as someone suggested)
//     // font size in pixels =  font size in inches × screen dpi
//     if (tabWindow() && tabWindow()->currentTab())
//     {
//         int logDpiY = tabWindow()->currentTab()->view()->logicalDpiY();
//         float toPix = (logDpiY < 96.0)
//                       ? 96.0 / 72.0
//                       : logDpiY / 72.0 ;
// 
//         int defaultFontSize = ReKonfig::defaultFontSize();
//         int minimumFontSize = ReKonfig::minFontSize();
// 
//         defaultSettings->setFontSize(QWebSettings::DefaultFontSize, qRound(defaultFontSize * toPix));
//         defaultSettings->setFontSize(QWebSettings::MinimumFontSize, qRound(minimumFontSize * toPix));
//     }
// 
//     // encodings
//     QString enc = ReKonfig::defaultEncoding();
//     defaultSettings->setDefaultTextEncoding(enc);
// 
//     // ================ WebKit ============================
//     defaultSettings->setAttribute(QWebSettings::DnsPrefetchEnabled, ReKonfig::dnsPrefetch());
//     defaultSettings->setAttribute(QWebSettings::PrintElementBackgrounds, ReKonfig::printElementBackgrounds());
// 
//     defaultSettings->setAttribute(QWebSettings::JavascriptEnabled, ReKonfig::javascriptEnabled());
//     defaultSettings->setAttribute(QWebSettings::JavascriptCanOpenWindows, ReKonfig::javascriptCanOpenWindows());
//     defaultSettings->setAttribute(QWebSettings::JavascriptCanAccessClipboard, ReKonfig::javascriptCanAccessClipboard());
// 
//     defaultSettings->setAttribute(QWebSettings::JavaEnabled, ReKonfig::javaEnabled());
// 
//     if (ReKonfig::pluginsEnabled() == 2)
//         defaultSettings->setAttribute(QWebSettings::PluginsEnabled, false);
//     else
//         defaultSettings->setAttribute(QWebSettings::PluginsEnabled, true);
// 
//     // Enabling WebKit "Page Cache" feature: http://webkit.org/blog/427/webkit-page-cache-i-the-basics/
//     defaultSettings->setMaximumPagesInCache(3);
// 
//     // ===== HTML 5 features WebKit support ======
//     defaultSettings->setAttribute(QWebSettings::OfflineStorageDatabaseEnabled, ReKonfig::offlineStorageDatabaseEnabled());
//     defaultSettings->setAttribute(QWebSettings::OfflineWebApplicationCacheEnabled, ReKonfig::offlineWebApplicationCacheEnabled());
//     defaultSettings->setAttribute(QWebSettings::LocalStorageEnabled, ReKonfig::localStorageEnabled());
//     if (ReKonfig::localStorageEnabled())
//     {
//         QString path = KStandardDirs::locateLocal("cache", QString("WebkitLocalStorage/rekonq"), true);
//         path.remove("rekonq");
//         QWebSettings::setOfflineStoragePath(path);
//         QWebSettings::setOfflineStorageDefaultQuota(50000);
//     }
// 
//     // ================= WebGl ===================
//     defaultSettings->setAttribute(QWebSettings::WebGLEnabled, ReKonfig::webGL());
//     defaultSettings->setAttribute(QWebSettings::AcceleratedCompositingEnabled, ReKonfig::webGL());
// 
//     // Applies user defined CSS to all open webpages.
//     defaultSettings->setUserStyleSheetUrl(ReKonfig::userCSS());
// 
    // ====== load Settings on main classes
    HistoryManager::self()->loadSettings();
// 
//     defaultSettings = 0;
// 
//     if (!tabWindow())
//         return;
// 
//     ReKonfig::useFavicon()
//     ? tabWindow()->changeWindowIcon(tabWindow()->mainView()->currentIndex())
//     : tabWindow()->setWindowIcon(KIcon("rekonq"))
//     ;
// 
//     // hovering unfocused tabs options
//     switch (ReKonfig::hoveringTabOption())
//     {
//     case 0: // tab previews
//     case 3: // nothing
//         for (int i = 0; i < tabWindow()->mainView()->tabBar()->count(); i++)
//         {
//             tabWindow()->mainView()->tabBar()->setTabToolTip(i, QL1S(""));
//         }
//         break;
// 
//     case 1: // title previews
//         for (int i = 0; i < tabWindow()->mainView()->tabBar()->count(); i++)
//         {
//             tabWindow()->mainView()->tabBar()->setTabToolTip(i, tabWindow()->mainView()->tabText(i).remove('&'));
//         }
//         break;
// 
//     case 2: // url previews
//         for (int i = 0; i < tabWindow()->mainView()->tabBar()->count(); i++)
//         {
//             tabWindow()->mainView()->tabBar()->setTabToolTip(i, tabWindow()->mainView()->webTab(i)->url().toMimeDataString());
//         }
//         break;
// 
//     default: // non extant case
//         ASSERT_NOT_REACHED(unknown hoveringTabOption);
//         break;
//     }

}


void Application::queryQuit()
{
    if (tabWindowList().count() > 1)
    {
        int answer = KMessageBox::questionYesNoCancel(
                         tabWindow(),
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
            tabWindow()->close();
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
