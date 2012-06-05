/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2011-2012 by Andrea Diamantini <adjam7 at gmail dot com>
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
#include "webpage.h"
#include "webpage.moc"

// KDE Includes
#include <KRun>

// Qt Includes
#include <QNetworkRequest>


WebPage::WebPage(QObject *parent)
    : KWebPage(parent)
    , _selfLoading(true)
{
    connect(this, SIGNAL(loadFinished(bool)), this, SLOT(disableSelfLoading()));

    connect(this, SIGNAL(unsupportedContent(QNetworkReply*)), this, SLOT(downloadResponse(QNetworkReply*)));
    connect(this, SIGNAL(downloadRequested(QNetworkRequest)), this, SLOT(downloadRequest(QNetworkRequest)));
}


bool WebPage::acceptNavigationRequest(QWebFrame *frame, const QNetworkRequest &request, NavigationType type)
{
    return KWebPage::acceptNavigationRequest(frame, request, type);

    // FIXME
//     (void)new KRun(request.url(), view(), 0);
//     return false;
}


void WebPage::setSelfLoadingEnabled(bool b)
{
    _selfLoading = b;
}


void WebPage::disableSelfLoading()
{
    _selfLoading = false;
}
