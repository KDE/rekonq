/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2010-2011 by Andrea Diamantini <adjam7 at gmail dot com>
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
#include "protocolhandler.h"
#include "protocolhandler.moc"

// Auto Includes
#include "rekonq.h"

// Local Includes
#include "adblockmanager.h"
#include "application.h"
#include "historymanager.h"
#include "mainview.h"
#include "mainwindow.h"
#include "newtabpage.h"
#include "urlbar.h"
#include "webpage.h"
#include "webtab.h"

// KDE Includes
#include <KIO/Job>
#include <KDirLister>
#include <KLocalizedString>
#include <KMessageBox>
#include <KProcess>
#include <KStandardDirs>
#include <KToolInvocation>
#include <KProtocolInfo>
#include <KRun>

// Qt Includes
#include <QtNetwork/QNetworkRequest>
#include <QtWebKit/QWebFrame>


static bool fileItemListLessThan(const KFileItem &s1, const KFileItem &s2)
{
    return s1.name().toLower() < s2.name().toLower();
}


static KFileItemList sortFileList(const KFileItemList &list)
{
    KFileItemList orderedList, dirList, fileList;

    // order dirs before files..
    Q_FOREACH(const KFileItem &item, list)
    {
        if (item.isDir())
            dirList << item;
        else
            fileList << item;
    }
    qStableSort(dirList.begin(), dirList.end(), fileItemListLessThan);
    qStableSort(fileList.begin(), fileList.end(), fileItemListLessThan);

    orderedList << dirList;
    orderedList << fileList;

    return orderedList;
}


// -------------------------------------------------------------------------------------------


ProtocolHandler::ProtocolHandler(QObject *parent)
    : QObject(parent)
    , _lister(new KDirLister(this))
    , _frame(0)
{
}


bool ProtocolHandler::preHandling(const QNetworkRequest &request, QWebFrame *frame)
{
    _url = request.url();
    _frame = frame;

    // javascript handling
    if (_url.protocol() == QL1S("javascript"))
    {
        QString scriptSource = _url.authority();
        if (scriptSource.isEmpty())
        {
            // if javascript:<code here> then authority() returns
            // an empty string. Extract the source manually
            // Use the prettyUrl() since that is unencoded

            // 11 is length of 'javascript:'
            // fromPercentEncoding() is used to decode all the % encoded
            // characters to normal, so that it is treated as valid javascript
            scriptSource = QUrl::fromPercentEncoding(_url.url().mid(11).toAscii());
            if (scriptSource.isEmpty())
                return false;
        }

        kDebug() << "EVALUATING JAVASCRIPT...";
        QVariant result = frame->evaluateJavaScript(scriptSource);
        return true;
    }

    // "abp" handling
    if (_url.protocol() == QL1S("abp"))
    {
        abpHandling();
        return true;
    }

    // "about" handling
    if (_url.protocol() == QL1S("about"))
    {
        QByteArray encodedUrl = _url.toEncoded();
        // let webkit manage the about:blank url...
        if (encodedUrl == QByteArray("about:blank"))
        {
            return false;
        }

        if (encodedUrl == QByteArray("about:home"))
        {
            switch (ReKonfig::newTabStartPage())
            {
            case 0: // favorites
                _url = KUrl("about:favorites");
                break;
            case 1: // closed tabs
                _url = KUrl("about:closedTabs");
                break;
            case 2: // bookmarks
                _url = KUrl("about:bookmarks");
                break;
            case 3: // history
                _url = KUrl("about:history");
                break;
            case 4: // downloads
                _url = KUrl("about:downloads");
            default: // unuseful
                break;
            }
        }

        WebPage *page = qobject_cast<WebPage *>(frame->page());
        page->setIsOnRekonqPage(true);

        NewTabPage p(frame);
        p.generate(_url);

        return true;
    }

    // let webkit try to load a known (or missing) protocol...
    if (KProtocolInfo::isKnownProtocol(_url))
        return false;
    
    // Error Message, for those protocols we cannot handle
    KMessageBox::error(rApp->mainWindow(), i18nc("@info", "rekonq does not know how to handle this protocol: %1", _url.protocol()));

    return true;
}


bool ProtocolHandler::postHandling(const QNetworkRequest &request, QWebFrame *frame)
{
    _url = request.url();
    _frame = frame;

    kDebug() << "URL PROTOCOL: " << _url;
    
    // "http(s)" (fast) handling
    if (_url.protocol() == QL1S("http") || _url.protocol() == QL1S("https"))
        return false;

    // "mailto" handling: It needs to be handled both here(mail links clicked)
    // and in prehandling (mail url launched)
    if (_url.protocol() == QL1S("mailto"))
    {
        KToolInvocation::invokeMailer(_url);
        return true;
    }

    // "ftp" handling. A little bit "hard" handling this. Hope I found
    // the best solution.
    // My idea is: webkit cannot handle in any way ftp. So we have surely to return true here.
    // We start trying to guess what the url represent: it's a dir? show its contents (and download them).
    // it's a file? download it. It's another thing? beat me, but I don't know what to do...
    if (_url.protocol() == QL1S("ftp"))
    {
        KIO::StatJob *job = KIO::stat(_url);
        connect(job, SIGNAL(result(KJob*)), this, SLOT(slotMostLocalUrlResult(KJob*)));
        return true;
    }

    // "file" handling. This is quite trivial :)
    if (_url.protocol() == QL1S("file"))
    {
        QFileInfo fileInfo(_url.path());
        if (fileInfo.isDir())
        {
            connect(_lister, SIGNAL(newItems(const KFileItemList &)), this, SLOT(showResults(const KFileItemList &)));
            _lister->openUrl(_url);

            return true;
        }
    }

    // we cannot handle this protocol in any way.
    // Try KRunning it...
    if (KProtocolInfo::isKnownProtocol(_url))
    {
        kDebug() << "WARNING: launching a new app...";
        (void)new KRun(_url, rApp->mainWindow(), 0, _url.isLocalFile());
        return true;
    }

    return false;
}


// ---------------------------------------------------------------------------------------------------------------------------


void ProtocolHandler::showResults(const KFileItemList &list)
{
    if (!_lister->rootItem().isNull() && _lister->rootItem().isReadable() && _lister->rootItem().isFile())
    {
        emit downloadUrl(_lister->rootItem().url());
    }
    else
    {
        QString html = dirHandling(list);
        _frame->setHtml(html);
        qobject_cast<WebPage *>(_frame->page())->setIsOnRekonqPage(true);

        rApp->mainWindow()->mainView()->currentUrlBar()->setQUrl(_url);
        rApp->mainWindow()->currentTab()->setFocus();
        rApp->historyManager()->addHistoryEntry(_url.prettyUrl());
    }
}


QString ProtocolHandler::dirHandling(const KFileItemList &list)
{
    if (!_lister)
    {
        return QString("rekonq error, sorry :(");
    }

    // let me modify it..
    KUrl rootUrl = _url;

    // display "rekonq info" page
    QString infoFilePath =  KStandardDirs::locate("data", "rekonq/htmls/rekonqinfo.html");
    QFile file(infoFilePath);

    bool isOpened = file.open(QIODevice::ReadOnly);
    if (!isOpened)
    {
        return QString("rekonq error, sorry :(");
    }

    QString title = _url.prettyUrl();
    QString msg = i18nc("%1=an URL", "<h1>Index of %1</h1>", _url.prettyUrl());


    if (rootUrl.cd(".."))
    {
        QString path = rootUrl.prettyUrl();
        QString uparrow = KIconLoader::global()->iconPath("arrow-up", KIconLoader::Small);
        msg += "<img src=\"file://" + uparrow + "\" alt=\"up-arrow\" />";
        msg += "<a href=\"" + path + "\">" + i18n("Up to higher level directory") + "</a><br /><br />";
    }

    msg += "<table width=\"100%\">";
    msg += "<tr><th align=\"left\">" + i18n("Name") + "</th><th>" + i18n("Size") + "</th><th>" + i18n("Last Modified") + "</th></tr>";

    KFileItemList orderedList = sortFileList(list);
    Q_FOREACH(const KFileItem &item, orderedList)
    {
        msg += "<tr>";
        QString fullPath = item.url().prettyUrl();

        QString iconName = item.iconName();
        QString icon = QString("file://") + KIconLoader::global()->iconPath(iconName, KIconLoader::Small);

        msg += "<td width=\"70%\">";
        msg += "<img src=\"" + icon + "\" alt=\"" + iconName + "\" /> ";
        msg += "<a href=\"" + fullPath + "\">" + item.name() + "</a>";
        msg += "</td>";

        msg += "<td align=\"right\">";
        if (item.isFile())
        {
            msg += QString::number(item.size() / 1024) + " KB";
        }
        msg += "</td>";

        msg += "<td align=\"right\">";
        msg += item.timeString();
        msg += "</td>";

        msg += "</tr>";
    }
    msg += "</table>";


    QString html = QString(QL1S(file.readAll()))
                   .arg(title)
                   .arg(msg)
                   ;

    return html;
}


void ProtocolHandler::slotMostLocalUrlResult(KJob *job)
{
    if (job->error())
    {
        // TODO
        kDebug() << "error";
    }
    else
    {
        KIO::StatJob *statJob = static_cast<KIO::StatJob*>(job);
        KIO::UDSEntry entry = statJob->statResult();
        if (entry.isDir())
        {
            connect(_lister, SIGNAL(newItems(const KFileItemList &)), this, SLOT(showResults(const KFileItemList &)));
            _lister->openUrl(_url);
        }
        else
        {
            emit downloadUrl(_url);
        }
    }
}


/**
 * abp scheme (easy) explanation
 *
 */
void ProtocolHandler::abpHandling()
{
    kDebug() << "handling abp url: " << _url;

    QString path = _url.path();
    if (path != QL1S("subscribe"))
        return;

    QMap<QString, QString> map = _url.queryItems(KUrl::CaseInsensitiveKeys);

    QString location = map.value(QL1S("location"));
    QString title = map.value(QL1S("title"));
    QString requireslocation = map.value(QL1S("requireslocation"));
    QString requirestitle = map.value(QL1S("requirestitle"));

    QString info;
    if (requirestitle.isEmpty() || requireslocation.isEmpty())
    {
        info = title;
    }
    else
    {
        info = i18n("\n %1,\n %2 (required by %3)\n", title, requirestitle, title);
    }

    if (KMessageBox::questionYesNo(0,
                                   i18n("Do you want to add the following subscriptions to your adblock settings?\n") + info,
                                   i18n("Add automatic subscription to the adblock"),
                                   KGuiItem(i18n("Add")),
                                   KGuiItem(i18n("Discard"))
                                  )
       )
    {
        if (!requireslocation.isEmpty() && !requirestitle.isEmpty())
        {
            rApp->adblockManager()->addSubscription(requirestitle, requireslocation);
        }
        rApp->adblockManager()->addSubscription(title, location);
        rApp->adblockManager()->loadSettings(false);
    }
}
