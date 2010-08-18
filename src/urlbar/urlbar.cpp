/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2008-2009 by Andrea Diamantini <adjam7 at gmail dot com>
* Copyright (C) 2009 by Domrachev Alexandr <alexandr.domrachev@gmail.com>
* Copyright (C) 2009 by Paweł Prażak <pawelprazak at gmail dot com>
* Copyright (C) 2009 by Lionel Chauvin <megabigbug@yahoo.fr>
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
#include "bookmarksmanager.h"
#include "bookmarkwidget.h"

// KDE Includes
#include <KCompletionBox>

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
        , _privateMode(false)
        , _icon(new IconButton(this))
        , _suggestionTimer(new QTimer(this))
{
    // initial style
    setStyleSheet(QString("UrlBar { padding: 0 0 0 %1px;} ").arg(_icon->sizeHint().width()));

    // doesn't show the clear button
    setClearButtonShown(false);

    // trap Key_Enter & Key_Return events, while emitting the returnPressed signal
    setTrapReturnKey(true);

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

    // bookmark icon
    _icon->setIcon(KIcon("bookmarks").pixmap(32,32, QIcon::Disabled));
    connect(Application::bookmarkProvider()->bookmarkManager(), SIGNAL(changed(const QString &, const QString &)), this, SLOT(onBookmarksChanged()));
    connect(_icon, SIGNAL(clicked(const QPoint &)), this, SLOT(showBookmarkInfo(const QPoint &)));
    
    // load typed urls
    connect(this, SIGNAL(returnPressed(const QString &)), this, SLOT(loadTyped(const QString &)));

    _suggestionTimer->setSingleShot(true);
    connect(_suggestionTimer, SIGNAL(timeout()), this, SLOT(suggest()));
    
    activateSuggestions(true);
}


UrlBar::~UrlBar()
{
    activateSuggestions(false);
    delete _icon;
    _box.clear();
}


void UrlBar::setQUrl(const QUrl& url)
{
    if (url.scheme() == QL1S("about"))
    {
        _icon->setIcon(KIcon("arrow-right"));
        clear();
        setFocus();
    }
    else
    {
        clearFocus();
        KLineEdit::setUrl(url);
        setCursorPosition(0);
//         _icon->setIcon(Application::icon(url));
//         updateIcon();
    }
}


void UrlBar::activated(const KUrl& url, Rekonq::OpenType type)
{
    activateSuggestions(false);

    clearFocus();
    setUrl(url);
    Application::instance()->loadUrl(url, type);
}


void UrlBar::paintEvent(QPaintEvent *event)
{   
    QColor backgroundColor;
    QColor foregroundColor;
    
    if (_privateMode)
    {
        backgroundColor = QColor(220, 220, 220);  // light gray
        foregroundColor = Qt::black;
    }
    else
    {
        backgroundColor = Application::palette().color(QPalette::Base);
        foregroundColor = Application::palette().color(QPalette::Text);
    }

    // set background color of UrlBar
    QPalette p = palette();

    int progr = _tab->progress();
    if (progr == 0)
    {
        if (_tab->url().scheme() == QL1S("https"))
        {
            backgroundColor = QColor(255, 255, 171);  // light yellow
            foregroundColor = Qt::black;
        }
        p.setBrush(QPalette::Base, backgroundColor);
        p.setBrush(QPalette::Text, foregroundColor);
    }
    else
    {
        QColor highlight = Application::palette().color(QPalette::Highlight);

        int r = (highlight.red()+2*backgroundColor.red())/3;
        int g = (highlight.green()+2*backgroundColor.green())/3;
        int b = (highlight.blue()+2*backgroundColor.blue())/3;
        
        QColor loadingColor(r, g, b);

        if (abs(loadingColor.lightness() - backgroundColor.lightness()) < 20) //eg. Gaia color scheme
        {
            r = (2*highlight.red()+backgroundColor.red())/3;
            g = (2*highlight.green()+backgroundColor.green())/3;
            b = (2*highlight.blue()+backgroundColor.blue())/3;
            loadingColor = QColor(r, g, b);
        }

        QLinearGradient gradient( QPoint(0, 0), QPoint(width(), 0) );
        gradient.setColorAt(0, loadingColor);
        gradient.setColorAt(((double)progr) / 100 - .000001, loadingColor);
        gradient.setColorAt(((double)progr) / 100, backgroundColor);
        p.setBrush(QPalette::Base, gradient);
    }
    setPalette(p);

    // you need this before our code to draw inside the line edit..
    KLineEdit::paintEvent(event);

    if( text().isEmpty() && progr == 0 )
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
    if ((event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return)
            && !currentText.startsWith(QL1S("http://"), Qt::CaseInsensitive))
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

        QUrl url(QL1S("http://www.") + currentText);
        QString host = url.host();
        if (!host.endsWith(append, Qt::CaseInsensitive))
        {
            host += append;
            url.setHost(host);
            setText(url.toString());
        }
    }

    if (event->key() == Qt::Key_Escape)
    {
        clearFocus();
        event->accept();
    }

    KLineEdit::keyPressEvent(event);
}


void UrlBar::focusInEvent(QFocusEvent *event)
{
    activateSuggestions(true);

    KLineEdit::focusInEvent(event);
}


void UrlBar::setPrivateMode(bool on)
{
    _privateMode = on;
}


void UrlBar::dropEvent(QDropEvent *event)
{
    KLineEdit::dropEvent(event);
    activated(text());
}


void UrlBar::loadFinished()
{
    if (_tab->progress() != 0)
        return;

    if (_tab->url().scheme() == QL1S("about"))
    {
        update();
        return;
    }

    // setting bookmark icon
    if (Application::bookmarkProvider()->bookmarkForUrl(_tab->url()).isNull())
    {
        _icon->setIcon(KIcon("bookmarks").pixmap(32,32, QIcon::Disabled));
        _icon->setToolTip(i18n("Bookmark this page"));
    }
    else
    {
        _icon->setIcon(KIcon("bookmarks"));
        _icon->setToolTip(i18n("Edit this bookmark"));
    }
        
    // show KGet downloads??
    if (ReKonfig::kgetList())
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
        IconButton *bt = addRightIcon(UrlBar::SSL);
        connect(bt, SIGNAL(clicked(QPoint)), _tab->page(), SLOT(showSSLInfo(QPoint)));
    }

    // we need to update urlbar after the right icon settings
    // removing this code (where setStyleSheet automatically calls update) needs adding again 
    // an update call
    kDebug() << "resetting stylesheet";
    int rightIconWidth = 25 * (_rightIconsList.count());
    setStyleSheet(QString("UrlBar { padding: 0 %2px 0 %1px;} ").arg(_icon->sizeHint().width()).arg(rightIconWidth));
}


void UrlBar::showBookmarkInfo(const QPoint &pos)
{
    if( _tab->url().scheme() == QL1S("about") )
        return;
    
    KBookmark bookmark = Application::bookmarkProvider()->bookmarkForUrl(_tab->url());

    IconButton *bt = qobject_cast<IconButton *>(this->sender());
    if (!bt)
        return;

    if (bookmark.isNull())
    {
        bookmark = Application::bookmarkProvider()->rootGroup().addBookmark(_tab->view()->title(), _tab->url());
        Application::bookmarkProvider()->bookmarkManager()->emitChanged();
    }
    
    BookmarkWidget *widget = new BookmarkWidget(bookmark, window());
    widget->showAt(pos);
}


void UrlBar::onBookmarksChanged()
{
    clearRightIcons();
    loadFinished();
}


void UrlBar::loadTyped(const QString &text)
{
    activated( KUrl(text) );
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


void UrlBar::mouseDoubleClickEvent(QMouseEvent *)
{
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
        rightIcon->setIcon(KIcon("object-locked"));
        rightIcon->setToolTip(i18n("Show SSL Info"));
        break;
    default:
        kDebug() << "ERROR.. default non extant case!!";
        break;
    }

    _rightIconsList << rightIcon;
    int iconsCount = _rightIconsList.count();
    int iconHeight = (height() - 18) / 2;
    rightIcon->move(width() - 23*iconsCount, iconHeight);
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
        bt->move(w - 25*(i + 1), newHeight);
    }

    KLineEdit::resizeEvent(event);
}


void UrlBar::detectTypedString(const QString &typed)
{
    if(typed.count() == 1)
    {
        QTimer::singleShot(0, this, SLOT(suggest()));
        return;
    }
    
    if(_suggestionTimer->isActive())
        _suggestionTimer->stop();
    _suggestionTimer->start(50);
}


void UrlBar::suggest()
{
    if(!_box.isNull())
        _box.data()->suggestUrls( text() );
}
