/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2010-2011 by Matthieu Gicquel <matgic78 at gmail dot com>
* Copyright (C) 2010-2012 by Andrea Diamantini <adjam7 at gmail dot com>
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
#include "analyzerpanel.h"
#include "analyzerpanel.moc"

// Local Includes
#include "networkanalyzer.h"
#include "networkaccessmanager.h"
#include "webtab.h"
#include "webview.h"
#include "webpage.h"

// KDE Includes
#include "KAction"


NetworkAnalyzerPanel::NetworkAnalyzerPanel(const QString &title, QWidget *parent)
    : QDockWidget(title, parent)
    , _viewer(new NetworkAnalyzer(this))
{
    setObjectName("networkAnalyzerDock");
    setWidget(_viewer);
}


void NetworkAnalyzerPanel::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event);
    toggle(false);
}


MainWindow* NetworkAnalyzerPanel::mainWindow()
{
    return qobject_cast<MainWindow *>(parentWidget());
}


void NetworkAnalyzerPanel::toggle(bool enable)
{
    mainWindow()->actionByName("net_analyzer")->setChecked(enable);
    WebPage *page = mainWindow()->currentTab()->page();
    NetworkAccessManager *manager = qobject_cast<NetworkAccessManager *>(page->networkAccessManager());

    page->enableNetworkAnalyzer(enable);

    if (enable)
    {
        connect(page, SIGNAL(loadStarted()), _viewer, SLOT(clear()));
        connect(manager, SIGNAL(networkData(QNetworkAccessManager::Operation, QNetworkRequest, QNetworkReply*)),
                _viewer, SLOT(addRequest(QNetworkAccessManager::Operation, QNetworkRequest, QNetworkReply*)));
    }
    else
    {
        disconnect(page, SIGNAL(loadStarted()), _viewer, SLOT(clear()));
        disconnect(manager, SIGNAL(networkData(QNetworkAccessManager::Operation, QNetworkRequest, QNetworkReply*)),
                   _viewer, SLOT(addRequest(QNetworkAccessManager::Operation, QNetworkRequest, QNetworkReply*)));
    }

    setVisible(enable);
}


void NetworkAnalyzerPanel::changeCurrentPage()
{
    bool enable = mainWindow()->currentTab()->page()->hasNetworkAnalyzerEnabled();
    toggle(enable);
}
