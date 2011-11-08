/*
 * This file is part of the KDE project.
 * Copyright (C) 2011 by Andrea Diamantini <adjam7@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) version 3, or any
 * later version accepted by the membership of KDE e.V. (or its
 * successor approved by the membership of KDE e.V.), which shall
 * act as a proxy defined in Section 6 of version 3 of the license.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */


// Self Includes
#include "webpage.h"
#include "webpage.moc"

// KDE Includes
#include <KRun>

// Qt Includes
#include <QNetworkRequest>


WebPage::WebPage(QObject *parent)
    : KWebPage(parent)
    , _selfLoading(false)
{
    connect(this, SIGNAL(loadFinished(bool)), this, SLOT(disableSelfLoading()));
}


bool WebPage::acceptNavigationRequest(QWebFrame *frame, const QNetworkRequest &request, NavigationType type)
{
    if (_selfLoading)
    {
        return KWebPage::acceptNavigationRequest(frame, request, type);
    }
    
    (void)new KRun(request.url(), view(), 0);
    return false;
}


void WebPage::setSelfLoadingEnabled(bool b)
{
    _selfLoading = b;
}


void WebPage::disableSelfLoading()
{
    _selfLoading = false;
}
