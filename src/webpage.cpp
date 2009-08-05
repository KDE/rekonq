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

// Auto Includes
#include "rekonq.h"

// Local Includes
#include "application.h"
#include "mainwindow.h"
#include "mainview.h"
#include "cookiejar.h"
#include "networkaccessmanager.h"
#include "history.h"
#include "webview.h"

// KDE Includes
#include <KStandardDirs>
#include <KUrl>
#include <KDebug>


#include <KDE/KParts/BrowserRun>
#include <KDE/KMimeTypeTrader>
#include <KDE/KRun>
#include <KDE/KFileDialog>
#include <KDE/KInputDialog>
#include <KDE/KMessageBox>
#include <KDE/KJobUiDelegate>

// Qt Includes
#include <QtGui/QContextMenuEvent>
#include <QtGui/QWheelEvent>
#include <QtGui/QMouseEvent>
#include <QtGui/QClipboard>
#include <QtGui/QKeyEvent>



#include <QUiLoader>

WebPage::WebPage(QObject *parent)
        : QWebPage(parent)
        , m_keyboardModifiers(Qt::NoModifier)
        , m_pressedButtons(Qt::NoButton)
{
    setForwardUnsupportedContent(true);

    setNetworkAccessManager(Application::networkAccessManager());
    connect(networkAccessManager(), SIGNAL(finished(QNetworkReply*)), this, SLOT(manageNetworkErrors(QNetworkReply*)));

    connect(this, SIGNAL(downloadRequested(const QNetworkRequest &)), this, SLOT(slotDownloadRequested(const QNetworkRequest &)));
    connect(this, SIGNAL(unsupportedContent(QNetworkReply *)), this, SLOT(slotHandleUnsupportedContent(QNetworkReply *)));
}


bool WebPage::acceptNavigationRequest(QWebFrame *frame, const QNetworkRequest &request, NavigationType type)
{
    if (m_keyboardModifiers & Qt::ControlModifier || m_pressedButtons == Qt::MidButton)
    {
        Application::instance()->loadUrl(request.url(), Rekonq::SettingOpenTab);
        m_keyboardModifiers = Qt::NoModifier;
        m_pressedButtons = Qt::NoButton;
        return false;
    }

    return QWebPage::acceptNavigationRequest(frame, request, type);
}


WebPage *WebPage::createWindow(QWebPage::WebWindowType type)
{
    kDebug() << "WebPage createWindow slot";

    return newWindow(type);
}


WebPage *WebPage::newWindow(WebWindowType type)
{
    // added to manage web modal dialogs
    if (type == QWebPage::WebModalDialog)
    {
        kDebug() << "Modal Dialog ---------------------------------------";
    }

    WebView *w = Application::instance()->mainWindow()->mainView()->newTab(!ReKonfig::openTabsBack());
    return w->page();
}


// FIXME: dear slot, you need to handle unsupported content a bit better..
void WebPage::slotHandleUnsupportedContent(QNetworkReply *reply)
{

    const KUrl url(reply->request().url());
    kDebug() << "title:" << url;
    kDebug() << "error:" << reply->errorString();

    QString filename = url.fileName();
    QString mimetype = reply->header(QNetworkRequest::ContentTypeHeader).toString();
    KService::Ptr offer = KMimeTypeTrader::self()->preferredService(mimetype);

    KParts::BrowserRun::AskSaveResult res = KParts::BrowserRun::askSave(
                                                url,
                                                offer,
                                                mimetype,
                                                filename
                                            );
    switch (res) 
    {
    case KParts::BrowserRun::Save:
        slotDownloadRequested(reply->request());
        return;
    case KParts::BrowserRun::Cancel:
        return;
    default: // non extant case
        break;
    }

    KUrl::List list;
    list.append(url);
    KRun::run(*offer,url,0);
    return;
}


void WebPage::manageNetworkErrors(QNetworkReply* reply)
{
    if(reply->error() == QNetworkReply::NoError)
        return;

    viewErrorPage(reply);
}


void WebPage::viewErrorPage(QNetworkReply *reply)
{
    // display "not found" page
    QString notfoundFilePath =  KStandardDirs::locate("data", "rekonq/htmls/notfound.html");
    QFile file(notfoundFilePath);
    bool isOpened = file.open(QIODevice::ReadOnly);
    if (!isOpened)
    {
        kWarning() << "Couldn't open the notfound.html file";
        return;
    }
    QString title = i18n("Error loading page: ") + reply->url().toString();

    QString imagePath = KIconLoader::global()->iconPath("rekonq", KIconLoader::NoGroup, false);

    QString html = QString(QLatin1String(file.readAll()))
                   .arg(title)
                   .arg("file://" + imagePath)
                   .arg(reply->errorString())
                   .arg(reply->url().toString());

    // test
    QList<QWebFrame*> frames;
    frames.append(mainFrame());
    while (!frames.isEmpty())
    {
        QWebFrame *firstFrame = frames.takeFirst();
        if (firstFrame->url() == reply->url())
        {
            firstFrame->setHtml(html, reply->url());
            return;
        }
        QList<QWebFrame *> children = firstFrame->childFrames();
        foreach(QWebFrame *frame, children)
        {
            frames.append(frame);
        }
    }
}


void WebPage::javaScriptAlert(QWebFrame *frame, const QString &msg)
{
    KMessageBox::error(frame->page()->view(), msg, i18n("JavaScript"));
}


bool WebPage::javaScriptConfirm(QWebFrame *frame, const QString &msg)
{
    return (KMessageBox::warningYesNo(frame->page()->view(), msg, i18n("JavaScript"), KStandardGuiItem::ok(), KStandardGuiItem::cancel())
            == KMessageBox::Yes);
}


bool WebPage::javaScriptPrompt(QWebFrame *frame, const QString &msg, const QString &defaultValue, QString *result)
{
    bool ok = false;
    *result = KInputDialog::getText(i18n("JavaScript"), msg, defaultValue, &ok, frame->page()->view());
    return ok;
}


QObject *WebPage::createPlugin(const QString &classId, const QUrl &url, const QStringList &paramNames, const QStringList &paramValues)
{
    kDebug() << "create Plugin requested:";
    kDebug() << "classid:" << classId;
    kDebug() << "url:" << url;
    kDebug() << "paramNames:" << paramNames << " paramValues:" << paramValues;

    QUiLoader loader;
    return loader.createWidget(classId, view());
}


void WebPage::slotDownloadRequested(const QNetworkRequest &request)
{
    const KUrl url(request.url());
    kDebug() << url;

    const QString destUrl = KFileDialog::getSaveFileName(url.fileName(), QString(), view());
    if (destUrl.isEmpty()) return;
    KIO::Job *job = KIO::file_copy(url, KUrl(destUrl), -1, KIO::Overwrite);
    //job->setMetaData(metadata); //TODO: add metadata from request
    job->addMetaData("MaxCacheSize", "0"); // Don't store in http cache.
    job->addMetaData("cache", "cache"); // Use entry from cache if available.
    job->uiDelegate()->setAutoErrorHandlingEnabled(true);
}


QString WebPage::chooseFile(QWebFrame *frame, const QString &suggestedFile)
{
    return KFileDialog::getOpenFileName(suggestedFile, QString(), frame->page()->view());
}
