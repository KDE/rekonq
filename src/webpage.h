/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2007-2008 Trolltech ASA. All rights reserved
* Copyright (C) 2008-2009 by Andrea Diamantini <adjam7 at gmail dot com>
*
*
* This program is free software; you can redistribute it
* and/or modify it under the terms of the GNU General
* Public License as published by the Free Software Foundation;
* either version 3, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* ============================================================ */


#ifndef WEBPAGE_H
#define WEBPAGE_H


// KDE Includes
#include <KUrl>

#include <kdewebkit/kwebpage.h>
#include <kdewebkit/kwebview.h>

// Qt Includes
#include <QWebPage>

// Forward Declarations
class QWebFrame;
class QNetworkReply;


class WebPage : public KWebPage
{
    Q_OBJECT

public:
    explicit WebPage(QObject *parent = 0);

public slots:
    void manageNetworkErrors(QNetworkReply* reply);

protected:
    bool acceptNavigationRequest(QWebFrame *frame,
                                 const QNetworkRequest &request,
                                 NavigationType type);

    KWebPage *createWindow(QWebPage::WebWindowType type);


protected Q_SLOTS:
    virtual void slotHandleUnsupportedContent(QNetworkReply *reply);

private:
    void viewErrorPage(QNetworkReply *);

    KUrl m_loadingUrl;
};

#endif
