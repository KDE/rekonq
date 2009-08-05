/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2008 Benjamin C. Meyer <ben@meyerhome.net>
* Copyright (C) 2008 Dirk Mueller <mueller@kde.org>
* Copyright (C) 2008 Urs Wolfer <uwolfer @ kde.org>
* Copyright (C) 2008 Michael Howell <mhowell123@gmail.com>
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
    WebPage *createWindow(WebWindowType type);
    virtual WebPage *newWindow(WebWindowType type);
    
    virtual bool acceptNavigationRequest(QWebFrame *frame, 
                                         const QNetworkRequest &request, 
                                         NavigationType type);
    
    QString chooseFile(QWebFrame *frame, 
                       const QString &suggestedFile);
    
    void javaScriptAlert(QWebFrame *frame, 
                         const QString &msg);
                         
    bool javaScriptConfirm(QWebFrame *frame, 
                           const QString &msg);
                           
    bool javaScriptPrompt(QWebFrame *frame, 
                          const QString &msg, 
                          const QString &defaultValue, QString *result);
    
    QObject *createPlugin(const QString &classId, 
                          const QUrl &url, 
                          const QStringList &paramNames, 
                          const QStringList &paramValues);
    

protected Q_SLOTS:    
    virtual void slotHandleUnsupportedContent(QNetworkReply *reply);
    virtual void slotDownloadRequested(const QNetworkRequest &request);
    
private:
    friend class WebView;
    
    void viewErrorPage(QNetworkReply *);

    // keyboard/mouse modifiers
    Qt::KeyboardModifiers m_keyboardModifiers;
    Qt::MouseButtons m_pressedButtons;
};

#endif
