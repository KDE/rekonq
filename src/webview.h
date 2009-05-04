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

// Qt Includes
#include <QWebPage>

// Forward Declarations

class KActionCollection;

class QWebFrame;
class QMouseEvent;
class QNetworkReply;


class WebPage : public QWebPage
{
    Q_OBJECT

signals:
    void loadingUrl(const QUrl &url);   // WARNING has to be QUrl!!

public:
    WebPage(QObject *parent = 0);
    ~WebPage();

protected:
    bool acceptNavigationRequest(QWebFrame *frame,
                                 const QNetworkRequest &request,
                                 NavigationType type);

    QWebPage *createWindow(QWebPage::WebWindowType type);
    QObject *createPlugin(const QString &classId,
                          const QUrl &url,
                          const QStringList &paramNames,
                          const QStringList &paramValues);

private slots:
    void handleUnsupportedContent(QNetworkReply *reply);

private:
    friend class WebView;

    // set the webview mousepressedevent
    Qt::KeyboardModifiers m_keyboardModifiers;
    Qt::MouseButtons m_pressedButtons;
    bool m_openInNewTab;
    KUrl m_loadingUrl;
};


// ----------------------------------------------------------------------------------------------------

// Qt Includes
#include <QWebView>


class WebView : public QWebView
{
    Q_OBJECT

public:
    WebView(QWidget *parent = 0);

    KActionCollection* webActions();

    // inline
    WebPage *webPage() const
    {
        return m_page;
    }
    KUrl url() const
    {
        return KUrl(QWebView::url());
    }
    QString lastStatusBarText() const
    {
        return m_statusBarText;
    }
    int progress() const
    {
        return m_progress;
    }

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

private slots:
    void setProgress(int progress)
    {
        m_progress = progress;
    }
    void loadFinished();
    void setStatusBarText(const QString &string)
    {
        m_statusBarText = string;
    }
    void downloadRequested(const QNetworkRequest &request);
    void openLinkInNewTab();

private:
    static KActionCollection* s_webActionCollection;

    WebPage *m_page;

    int m_progress;
    QString m_statusBarText;
};

#endif
