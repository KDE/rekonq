/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2008-2014 by Andrea Diamantini <adjam7 at gmail dot com>
* Copyright (C) 2009 by Domrachev Alexandr <alexandr.domrachev@gmail.com>
* Copyright (C) 2009 by Paweł Prażak <pawelprazak at gmail dot com>
* Copyright (C) 2009-2011 by Lionel Chauvin <megabigbug@yahoo.fr>
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
#include "urlbar.h"
#include "urlbar.moc"

// Auto Includes
#include "rekonq.h"

// App Includes
#include "application.h"

// Local Includes
#include "adblockmanager.h"
#include "bookmarkmanager.h"
#include "iconmanager.h"

#include "adblockwidget.h"
#include "bookmarkwidget.h"
#include "rsswidget.h"
#include "sslwidget.h"

#include "completionwidget.h"
#include "urlresolver.h"

#include "webtab.h"
#include "webpage.h"
#include "searchengine.h"
#include "websnap.h"

// KDE Includes
#include <KCompletionBox>
#include <KStandardDirs>
#include <KColorScheme>
#include <KMenu>
#include <KIcon>
#include <KIconLoader>
#include <KMessageBox>
#include <KStandardAction>
#include <KAction>

// Qt Includes
#include <QPainter>
#include <QPaintEvent>
#include <QPalette>
#include <QVBoxLayout>
#include <QClipboard>
#include <QTimer>

// const values
const int c_iconMargin = 4;


IconButton::IconButton(QWidget *parent)
    : QToolButton(parent)
{
    setToolButtonStyle(Qt::ToolButtonIconOnly);
    setStyleSheet("IconButton { background-color:transparent; border: none; padding: 0px}");
    setCursor(Qt::ArrowCursor);

    setContextMenuPolicy(Qt::PreventContextMenu);
}


void IconButton::mouseReleaseEvent(QMouseEvent* event)
{
    emit clicked(event->globalPos());
}


// -----------------------------------------------------------------------------------------------------------


QString guessUrlWithCustomFirstLevel(const QString &str1, const QString &str2)
{
    QUrl url(QL1S("http://www.") + str1);
    QString host = url.host().toLower();
    if (!host.endsWith(str2, Qt::CaseInsensitive))
    {
        host += str2;
        url.setHost(host);
    }
    return url.toString();
}


// -----------------------------------------------------------------------------------------------------------


UrlBar::UrlBar(QWidget *parent)
    : KLineEdit(parent)
    , _box(new CompletionWidget(this))
    , _tab(0)
    , _icon(new IconButton(this))
    , _suggestionTimer(new QTimer(this))
{
    setLayoutDirection(Qt::LeftToRight);

    // set initial icon
    _icon->setIcon(KIcon("arrow-right"));

    // initial style
    setStyleSheet(QString("UrlBar { padding: 2px 0 2px %1px; height: %1px } ").arg(_icon->sizeHint().width()));

    // doesn't show the clear button
    setClearButtonShown(false);

    // enable dragging
    setDragEnabled(true);

    // insert decoded URLs
    setUrlDropsEnabled(true);

    // tooltip
    setToolTip(i18n("Type here to search your bookmarks, history and the web..."));

    // accept focus, via tabbing, clicking & wheeling
    setFocusPolicy(Qt::WheelFocus);

    // disable completion object (we have our own :) )
    setCompletionObject(0);

    _tab = qobject_cast<WebTab *>(parent);

    connect(_tab, SIGNAL(loadProgressing()), this, SLOT(update()));

    connect(_tab, SIGNAL(urlChanged(QUrl)), this, SLOT(setQUrl(QUrl)));
    connect(_tab, SIGNAL(loadFinished(bool)), this, SLOT(loadFinished()));
    connect(_tab, SIGNAL(loadStarted()), this, SLOT(loadStarted()));

    // bookmark icon
    connect(BookmarkManager::self(), SIGNAL(bookmarksUpdated()), this, SLOT(updateRightIcons()));

    // suggestions
    connect(_box.data(), SIGNAL(chosenUrl(KUrl,Rekonq::OpenType)), this, SLOT(loadRequestedUrl(KUrl,Rekonq::OpenType)));
    connect(this, SIGNAL(textEdited(QString)), this, SLOT(detectTypedString(QString)));

    _suggestionTimer->setSingleShot(true);
    connect(_suggestionTimer, SIGNAL(timeout()), this, SLOT(suggest()));
}


UrlBar::~UrlBar()
{
    _suggestionTimer->stop();
    _box.clear();

    disconnect();
}


void UrlBar::setQUrl(const QUrl& url)
{
    if (url.scheme() == QL1S("rekonq"))
        return;

    // we don't set empty url
    if (url.isEmpty())
        return;
    
    clearFocus();
    
    // Workaround for KLineEdit bug: incorrectly displaying
    // unicode symbols at query parameter
    const QByteArray urlTextData = url.toString().toUtf8();
    const QString humanReadableUrl = QString::fromUtf8(
        QByteArray::fromPercentEncoding(urlTextData).constData()
    );
    
    // End workaround
    setText(humanReadableUrl);

    setCursorPosition(0);
}


void UrlBar::loadRequestedUrl(const KUrl& url, Rekonq::OpenType type)
{
    clearFocus();
    
    // Workaround for KLineEdit bug: incorrectly displaying
    // unicode symbols at query parameter
    const QByteArray urlTextData = url.prettyUrl().toUtf8();
    const QString humanReadableUrl = QString::fromUtf8(
        QByteArray::fromPercentEncoding(urlTextData).constData()
    );
    
    // End workaround
    setText(humanReadableUrl);
    
    rApp->loadUrl(url, type);
}


void UrlBar::loadTypedUrl()
{
    KUrl urlToLoad;
    if (!_box.isNull())
    {
        urlToLoad = _box.data()->activeSuggestion();
        if (!urlToLoad.isEmpty())
        {
            loadRequestedUrl(urlToLoad);
            return;
        }
    }

    // fallback here
    urlToLoad = UrlResolver::urlFromTextTyped(text());
    loadRequestedUrl(urlToLoad);
}


void UrlBar::paintEvent(QPaintEvent *event)
{
    KColorScheme colorScheme(palette().currentColorGroup());
    QColor backgroundColor;
    QColor foregroundColor;

    if (_tab->page()->settings()->testAttribute(QWebSettings::PrivateBrowsingEnabled))
    {
        backgroundColor = QColor(220, 220, 220);  // light gray
        foregroundColor = Qt::black;
    }
    else
    {
        backgroundColor = rApp->palette().color(QPalette::Base);
        foregroundColor = rApp->palette().color(QPalette::Text);
    }

    // set background color of UrlBar
    QPalette p = palette();

    int progr = _tab->progress();
    if (progr == 0 || progr == 100)
    {
        p.setBrush(QPalette::Base, backgroundColor);
        p.setBrush(QPalette::Text, foregroundColor);
    }
    else
    {
        QColor highlight = rApp->palette().color(QPalette::Highlight);

        int r = (highlight.red() + 2 * backgroundColor.red()) / 3;
        int g = (highlight.green() + 2 * backgroundColor.green()) / 3;
        int b = (highlight.blue() + 2 * backgroundColor.blue()) / 3;

        QColor loadingColor(r, g, b);

        if (abs(loadingColor.lightness() - backgroundColor.lightness()) < 20) //eg. Gaia color scheme
        {
            r = (2 * highlight.red() + backgroundColor.red()) / 3;
            g = (2 * highlight.green() + backgroundColor.green()) / 3;
            b = (2 * highlight.blue() + backgroundColor.blue()) / 3;
            loadingColor = QColor(r, g, b);
        }

        QLinearGradient gradient(QPoint(0, 0), QPoint(width(), 0));
        gradient.setColorAt(0, loadingColor);
        gradient.setColorAt(((double)progr) / 100 - .000001, loadingColor);
        gradient.setColorAt(((double)progr) / 100, backgroundColor);
        p.setBrush(QPalette::Base, gradient);
    }
    setPalette(p);

    // you need this before our code to draw inside the line edit..
    KLineEdit::paintEvent(event);

    if (text().isEmpty() && (progr == 0 || progr == 100))
    {
        QStyleOptionFrame option;
        initStyleOption(&option);
        QRect textRect = style()->subElementRect(QStyle::SE_LineEditContents, &option, this);
        QPainter painter(this);
        painter.setPen(Qt::gray);
        painter.drawText(textRect,
                         Qt::AlignVCenter | Qt::AlignCenter,
                         i18n("Type here to search your bookmarks, history and the web...")
                        );
    }
}


void UrlBar::keyReleaseEvent(QKeyEvent *event)
{
    QString trimmedText = text().trimmed();

    if (trimmedText.isEmpty())
    {
        disconnect(_icon);
        _icon->setIcon(KIcon("arrow-right"));
        return KLineEdit::keyReleaseEvent(event);
    }
    
    // this handles the Modifiers + Return key combinations
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)
    {
        KUrl urlToLoad;
        switch (event->modifiers())
        {
        case Qt::AltModifier:
            urlToLoad = UrlResolver::urlFromTextTyped(text());
            loadRequestedUrl(urlToLoad, Rekonq::NewFocusedTab);
            break;

        case Qt::ControlModifier:
            loadRequestedUrl(guessUrlWithCustomFirstLevel(trimmedText, QL1S(".com")));
            break;

        case 0x06000000: // Qt::ControlModifier | Qt::ShiftModifier:
            loadRequestedUrl(guessUrlWithCustomFirstLevel(trimmedText, QL1S(".org")));
            break;

        case Qt::ShiftModifier:
            loadRequestedUrl(guessUrlWithCustomFirstLevel(trimmedText, QL1S(".net")));
            break;

        default:
            urlToLoad = UrlResolver::urlFromTextTyped(text());
            loadRequestedUrl(urlToLoad);
            break;
        }
    }

    if (event->key() == Qt::Key_Escape)
    {
        clearFocus();
        if (!(_tab->url().protocol() == QL1S("rekonq")))
            setText(_tab->url().prettyUrl());
        event->accept();
    }

    KLineEdit::keyReleaseEvent(event);
}


void UrlBar::focusInEvent(QFocusEvent *event)
{
    emit focusIn();
    KLineEdit::focusInEvent(event);
}


void UrlBar::dropEvent(QDropEvent *event)
{
    // handles only plain-text with url format
    if (event->mimeData()->hasFormat("text/plain") && event->source() != this)
    {
        QUrl url = QUrl::fromUserInput(event->mimeData()->data("text/plain"));

        if (url.isValid())
        {
            setQUrl(url);
            loadRequestedUrl(text());
            return;
        }
    }

    // handles everything else
    KLineEdit::dropEvent(event);
    loadRequestedUrl(text());
}


void UrlBar::loadStarted()
{
    _icon->setIcon(KIcon("text-html"));
    clearRightIcons();
}


void UrlBar::loadFinished()
{
    refreshFavicon();
    updateRightIcons();
}


void UrlBar::updateRightIcons()
{
    if (_tab->isPageLoading())
        return;

    clearRightIcons();
    
    if (_tab->url().scheme() == QL1S("rekonq"))
    {
        update();
        return;
    }
    
    // show bookmark info
    IconButton *bt = addRightIcon(UrlBar::BK);
    connect(bt, SIGNAL(clicked(QPoint)), this, SLOT(manageStarred(QPoint)));

    // show KGet downloads??
    if (!KStandardDirs::findExe("kget").isNull() && ReKonfig::kgetList())
    {
        IconButton *bt = addRightIcon(UrlBar::KGet);
        connect(bt, SIGNAL(clicked(QPoint)), _tab->page(), SLOT(downloadAllContentsWithKGet()));
    }

    // show RSS
    if (_tab->hasRSSInfo())
    {
        IconButton *bt = addRightIcon(UrlBar::RSS);
        connect(bt, SIGNAL(clicked(QPoint)), this, SLOT(showRSSInfo(QPoint)));
    }

    // Show adblock
    if (AdBlockManager::self()->isEnabled())
    {
        IconButton *bt = addRightIcon(UrlBar::AdBlock);
        connect(bt, SIGNAL(clicked(QPoint)), this, SLOT(manageAdBlock(QPoint)));
    }

    // we need to update urlbar after the right icon settings
    // removing this code (where setStyleSheet automatically calls update) needs adding again
    // an update call
    int oneIconWidth = _icon->sizeHint().width();
    int rightIconWidth = (oneIconWidth + c_iconMargin) * (_rightIconsList.count());
    setStyleSheet(QString("UrlBar { padding: 2px %2px 2px %1px; height: %1px } ").arg(oneIconWidth).arg(rightIconWidth));
}


void UrlBar::mouseDoubleClickEvent(QMouseEvent *event)
{
    Q_UNUSED(event);

    selectAll();
}


void UrlBar::contextMenuEvent(QContextMenuEvent* event)
{
    KMenu menu;
    const bool clipboardFilled = !rApp->clipboard()->text().isEmpty();

    // Cut
    KAction *a = KStandardAction::cut(this, SLOT(cut()), &menu);
    a->setEnabled(hasSelectedText());
    menu.addAction(a);

    // Copy
    a = KStandardAction::copy(this, SLOT(copy()), &menu);
    a->setEnabled(hasSelectedText());
    menu.addAction(a);

    // Paste
    a = KStandardAction::paste(this, SLOT(paste()), &menu);
    a->setEnabled(clipboardFilled);
    menu.addAction(a);

    // Paste & Go
    const QString clipboardText = rApp->clipboard()->text();
    if (isValidURL(clipboardText) || clipboardText.isEmpty())
    {
        a = new KAction(i18n("Paste && Go"), &menu);
        connect(a, SIGNAL(triggered(bool)), this, SLOT(pasteAndGo()));
    }
    else
    {
        a = new KAction(i18n("Paste && Search"), &menu);
        connect(a, SIGNAL(triggered(bool)), this, SLOT(pasteAndSearch()));
    }
    a->setEnabled(clipboardFilled);
    menu.addAction(a);

    // Delete
    a = new KAction(KIcon("edit-delete"), i18n("Delete"), &menu);
    connect(a, SIGNAL(triggered(bool)), this, SLOT(delSlot()));
    a->setEnabled(hasSelectedText());
    menu.addAction(a);

    menu.addSeparator();

    // Select All
    a = KStandardAction::selectAll(this, SLOT(selectAll()), &menu);
    a->setEnabled(!text().isEmpty());
    menu.addAction(a);

    menu.exec(event->globalPos());
}


bool UrlBar::isValidURL(QString url)
{
    bool isValid = false;
    if (url.startsWith(QL1S("http://"))
            || url.startsWith(QL1S("https://"))
            || url.startsWith(QL1S("ftp://"))
       )
        url = url.remove(QRegExp("(http|https|ftp)://"));

    if (url.contains(QL1C('.'))
            && url.indexOf(QL1C('.')) > 0
            && url.indexOf(QL1C('.')) < url.length()
            && !url.trimmed().contains(QL1C(' '))
            && QUrl::fromUserInput(url).isValid()
       )
        isValid = true;

    return isValid;
}


IconButton *UrlBar::addRightIcon(UrlBar::icon ic)
{
    IconButton *rightIcon = new IconButton(this);

    switch (ic)
    {
    case UrlBar::KGet:
        rightIcon->setIcon(KIcon("download"));
        rightIcon->setToolTip(i18n("List all links with KGet"));
        break;
    case UrlBar::RSS:
        rightIcon->setIcon(KIcon("application-rss+xml"));
        rightIcon->setToolTip(i18n("List all available RSS feeds"));
        break;
    case UrlBar::BK:
        if (BookmarkManager::self()->bookmarkForUrl(_tab->url()).isNull() && 
            !ReKonfig::previewUrls().contains(_tab->url().url()))
        {
            rightIcon->setIcon(KIcon("bookmarks").pixmap(32, 32, QIcon::Disabled));
        }
        else
        {
            rightIcon->setIcon(KIcon("bookmarks"));
        }
        break;
    case UrlBar::SearchEngine:
    {
        KIcon wsIcon("edit-web-search");
        if (wsIcon.isNull())
        {
            wsIcon = KIcon("preferences-web-browser-shortcuts");
        }
        rightIcon->setIcon(wsIcon);
        rightIcon->setToolTip(i18n("Add search engine"));
        break;
    }
    case UrlBar::AdBlock:
    {
        QStringList hosts = ReKonfig::whiteReferer();
        if (!hosts.contains(_tab->url().host()))
        {
            rightIcon->setIcon(KIcon("preferences-web-browser-adblock"));
            rightIcon->setToolTip(i18n("AdBlock is enabled on this site"));
        }
        else
        {
            rightIcon->setIcon(KIcon("preferences-web-browser-adblock").pixmap(32, 32, QIcon::Disabled));
            rightIcon->setToolTip(i18n("AdBlock is not enabled on this site"));
        }
        break;
    }
    default:
        ASSERT_NOT_REACHED("ERROR.. default non extant case!!");
        break;
    }

    _rightIconsList << rightIcon;

    int iconsCount = _rightIconsList.count();
    updateRightIconPosition(rightIcon, iconsCount);

    rightIcon->show();

    return rightIcon;
}


void UrlBar::clearRightIcons()
{
    qDeleteAll(_rightIconsList);
    _rightIconsList.clear();
}


void UrlBar::resizeEvent(QResizeEvent *event)
{
    int ih = _icon->sizeHint().height();
    int iconsCount = _rightIconsList.count();
    int iconHeight = (height() - ih) / 2;

    _icon->move(c_iconMargin, iconHeight);

    for (int i = 0; i < iconsCount; ++i)
    {
        IconButton *bt = _rightIconsList.at(i);
        updateRightIconPosition(bt, i + 1);
    }

    KLineEdit::resizeEvent(event);
}


void UrlBar::detectTypedString(const QString &typed)
{
    if (typed.count() == 1)
    {
        _icon->setIcon(KIcon("arrow-right"));
        QTimer::singleShot(0, this, SLOT(suggest()));
        return;
    }

    if (_suggestionTimer->isActive())
        _suggestionTimer->stop();
    _suggestionTimer->start(100);
}


void UrlBar::suggest()
{
    if (!_box.isNull())
    {
        _box.data()->suggestUrls(text().trimmed());
    }
}


void UrlBar::refreshFavicon()
{
    _icon->disconnect();
    
    const QString scheme = _tab->url().protocol();
    
    if (_tab->page()->settings()->testAttribute(QWebSettings::PrivateBrowsingEnabled))
    {
        _icon->setIcon(KIcon("view-media-artist"));
        return;
    }
    
    if (scheme == QL1S("https"))
    {
        if (_tab->page()->hasSslValid())
        {
            _icon->setIcon(KIcon("security-high"));
        }
        else
        {
            _icon->setIcon(KIcon("security-low"));
        }
        
        connect(_icon, SIGNAL(clicked(QPoint)), this, SLOT(showSSLInfo(QPoint)), Qt::UniqueConnection);
        return;
    }

    if (scheme == QL1S("rekonq"))
    {
        _icon->setIcon(KIcon("arrow-right"));
        return;
    }

    _icon->setIcon(KIcon("text-html"));
}


void UrlBar::pasteAndGo()
{
    KUrl urlToLoad = UrlResolver::urlFromTextTyped(rApp->clipboard()->text().trimmed());
    kDebug() << "Url to load: " << urlToLoad;
    loadRequestedUrl(urlToLoad);
}


void UrlBar::pasteAndSearch()
{
    KService::Ptr defaultEngine = SearchEngine::defaultEngine();
    if (defaultEngine)
        loadRequestedUrl(KUrl(SearchEngine::buildQuery(defaultEngine, QApplication::clipboard()->text().trimmed())));
}


void UrlBar::delSlot()
{
    del();
}


void UrlBar::manageBookmarks()
{
    if (_tab->url().scheme() == QL1S("rekonq"))
        return;

    KBookmark bookmark = BookmarkManager::self()->bookmarkForUrl(_tab->url());

    if (bookmark.isNull())
    {
        bookmark = BookmarkManager::self()->bookmarkCurrentPage();
    }

    // calculate position
    int iconSize = IconSize(KIconLoader::Small) + c_iconMargin;

    // Add a generic 10 to move it a bit below and right.
    // No need to be precise...
    int iconWidth = 10 + width() - ((iconSize + c_iconMargin));
    int iconHeight = 10 + (height() - iconSize) / 2;

    QPoint p = mapToGlobal(QPoint(iconWidth, iconHeight));

    // show bookmark widget
    BookmarkWidget *widget = new BookmarkWidget(bookmark, window());
    widget->showAt(p);
}


void UrlBar::manageAdBlock(QPoint pos)
{
    IconButton *bt = qobject_cast<IconButton *>(this->sender());
    if (!bt)
        return;

    if (_tab->url().scheme() == QL1S("rekonq"))
        return;

    AdBlockWidget *widget = new AdBlockWidget(_tab->url(), this);
    connect(widget, SIGNAL(updateIcon()), this, SLOT(updateRightIcons()));
    widget->showAt(pos);
}


void UrlBar::updateRightIconPosition(IconButton *icon, int iconsCount)
{
    // NOTE: cannot show a (let's say) 16x16 icon in a 16x16 square.
    // It needs some margin. It usually is 3, but using 4 (default rekonq icon margin)
    // seems NOT a big problem and let's us using just one const ;)
    int iconSize = IconSize(KIconLoader::Small) + c_iconMargin;

    int iconWidth = width() - ((iconSize + c_iconMargin) * iconsCount);
    int iconHeight = (height() - iconSize) / 2;

    icon->move(iconWidth, iconHeight);
}


void UrlBar::showRSSInfo(QPoint pos)
{
    QWebElementCollection col = _tab->page()->mainFrame()->findAllElements("link[type=\"application/rss+xml\"]");
    col.append(_tab->page()->mainFrame()->findAllElements("link[type=\"application/atom+xml\"]"));

    QMap<KUrl, QString> map;

    Q_FOREACH(const QWebElement & el, col)
    {
        QString urlString;
        if (el.attribute("href").startsWith(QL1S("http")))
            urlString = el.attribute("href");
        else
        {
            KUrl u = _tab->url();
            // NOTE
            // cd() is probably better than setPath() here,
            // for all those url sites just having a path
            if (u.cd(el.attribute("href")))
                urlString = u.toMimeDataString();
        }

        QString title = el.attribute("title");
        if (title.isEmpty())
            title = el.attribute("href");

        map.insert(KUrl(urlString), title);
    }

    RSSWidget *widget = new RSSWidget(map, window());
    widget->showAt(pos);
}


void UrlBar::showSSLInfo(QPoint pos)
{
    if (_tab->url().scheme() == QL1S("https"))
    {
        SSLWidget *widget = new SSLWidget(_tab->url(), _tab->page()->sslInfo(), this);
        widget->showAt(pos);
    }
    else
    {
        KMessageBox::information(this,
                                 i18n("This site does not contain SSL information."),
                                 i18nc("Secure Sockets Layer", "SSL")
                                );
    }
}


void UrlBar::manageStarred(QPoint pos)
{
    KMenu menu;
    KAction *a;

    // Bookmarks
    if (BookmarkManager::self()->bookmarkForUrl(_tab->url()).isNull())
    {
        a = new KAction(KIcon(KIcon("bookmarks").pixmap(32, 32, QIcon::Disabled)), i18n("Add Bookmark"), &menu);
        connect(a, SIGNAL(triggered(bool)), this, SLOT(manageBookmarks()));
    }
    else
    {
        a = new KAction(KIcon("bookmarks"), i18n("Edit Bookmark"), &menu);
        connect(a, SIGNAL(triggered(bool)), this, SLOT(manageBookmarks()));        
    }
    menu.addAction(a);
    
    // Favorites
    if (ReKonfig::previewUrls().contains(_tab->url().url()))
    {
        a = new KAction(KIcon("emblem-favorite"), i18n("Remove from Favorites"), &menu);
        connect(a, SIGNAL(triggered(bool)), this, SLOT(removeFromFavorites()));        
    }
    else
    {
        a = new KAction(KIcon(KIcon("emblem-favorite").pixmap(32, 32, QIcon::Disabled)), i18n("Add to Favorites"), &menu);
        connect(a, SIGNAL(triggered(bool)), this, SLOT(addToFavorites()));
    }
    menu.addAction(a);
    
    QPoint p(pos.x() - menu.sizeHint().width() + 15, pos.y() + 15);
    menu.exec(p);
}


void UrlBar::addToFavorites()
{
    if (_tab->url().scheme() == QL1S("rekonq"))
        return;

    // else, add as favorite
    QStringList urls = ReKonfig::previewUrls();
    urls << _tab->url().url();
    ReKonfig::setPreviewUrls(urls);

    QStringList titles = ReKonfig::previewNames();
    titles << _tab->view()->title();
    ReKonfig::setPreviewNames(titles);

    // also, save a site snapshot
    WebSnap *snap = new WebSnap(_tab->url(), this);
    Q_UNUSED(snap);

    updateRightIcons();
}


void UrlBar::removeFromFavorites()
{
    if (_tab->url().scheme() == QL1S("rekonq"))
        return;

    QStringList urls = ReKonfig::previewUrls();
    if (urls.removeOne(_tab->url().url()))
    {
        ReKonfig::setPreviewUrls(urls);
        QStringList titles = ReKonfig::previewNames();
        titles.removeOne(_tab->view()->title());
        ReKonfig::setPreviewNames(titles);

        updateRightIcons();
    }    
}


void UrlBar::clearUrlbar()
{
    clear();
    clearRightIcons();
    setFocus();
}
