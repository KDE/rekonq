/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2008 Benjamin C. Meyer <ben@meyerhome.net>
* Copyright (C) 2008 Dirk Mueller <mueller@kde.org>
* Copyright (C) 2008 Urs Wolfer <uwolfer @ kde.org>
* Copyright (C) 2008 Michael Howell <mhowell123@gmail.com>
* Copyright (C) 2008-2012 by Andrea Diamantini <adjam7 at gmail dot com>
* Copyright (C) 2010 by Matthieu Gicquel <matgic78 at gmail dot com>
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


#ifndef WEBPAGE_H
#define WEBPAGE_H


// Rekonq Includes
#include "rekonq_defines.h"

// Local Includes
#include "protocolhandler.h"
#include "websslinfo.h"

// KDE Includes
#include <KWebPage>


class REKONQ_TESTS_EXPORT WebPage : public KWebPage
{
    Q_OBJECT

public:
    explicit WebPage(QWidget *parent = 0);
    ~WebPage();

    bool hasNetworkAnalyzerEnabled() const;
    void enableNetworkAnalyzer(bool b);

    bool isOnRekonqPage() const;
    void setIsOnRekonqPage(bool b);

    KUrl loadingUrl();

    QString suggestedFileName();

    inline bool hasAdBlockedElements() const
    {
        return _hasAdBlockedElements;
    };

    inline void setHasAdBlockedElements(bool b)
    {
        _hasAdBlockedElements = b;
    };

    bool hasSslValid() const;

public Q_SLOTS:
    void downloadAllContentsWithKGet();

    virtual void downloadRequest(const QNetworkRequest &request);
    virtual void downloadUrl(const KUrl &url);

protected:
    WebPage *createWindow(WebWindowType type);

    virtual bool acceptNavigationRequest(QWebFrame *frame,
                                         const QNetworkRequest &request,
                                         NavigationType type);

private Q_SLOTS:
    void handleUnsupportedContent(QNetworkReply *reply);
    void manageNetworkErrors(QNetworkReply *reply);
    void loadStarted();
    void loadFinished(bool);
    void showSSLInfo(QPoint);

    void copyToTempFileResult(KJob*);

private:
    QString errorPage(QNetworkReply *reply);
    KUrl _loadingUrl;

    ProtocolHandler _protHandler;
    WebSslInfo _sslInfo;

    QString _mimeType;
    QString _suggestedFileName;

    bool _networkAnalyzer;
    bool _isOnRekonqPage;
    bool _hasAdBlockedElements;
};

#endif
