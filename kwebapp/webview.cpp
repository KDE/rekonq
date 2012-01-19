/***************************************************************************
 *   Copyright (C) 2011-2012 by Andrea Diamantini <adjam7@gmail.com>                            *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/


// Self Includes
#include "webview.h"
#include "webview.moc"

// KDE Includes
#include <KIO/Job>
#include <KIO/RenameDialog>
#include <KIO/JobUiDelegate>

#include <KGlobalSettings>
#include <KStandardDirs>
#include <KFileDialog>
#include <KJobUiDelegate>
#include <KLocalizedString>
#include <KMenu>
#include <KAction>
#include <KUrl>
#include <KRun>

// Qt Includes
#include <QUrl>
#include <QDebug>
#include <QWebHitTestResult>
#include <QWebHistory>
#include <QNetworkRequest>
#include <QPointer>


WebView::WebView(const QUrl &url, QWidget *parent)
    : KWebView(parent)
{
    page()->setForwardUnsupportedContent(true);
    connect(page(), SIGNAL(unsupportedContent(QNetworkReply*)), page(), SLOT(downloadResponse(QNetworkReply*)));
    connect(page(), SIGNAL(downloadRequested(QNetworkRequest)), page(), SLOT(downloadRequest(QNetworkRequest)));
    connect(this, SIGNAL(linkShiftClicked(KUrl)), page(), SLOT(downloadUrl(KUrl)));

    QWebSettings::setIconDatabasePath(KStandardDirs::locateLocal("cache", "kwebapp.favicons"));

    setContextMenuPolicy(Qt::CustomContextMenu);

    connect(this, SIGNAL(titleChanged(QString)), this, SLOT(setTitle(QString)));
    connect(this, SIGNAL(iconChanged()), this, SLOT(setIcon()));
    connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(menuRequested(QPoint)));

    const QString iconPath = KStandardDirs::locateLocal("cache" , "favicons/" , true) + url.host() + "_WEBAPPICON.png";
    setWindowIcon(QIcon(iconPath));

    // last...
    load(url);
}


void WebView::setTitle(const QString &t)
{
    setWindowTitle(t);
}


void WebView::setIcon()
{
    setWindowIcon(icon());
}


void WebView::menuRequested(const QPoint &pos)
{
    QWebHitTestResult result = page()->mainFrame()->hitTestContent(pos);

    KMenu menu(this);
    QAction *a;

    // is a link?
    if (!result.linkUrl().isEmpty())
    {
        a = new KAction(KIcon("window-new"), i18n("Open in default browser"), this);
        a->setData(result.linkUrl());
        connect(a, SIGNAL(triggered(bool)), this, SLOT(openLinkInDefaultBrowser()));
        menu.addAction(a);

        menu.addAction(pageAction(KWebPage::DownloadLinkToDisk));
        menu.addAction(pageAction(KWebPage::CopyLinkToClipboard));
        menu.addSeparator();
    }

    if (history()->canGoBack())
    {
        menu.addAction(pageAction(KWebPage::Back));
    }

    if (history()->canGoBack())
    {
        menu.addAction(pageAction(KWebPage::Forward));
    }

    menu.addAction(pageAction(KWebPage::Reload));

    menu.exec(mapToGlobal(pos));
}


void WebView::openLinkInDefaultBrowser()
{
    KAction *a = qobject_cast<KAction*>(sender());
    KUrl u(a->data().toUrl());

    (void)new KRun(u, this, 0);
}
