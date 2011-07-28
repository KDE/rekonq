/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2008-2011 by Andrea Diamantini <adjam7 at gmail dot com>
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

// Local Includes
#include "application.h"
#include "mainwindow.h"
#include "webtab.h"
#include "webpage.h"
#include "webview.h"
#include "completionwidget.h"
#include "bookmarkprovider.h"
#include "bookmarkowner.h"
#include "bookmarkwidget.h"
#include "iconmanager.h"
#include "favoritewidget.h"

// KDE Includes
#include <KCompletionBox>
#include <KStandardDirs>
#include <KColorScheme>
#include <KMenu>

// Qt Includes
#include <QtGui/QPainter>
#include <QtGui/QPaintEvent>
#include <QtGui/QPalette>
#include <QtGui/QVBoxLayout>



IconButton::IconButton(QWidget *parent)
    : QToolButton(parent)
{
    setToolButtonStyle(Qt::ToolButtonIconOnly);
    setStyleSheet("IconButton { background-color:transparent; border: none; padding: 0px}");
    setCursor(Qt::ArrowCursor);
}


void IconButton::mouseReleaseEvent(QMouseEvent* event)
{
    emit clicked(event->globalPos());
}


// -----------------------------------------------------------------------------------------------------------


UrlBar::UrlBar(QWidget *parent)
    : KLineEdit(parent)
    , _tab(0)
    , _icon(new IconButton(this))
    , _suggestionTimer(new QTimer(this))
{
    // initial style
    setStyleSheet(QString("UrlBar { padding: 0 0 0 %1px;} ").arg(_icon->sizeHint().width()));

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

    connect(_tab->view(), SIGNAL(urlChanged(const QUrl &)), this, SLOT(setQUrl(const QUrl &)));
    connect(_tab->view(), SIGNAL(loadFinished(bool)), this, SLOT(loadFinished()));
    connect(_tab->view(), SIGNAL(loadStarted()), this, SLOT(clearRightIcons()));
    connect(_tab->view(), SIGNAL(iconChanged()), this, SLOT(refreshFavicon()));

    // search icon
    connect(rApp->opensearchManager(), SIGNAL(openSearchEngineAdded(const QString &, const QString &, const QString &)),
            this, SLOT(updateRightIcons()));

    _suggestionTimer->setSingleShot(true);
    connect(_suggestionTimer, SIGNAL(timeout()), this, SLOT(suggest()));

    activateSuggestions(true);
}


UrlBar::~UrlBar()
{
    _suggestionTimer->stop();
    activateSuggestions(false);
    _box.clear();
}


void UrlBar::setQUrl(const QUrl& url)
{
    if (url.scheme() == QL1S("about"))
    {
        clear();
        setFocus();
    }
    else
    {
        clearFocus();
        KLineEdit::setUrl(url);
        setCursorPosition(0);
        refreshFavicon();
    }
}


void UrlBar::activated(const KUrl& url, Rekonq::OpenType type)
{
    activateSuggestions(false);
    clearFocus();
    setUrl(url);
    rApp->loadUrl(url, type);
}


void UrlBar::paintEvent(QPaintEvent *event)
{
    KColorScheme colorScheme(palette().currentColorGroup());
    QColor backgroundColor;
    QColor foregroundColor;

    if (QWebSettings::globalSettings()->testAttribute(QWebSettings::PrivateBrowsingEnabled))
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
    if (progr == 0)
    {
        if (_tab->url().scheme() == QL1S("https"))
        {
            backgroundColor = _tab->page()->hasSslValid()
                              ? colorScheme.background(KColorScheme::PositiveBackground).color()
                              : colorScheme.background(KColorScheme::NegativeBackground).color();

            foregroundColor = colorScheme.foreground(KColorScheme::NormalText).color();
        }
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

    if (text().isEmpty() && progr == 0)
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


void UrlBar::keyPressEvent(QKeyEvent *event)
{
    // this handles the Modifiers + Return key combinations
    QString currentText = text().trimmed();
    if (event->modifiers() == Qt::AltModifier)
    {
        if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)
        {
            activated(currentText, Rekonq::NewFocusedTab);
        }
    }
    if ((event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return)
            && !currentText.startsWith(QL1S("http://"), Qt::CaseInsensitive)
            && event->modifiers() != Qt::NoModifier)
    {
        QString append;
        if (event->modifiers() == Qt::ControlModifier)
        {
            append = QL1S(".com");
        }
        else if (event->modifiers() == (Qt::ControlModifier | Qt::ShiftModifier))
        {
            append = QL1S(".org");
        }
        else if (event->modifiers() == Qt::ShiftModifier)
        {
            append = QL1S(".net");
        }

        if (!append.isEmpty())
        {
            QUrl url(QL1S("http://www.") + currentText);
            QString host = url.host();
            if (!host.endsWith(append, Qt::CaseInsensitive))
            {
                host += append;
                url.setHost(host);
            }

            // now, load it!
            activated(url);
        }
    }

    else if ((event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return)
             && !currentText.isEmpty())
    {
        loadTyped(currentText);
    }

    else if (event->key() == Qt::Key_Escape)
    {
        clearFocus();
        if (text() != rApp->mainWindow()->currentTab()->view()->url().toString()
                && !rApp->mainWindow()->currentTab()->view()->url().toString().startsWith("about"))
            setText(rApp->mainWindow()->currentTab()->view()->url().toString());
        event->accept();
    }

    KLineEdit::keyPressEvent(event);
}


void UrlBar::focusInEvent(QFocusEvent *event)
{
    activateSuggestions(true);

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
            activated(text());
            return;
        }
    }

    // handles everything else
    KLineEdit::dropEvent(event);
    activated(text());
}


void UrlBar::loadFinished()
{
    if (_tab->url().scheme() == QL1S("about"))
    {
        update();
        return;
    }

    // show Favorite Icon
    if (ReKonfig::previewUrls().contains(_tab->url().url()))
    {
        IconButton *bt = addRightIcon(UrlBar::Favorite);
        connect(bt, SIGNAL(clicked(QPoint)), this, SLOT(showFavoriteDialog(QPoint)));
    }

    // show bookmark info
    IconButton *bt = addRightIcon(UrlBar::BK);
    connect(bt, SIGNAL(clicked(QPoint)), this, SLOT(showBookmarkInfo(QPoint)));

    // show KGet downloads??
    if (!KStandardDirs::findExe("kget").isNull() && ReKonfig::kgetList())
    {
        IconButton *bt = addRightIcon(UrlBar::KGet);
        connect(bt, SIGNAL(clicked(QPoint)), _tab->page(), SLOT(downloadAllContentsWithKGet(QPoint)));
    }

    // show RSS
    if (_tab->hasRSSInfo())
    {
        IconButton *bt = addRightIcon(UrlBar::RSS);
        connect(bt, SIGNAL(clicked(QPoint)), _tab, SLOT(showRSSInfo(QPoint)));
    }

    // show SSL
    if (_tab->url().scheme() == QL1S("https"))
    {
        // NOTE: the choice for the right SSL icon is done in the addRightIcon method
        IconButton *bt = addRightIcon(UrlBar::SSL);
        connect(bt, SIGNAL(clicked(QPoint)), _tab->page(), SLOT(showSSLInfo(QPoint)));
    }

    // show add search engine
    if (_tab->hasNewSearchEngine())
    {
        IconButton *bt = addRightIcon(UrlBar::SearchEngine);
        connect(bt, SIGNAL(clicked(QPoint)), _tab, SLOT(showSearchEngine(QPoint)));
    }

    // we need to update urlbar after the right icon settings
    // removing this code (where setStyleSheet automatically calls update) needs adding again
    // an update call
    int rightIconWidth = 25 * (_rightIconsList.count());
    setStyleSheet(QString("UrlBar { padding: 0 %2px 0 %1px;} ").arg(_icon->sizeHint().width()).arg(rightIconWidth));
}


void UrlBar::showBookmarkDialog()
{
    showBookmarkInfo(QCursor::pos());
}


void UrlBar::showBookmarkInfo(QPoint pos)
{
    if (_tab->url().scheme() == QL1S("about"))
        return;

    KBookmark bookmark = rApp->bookmarkProvider()->bookmarkForUrl(_tab->url());

    if (bookmark.isNull())
    {
        bookmark = rApp->bookmarkProvider()->bookmarkOwner()->bookmarkCurrentPage();
        updateRightIcons();
    }
    else
    {
        BookmarkWidget *widget = new BookmarkWidget(bookmark, window());
        connect(widget, SIGNAL(updateIcon()), this, SLOT(updateRightIcons()));
        widget->showAt(pos);
    }
}


void UrlBar::updateRightIcons()
{
    if (!_tab->isPageLoading())
    {
        clearRightIcons();
        loadFinished();
    }
}


void UrlBar::loadTyped(const QString &text)
{
    activated(KUrl(text));
}


void UrlBar::activateSuggestions(bool b)
{
    if (b)
    {
        if (_box.isNull())
        {
            _box = new CompletionWidget(this);
            installEventFilter(_box.data());
            connect(_box.data(), SIGNAL(chosenUrl(const KUrl &, Rekonq::OpenType)), this, SLOT(activated(const KUrl &, Rekonq::OpenType)));

            // activate suggestions on edit text
            connect(this, SIGNAL(textChanged(const QString &)), this, SLOT(detectTypedString(const QString &)));
        }
    }
    else
    {
        disconnect(this, SIGNAL(textChanged(const QString &)), this, SLOT(detectTypedString(const QString &)));
        removeEventFilter(_box.data());
        _box.data()->deleteLater();
    }
}


void UrlBar::mouseDoubleClickEvent(QMouseEvent *event)
{
    Q_UNUSED(event);

    selectAll();
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
    case UrlBar::SSL:
        _tab->page()->hasSslValid()
        ? rightIcon->setIcon(KIcon("object-locked"))
        : rightIcon->setIcon(KIcon("object-unlocked"));
        rightIcon->setToolTip(i18n("Show SSL Info"));
        break;
    case UrlBar::BK:
        if (rApp->bookmarkProvider()->bookmarkForUrl(_tab->url()).isNull())
        {
            rightIcon->setIcon(KIcon("bookmarks").pixmap(32, 32, QIcon::Disabled));
            rightIcon->setToolTip(i18n("Bookmark this page"));
        }
        else
        {
            rightIcon->setIcon(KIcon("bookmarks"));
            rightIcon->setToolTip(i18n("Edit this bookmark"));
        }
        rightIcon->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(rightIcon, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(bookmarkContextMenu(QPoint)));
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
    case UrlBar::Favorite:
        rightIcon->setIcon(KIcon("emblem-favorite"));
        rightIcon->setToolTip(i18n("Remove from favorite"));
        break;
    default:
        ASSERT_NOT_REACHED("ERROR.. default non extant case!!");
        break;
    }

    _rightIconsList << rightIcon;
    int iconsCount = _rightIconsList.count();
    int iconHeight = (height() - 18) / 2;
    rightIcon->move(width() - 23 * iconsCount, iconHeight);
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
    int newHeight = (height() - 18) / 2;
    _icon->move(4, newHeight);

    int iconsCount = _rightIconsList.count();
    int w = width();

    for (int i = 0; i < iconsCount; ++i)
    {
        IconButton *bt = _rightIconsList.at(i);
        bt->move(w - 25 * (i + 1), newHeight);
    }

    KLineEdit::resizeEvent(event);
}


void UrlBar::detectTypedString(const QString &typed)
{
    if (typed.count() == 1)
    {
        QTimer::singleShot(0, this, SLOT(suggest()));
        return;
    }

    if (_suggestionTimer->isActive())
        _suggestionTimer->stop();
    _suggestionTimer->start(50);
}


void UrlBar::suggest()
{
    if (!_box.isNull())
        _box.data()->suggestUrls(text());
}


void UrlBar::refreshFavicon()
{
    if (QWebSettings::globalSettings()->testAttribute(QWebSettings::PrivateBrowsingEnabled))
    {
        _icon->setIcon(KIcon("view-media-artist"));
        return;
    }

    KUrl u = _tab->url();
    if (u.scheme() == QL1S("about"))
    {
        _icon->setIcon(KIcon("arrow-right"));
        return;
    }
    _icon->setIcon(rApp->iconManager()->iconForUrl(u));
}


void UrlBar::showFavoriteDialog(QPoint pos)
{
    if (_tab->url().scheme() == QL1S("about"))
        return;

    IconButton *bt = qobject_cast<IconButton *>(this->sender());
    if (!bt)
        return;

    FavoriteWidget *widget = new FavoriteWidget(_tab, window());
    connect(widget, SIGNAL(updateIcon()), this, SLOT(updateRightIcons()));
    widget->showAt(pos);
}


void UrlBar::bookmarkContextMenu(QPoint pos)
{
    Q_UNUSED(pos);

    KMenu menu(this);
    QAction *qa;

    if (!rApp->bookmarkProvider()->bookmarkForUrl(_tab->url()).isNull())
    {
        qa = new KAction(KIcon("bookmarks"), i18n("Edit Bookmark"), this);
        connect(qa, SIGNAL(triggered(bool)), this, SLOT(showBookmarkDialog()));
        menu.addAction(qa);
    }

    if (!ReKonfig::previewUrls().contains(_tab->url().url()))
    {
        qa = new KAction(KIcon("emblem-favorite"), i18n("Add to favorite"), this);
        connect(qa, SIGNAL(triggered(bool)), this, SLOT(addFavorite()));
        menu.addAction(qa);
    }

    menu.exec(QCursor::pos());
}



void UrlBar::addFavorite()
{
    if (ReKonfig::previewUrls().contains(_tab->url().url()))
        return;

    QStringList urls = ReKonfig::previewUrls();
    urls << _tab->url().url();
    ReKonfig::setPreviewUrls(urls);

    QStringList titles = ReKonfig::previewNames();
    titles << _tab->view()->title();
    ReKonfig::setPreviewNames(titles);

    updateRightIcons();
}
