/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2010-2012 by Andrea Diamantini <adjam7 at gmail dot com>
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
#include "historymanager.h"
#include "webwindow.h"
#include "webpage.h"
#include "webtab.h"
#include "urlbar.h"
// #include "newtabpage.h"

// KDE Includes
#include <KIO/Job>
#include <KDirLister>
#include <KLocale>
#include <KLocalizedString>
#include <KMessageBox>
#include <KProcess>
#include <KStandardDirs>
#include <KToolInvocation>
#include <KProtocolInfo>
#include <KRun>

// Qt Includes
#include <QNetworkRequest>
#include <QWebFrame>
#include <QTextDocument>


static bool fileItemListLessThan(const KFileItem &s1, const KFileItem &s2)
{
    return s1.name().toLower() < s2.name().toLower();
}


static KFileItemList sortFileList(const KFileItemList &list)
{
    KFileItemList orderedList, dirList, fileList;

    // order dirs before files..
    Q_FOREACH(const KFileItem & item, list)
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
    , _webwin(0)
{
}


void ProtocolHandler::setWindow(WebWindow *w)
{
    _webwin = w;
    _lister->setMainWindow(_webwin);
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

        QVariant result = frame->evaluateJavaScript(scriptSource);
        return true;
    }

    // "about" handling
    if (_url.protocol() == QL1S("about"))
    {
        QByteArray encodedUrl = _url.toEncoded();
        // let webkit manage the about:blank url...
        if (encodedUrl.startsWith(QByteArray("about:blank")))
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
                break;
            case 5: // tabs
                _url = KUrl("about:tabs");
            default: // unuseful
                break;
            }
        }

        WebPage *page = qobject_cast<WebPage *>(frame->page());
        page->setIsOnRekonqPage(true);

// FIXME        NewTabPage p(frame);
//         p.generate(_url);

        return true;
    }

    // "mailto" handling: It needs to be handled both in preHandling (mail url launched)
    // and in postHandling (mail links clicked)
    if (_url.protocol() == QL1S("mailto"))
    {
        KToolInvocation::invokeMailer(_url);
        return true;
    }

    // "apt" handling
    // NOTE: this is a stupid workaround to ensure apt protocol works
    if (_url.protocol() == QL1S("apt"))
    {
        kDebug() << "APT URL: " << _url;
        (void)new KRun(_url, _webwin, 0, _url.isLocalFile());
        return true;
    }

    // let webkit try to load a known (or missing) protocol...
    if (KProtocolInfo::isKnownProtocol(_url))
        return false;

    // Error Message, for those protocols we cannot handle
    KMessageBox::error(_webwin, i18nc("@info", "rekonq does not know how to handle this protocol: %1", _url.protocol()));

    return true;
}


bool ProtocolHandler::postHandling(const QNetworkRequest &request, QWebFrame *frame)
{
    _url = request.url();
    _frame = frame;

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
            connect(_lister, SIGNAL(newItems(KFileItemList)), this, SLOT(showResults(KFileItemList)));
            _lister->openUrl(_url);

            return true;
        }

        return false;
    }

    // we cannot handle this protocol in any way.
    // Try KRunning it...
    if (KProtocolInfo::isKnownProtocol(_url))
    {
        (void)new KRun(_url, _webwin, 0, _url.isLocalFile());
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

        _webwin->urlBar()->setQUrl(_url);
        _webwin->view()->setFocus();

        HistoryManager::self()->addHistoryEntry(_url, _url.prettyUrl());
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

    // 1. default data path
    QString dataPath = QL1S("file://") + infoFilePath;
    dataPath.remove(QL1S("/htmls/rekonqinfo.html"));

    // 2. title
    QString title = _url.prettyUrl();

    // 3. main content
    QString msg = i18nc("%1=an URL", "<h2>Index of %1</h2>", _url.prettyUrl());


    if (rootUrl.cd(".."))
    {
        QString path = rootUrl.prettyUrl();
        QString uparrow = KIconLoader::global()->iconPath("arrow-up", KIconLoader::Small);
        msg += "<img src=\"file://" + uparrow + "\" alt=\"up-arrow\" />";
        msg += "<a href=\"" + path + "\">" + i18n("Up to higher level directory") + "</a><br /><br />";
    }

    msg += QL1S("<table width=\"95%\" align=\"center\">");
    msg += QL1S("<tr>");
    msg += QL1S("<th align=\"left\">") + i18n("Name") + QL1S("</th>");
    msg += QL1S("<th align=\"center\">") + i18n("Size") + QL1S("</th>");
    msg += QL1S("<th align=\"right\">") + i18n("Last Modified") + QL1S("</th>");
    msg += QL1S("</tr>");

    KFileItemList orderedList = sortFileList(list);
    Q_FOREACH(const KFileItem & item, orderedList)
    {
        msg += QL1S("<tr>");
        QString fullPath = Qt::escape(item.url().prettyUrl());

        QString iconName = item.iconName();
        QString icon = QString("file://") + KIconLoader::global()->iconPath(iconName, KIconLoader::Small);

        msg += QL1S("<td width=\"70%\">");
        msg += QL1S("<img src=\"") + icon + QL1S("\" alt=\"") + iconName + QL1S("\" /> ");
        msg += QL1S("<a href=\"") + fullPath + QL1S("\">") + Qt::escape(item.name()) + QL1S("</a>");
        msg += QL1S("</td>");

        msg += QL1S("<td align=\"right\">");
        if (item.isFile())
        {
            msg += KGlobal::locale()->formatByteSize(item.size(), 1);
        }
        msg += QL1S("</td>");

        msg += QL1S("<td align=\"right\">");
        msg += item.timeString();
        msg += QL1S("</td>");

        msg += QL1S("</tr>");
    }
    msg += QL1S("</table>");

    // done. Replace variables and show it
    QString html = QL1S(file.readAll());

    html.replace(QL1S("$DEFAULT_PATH"), dataPath);
    html.replace(QL1S("$PAGE_TITLE"), title);
    html.replace(QL1S("$MAIN_CONTENT"), msg);

    return html;
}


void ProtocolHandler::slotMostLocalUrlResult(KJob *job)
{
    if (job->error())
    {
        kDebug() << "JOB ERROR: " << job->errorString();
        // TODO
    }
    else
    {
        KIO::StatJob *statJob = static_cast<KIO::StatJob*>(job);
        KIO::UDSEntry entry = statJob->statResult();
        if (entry.isDir())
        {
            connect(_lister, SIGNAL(newItems(KFileItemList)), this, SLOT(showResults(KFileItemList)));
            _lister->openUrl(_url);
        }
        else
        {
            emit downloadUrl(_url);
        }
    }
}
