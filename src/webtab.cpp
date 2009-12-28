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

// KDE Includes
#include <KService>
#include <KUriFilterData>
#include <KStandardShortcut>
#include <KMenu>
#include <KActionMenu>
#include <KWebView>
#include <kwebwallet.h>

// Qt Includes
#include <QContextMenuEvent>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QClipboard>
#include <QKeyEvent>
#include <QAction>
#include <QVBoxLayout>


WebTab::WebTab(QWidget* parent)
    : QWidget(parent)
    , m_view( new WebView(this) )
    , m_progress(0)
{
    QVBoxLayout* l = new QVBoxLayout(this);
    l->setMargin(0);
    l->setSpacing(0);

    QWidget* messageBar = new QWidget(this);
    l->addWidget(messageBar);
    messageBar->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Minimum);

    QVBoxLayout* l2 = new QVBoxLayout(messageBar);
    l2->setMargin(0);
    l2->setSpacing(0);

    l->addWidget(m_view);
    m_view->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
   
    KWebWallet *wallet = page()->wallet();
   
    if(wallet)
    {
        connect(wallet, SIGNAL(saveFormDataRequested(const QString &, const QUrl &)),
                this, SLOT(createWalletBar(const QString &, const QUrl &)));
    }

    connect(page(), SIGNAL(statusBarMessage(const QString&)), this, SLOT(setStatusBarText(const QString&)));

    connect(m_view, SIGNAL(loadProgress(int)), this, SLOT(updateProgress(int)));
    connect(m_view, SIGNAL(loadFinished(bool)), this, SLOT(loadFinished(bool)));

    connect(m_view, SIGNAL(linkMiddleOrCtrlClicked(const KUrl &)), this, SLOT(loadInNewTab(const KUrl &)) );
}


WebTab::~WebTab()
{
    delete m_view;
}


WebView *WebTab::view()
{
    return m_view;
}


WebPage *WebTab::page()
{
    return m_view->page();
}


KUrl WebTab::url() const 
{ 
    return KUrl(m_view->url()); 
}


int WebTab::progress()
{
    return m_progress;
}


QString WebTab::lastStatusBarText() const
{ 
    return m_statusBarText; 
}


void WebTab::setStatusBarText(const QString &string) 
{ 
    m_statusBarText = string; 
}


void WebTab::updateProgress(int p)
{
    m_progress = p;
}


void WebTab::loadFinished(bool)
{
    m_progress = 0;
}


void WebTab::loadInNewTab(const KUrl &url)
{
    Application::instance()->loadUrl(url, Rekonq::SettingOpenTab);
}


void WebTab::createWalletBar(const QString &key, const QUrl &url)
{    
    KWebWallet *wallet = page()->wallet();
    QWidget *messageBar = layout()->itemAt(0)->widget();

    WalletBar *walletBar = new WalletBar(messageBar);
    walletBar->onSaveFormData(key,url);
    messageBar->layout()->addWidget(walletBar);

    connect(walletBar, SIGNAL(saveFormDataAccepted(const QString &)),
            wallet, SLOT(acceptSaveFormDataRequest(const QString &)));
    connect(walletBar, SIGNAL(saveFormDataRejected(const QString &)),
            wallet, SLOT(rejectSaveFormDataRequest(const QString &)));
}
