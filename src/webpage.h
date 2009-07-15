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


class WebPage : public QWebPage
{
    Q_OBJECT

public:
    explicit WebPage(QObject *parent = 0);

public slots:
    void manageNetworkErrors(QNetworkReply* reply);

protected:
    QWebPage *createWindow(QWebPage::WebWindowType type);
    virtual WebPage *newWindow(WebWindowType type);

//     QString chooseFile(QWebFrame *frame, const QString &suggestedFile);
    
    void javaScriptAlert(QWebFrame *frame, const QString &msg);
    bool javaScriptConfirm(QWebFrame *frame, const QString &msg);
    bool javaScriptPrompt(QWebFrame *frame, const QString &msg, const QString &defaultValue, QString *result);
    
    QObject *createPlugin(const QString &classId, const QUrl &url, const QStringList &paramNames, const QStringList &paramValues);
        
protected Q_SLOTS:
    virtual void slotHandleUnsupportedContent(QNetworkReply *reply);
    virtual void slotDownloadRequested(const QNetworkRequest &request);
    
private:
    void viewErrorPage(QNetworkReply *);

    KUrl m_loadingUrl;
};

#endif
