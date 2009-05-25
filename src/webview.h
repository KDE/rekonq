/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2007-2008 Trolltech ASA. All rights reserved
* Copyright (C) 2008 Benjamin C. Meyer <ben@meyerhome.net>
* Copyright (C) 2008-2009 by Andrea Diamantini <adjam7 at gmail dot com>
* Copyright (C) 2009 by Paweł Prażak <pawelprazak at gmail dot com>
*
*
* This program is free software; you can redistribute it
* and/or modify it under the terms of the GNU General
* Public License as published by the Free Software Foundation;
* either version 2, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* ============================================================ */


#ifndef WEBVIEW_H
#define WEBVIEW_H

// KDE Includes
#include <KUrl>

#include <kdewebkit/kwebpage.h>
#include <kdewebkit/kwebview.h>

// Qt Includes
#include <QWebPage>

// Forward Declarations
class MainWindow;
class Application;

class KActionCollection;

class QWebFrame;
class QAuthenticator;
class QMouseEvent;
class QNetworkProxy;
class QNetworkReply;
class QSslError;
class WebPage;

// Qt Includes
#include <QWebView>


class WebView : public KWebView
{
    Q_OBJECT

public:
    WebView(QWidget *parent = 0);

    KUrl url() const;
    QString lastStatusBarText() const;
    int progress() const;

public Q_SLOTS:
    void load(const KUrl &url);

signals:
    // switching tabs
    void ctrlTabPressed();
    void shiftCtrlTabPressed();

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void contextMenuEvent(QContextMenuEvent *event);
    void wheelEvent(QWheelEvent *event);

    /**
    * Filters (SHIFT + ) CTRL + TAB events and emit (shift)ctrlTabPressed()
    * to make switch tab
    */
    void keyPressEvent(QKeyEvent *event);

    // to reimplement from KWebView
    virtual void setNewPage();

private slots:
    void setProgress(int progress);
    void loadFinished();
    void setStatusBarText(const QString &string);
    void downloadRequested(const QNetworkRequest &request);
    void openLinkInNewTab();

private:
    WebPage *m_page;

    int m_progress;
    QString m_statusBarText;
};

#endif
