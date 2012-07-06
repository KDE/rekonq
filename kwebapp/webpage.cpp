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

// Local Includes
#include "urlresolver.h"

// KDE Includes
#include <KRun>
#include <KToolInvocation>
#include <KProtocolInfo>
#include <KDebug>

// Qt Includes
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QWebFrame>

// Defines
#define QL1S(x) QLatin1String(x)


WebPage::WebPage(QObject *parent)
    : KWebPage(parent)
{
    // ----- handling unsupported content...
    setForwardUnsupportedContent(true);
    connect(this, SIGNAL(unsupportedContent(QNetworkReply*)), this, SLOT(handleUnsupportedContent(QNetworkReply*)));

    // downloads
    connect(this, SIGNAL(unsupportedContent(QNetworkReply*)), this, SLOT(downloadResponse(QNetworkReply*)));
    connect(this, SIGNAL(downloadRequested(QNetworkRequest)), this, SLOT(downloadRequest(QNetworkRequest)));
}


bool WebPage::acceptNavigationRequest(QWebFrame *frame, const QNetworkRequest &request, NavigationType type)
{
    QUrl reqUrl = request.url();
    QString protocol = reqUrl.scheme();

    // javascript handling
    if (protocol == QL1S("javascript"))
    {
        QString scriptSource = QUrl::fromPercentEncoding(reqUrl.toString().mid(11).toUtf8());
        mainFrame()->evaluateJavaScript(scriptSource);
        return false;
    }

    // "mailto" handling: It needs to be handled both here (mail url launched)
    // and in handleUnsupportedContent (mail links clicked)
    if (protocol == QL1S("mailto"))
    {
        KToolInvocation::invokeMailer(reqUrl);
        return false;
    }

    if (frame && UrlResolver::isKDEUrl(reqUrl.toString()))
    {
        QUrl newReqUrl = UrlResolver::urlFromTextTyped(reqUrl.toString());
        frame->load(newReqUrl);
        return false;
    }

    // don't let webkit try to load an unknown (or missing) protocol...
    if (!KProtocolInfo::isKnownProtocol(protocol))
    {
        kDebug() << "UNKNOWN PROTOCOL: " << protocol;
        return false;
    }

    return KWebPage::acceptNavigationRequest(frame, request, type);
}


void WebPage::handleUnsupportedContent(QNetworkReply *reply)
{
    Q_ASSERT(reply);

    if (!reply)
    {
        kDebug() << "NO REPLY. Why????";
        return;
    }

    QUrl replyUrl = reply->url();
    QString protocol = replyUrl.scheme();

    // "http(s)" (fast) handling
    if (protocol == QL1S("http") || protocol == QL1S("https"))
    {
        kDebug() << "Error: " << protocol;
        return;
    }

    // "mailto" handling.
    if (protocol == QL1S("mailto"))
    {
        KToolInvocation::invokeMailer(replyUrl);
        return;
    }

    return;
}
