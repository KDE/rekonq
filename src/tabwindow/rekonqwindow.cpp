/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2013 by Andrea Diamantini <adjam7 at gmail dot com>
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
#include "rekonqwindow.h"
#include "rekonqwindow.moc"

// Auto Includes
#include "rekonq.h"

// Local Includes
#include "application.h"

#include "tabwidget.h"
#include "tabbar.h"
#include "rekonqfactory.h"

#include "webpage.h"
#include "webwindow.h"

// KDE Includes
#include <KUrl>
#include <KLocalizedString>

// Qt Includes
#include <QVBoxLayout>
#include <QSizePolicy>
#include <QDBusConnection>


RekonqWindow::RekonqWindow(bool withTab, bool privateBrowsingMode, QWidget *parent)
    : RWindow(parent)
    , _tabWidget(new TabWidget(withTab, privateBrowsingMode, this))
    , _splitter(new QSplitter(this))
{
    init();
}


RekonqWindow::RekonqWindow(WebPage *pg, QWidget *parent)
    : RWindow(parent)
    , _tabWidget(new TabWidget(pg, this))
    , _splitter(new QSplitter(this))
{
    init();
}


RekonqWindow::~RekonqWindow()
{
}


void RekonqWindow::init()
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QVBoxLayout *l = new QVBoxLayout(this);
    l->setMargin(0);
    l->setSpacing(0);

    if (ReKonfig::showBookmarksPanel())
        showBookmarksPanel(true);
    
    if (ReKonfig::showHistoryPanel())
        showHistoryPanel(true);

    _splitter->addWidget(_tabWidget);
    _tabWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    l->addWidget(_splitter);

    // fix focus handling
    setFocusProxy(_tabWidget);
    
    // signals
    connect(_tabWidget, SIGNAL(closeWindow()), this, SLOT(close()));
    connect(_tabWidget, SIGNAL(windowTitleChanged(QString)), this, SLOT(setWindowTitle(QString)));
    connect(_tabWidget, SIGNAL(actionsReady()), this, SLOT(registerWindow()));
}


void RekonqWindow::registerWindow()
{
    // This is needed to properly support appmenu-qt feature
    RekonqFactory::createWidget(QL1S("menuBar"), this);
    QDBusConnection::sessionBus().registerObject(QL1S("rekonq"), this);
}


// --------------------------------------------------------------------------------------------------


TabWidget *RekonqWindow::tabWidget()
{
    return _tabWidget;
}


TabBar *RekonqWindow::tabBar()
{
    return _tabWidget->tabBar();
}


WebWindow *RekonqWindow::currentWebWindow() const
{
    return _tabWidget->currentWebWindow();
}


bool RekonqWindow::isPrivateBrowsingMode()
{
    return _tabWidget->isPrivateBrowsingWindowMode();
}


// --------------------------------------------------------------------------------------------------


void RekonqWindow::loadUrl(const KUrl &url, Rekonq::OpenType type, TabHistory *history)
{
    switch (type)
    {
    case Rekonq::NewWindow:
    case Rekonq::NewPrivateWindow:
    case Rekonq::WebApp:
        rApp->loadUrl(url, type);
        return;

    case Rekonq::NewTab:
    case Rekonq::NewBackGroundTab:
    case Rekonq::NewFocusedTab:
    case Rekonq::CurrentTab:
    default:
        _tabWidget->loadUrl(url, type, history);
        break;
    };
}


void RekonqWindow::showBookmarksPanel(bool on)
{
    if (on)
    {
        if (_bookmarksPanel.isNull())
        {
            _bookmarksPanel = new BookmarksPanel(i18n("Bookmarks Panel"), this);
            connect(_bookmarksPanel.data(), SIGNAL(openUrl(KUrl, Rekonq::OpenType)), this, SLOT(loadUrl(KUrl, Rekonq::OpenType)));

            QAction *a = _tabWidget->actionByName(QL1S("show_bookmarks_panel"));
            connect(_bookmarksPanel.data(), SIGNAL(visibilityChanged(bool)), a, SLOT(setChecked(bool)));
        }
        _splitter->insertWidget(0, _bookmarksPanel.data());
        _bookmarksPanel.data()->show();
    }
    else
    {
        _bookmarksPanel.data()->hide();
        delete _bookmarksPanel.data();
        _bookmarksPanel.clear();
    }
}


void RekonqWindow::showHistoryPanel(bool on)
{
    if (on)
    {
        if (_historyPanel.isNull())
        {
            _historyPanel = new HistoryPanel(i18n("History Panel"), this);
            connect(_historyPanel.data(), SIGNAL(openUrl(KUrl, Rekonq::OpenType)), this, SLOT(loadUrl(KUrl, Rekonq::OpenType)));
            
            QAction *a = _tabWidget->actionByName(QL1S("show_history_panel"));
            connect(_historyPanel.data(), SIGNAL(visibilityChanged(bool)), a, SLOT(setChecked(bool)));

        }
        _splitter->insertWidget(0, _historyPanel.data());
        _historyPanel.data()->show();
    }
    else
    {
        _historyPanel.data()->hide();
        delete _historyPanel.data();
        _historyPanel.clear();
    }
}
