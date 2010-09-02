/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2009 by Andrea Diamantini <adjam7 at gmail dot com>
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
#include "listitem.h"
#include "listitem.moc"

// Auto Includes
#include "rekonq.h"

// Local Includes
#include "urlresolver.h"
#include "application.h"
#include "websnap.h"
#include "completionwidget.h"
#include "searchengine.h"
#include "iconmanager.h"

// KDE Includes
#include <KIcon>
#include <KAction>

// Qt Includes
#include <QActionGroup>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QSizePolicy>
#include <QPixmap>
#include <QStylePainter>
#include <QMouseEvent>
#include <QWebSettings>
#include <QFile>
#include <QTextDocument>

ListItem::ListItem(const UrlSearchItem &item, QWidget *parent)
        : QWidget(parent)
        , m_option()
        , m_url(item.url)
{    
    m_option.initFrom(this);
    m_option.direction = Qt::LeftToRight;   

    // use the same application palette (hence, the same colors)
    // Qt docs says that using this cctor is possible & fast (qt:qpalette)
    QPalette p(Application::palette());
    setPalette(p);

    deactivate();
}


ListItem::~ListItem()
{
    disconnect();
}



void ListItem::activate()
{
    m_option.state |= QStyle::State_Selected;
    update();
}


void ListItem::deactivate()
{
    m_option.state  &= ~QStyle::State_Selected;
    update();
}


void ListItem::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);   

    QWidget::paintEvent(event);
    QPainter painter(this);
    m_option.rect = QRect(QPoint(), size());
    painter.fillRect(m_option.rect, palette().brush(backgroundRole()));

    if (m_option.state.testFlag(QStyle::State_Selected) ||  m_option.state.testFlag(QStyle::State_MouseOver))
    {
        style()->drawPrimitive(QStyle::PE_PanelItemViewItem, &m_option, &painter, this);
    }

}


void ListItem::enterEvent(QEvent *e)
{
    m_option.state |= QStyle::State_MouseOver;
    update();
    QWidget::enterEvent(e);
}


void ListItem::leaveEvent(QEvent *e)
{
    m_option.state &= ~QStyle::State_MouseOver;
    update();
    QWidget::enterEvent(e);
}


void ListItem::mousePressEvent(QMouseEvent *e)
{
    emit itemClicked(this, e->button(), e->modifiers());
    QWidget::mousePressEvent(e);
}


KUrl ListItem::url()
{
    return m_url;
}


QString ListItem::text()
{
    return m_url.url();
}


void ListItem::nextItemSubChoice()
{
    //will be override
}


// ---------------------------------------------------------------


TypeIconLabel::TypeIconLabel(int type, QWidget *parent)
        : QLabel(parent)
{
    setMinimumWidth(40);
    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->setMargin(0);
    hLayout->setAlignment(Qt::AlignRight);
    setLayout(hLayout);

    if (type & UrlSearchItem::Search) hLayout->addWidget(getIcon("edit-find"));
    if (type & UrlSearchItem::Browse) hLayout->addWidget(getIcon("applications-internet"));
    if (type & UrlSearchItem::Bookmark) hLayout->addWidget(getIcon("rating"));
    if (type & UrlSearchItem::History) hLayout->addWidget(getIcon("view-history"));
    if (type & UrlSearchItem::Suggestion) hLayout->addWidget(getIcon("help-hint"));
}


QLabel *TypeIconLabel::getIcon(QString icon)
{
    QLabel *iconLabel = new QLabel(this);
    iconLabel->setFixedSize(16, 16);
    QPixmap pixmap = KIcon(icon).pixmap(16);
    iconLabel->setPixmap(pixmap);
    return iconLabel;
}


// ---------------------------------------------------------------


IconLabel::IconLabel(const QString &icon, QWidget *parent)
        : QLabel(parent)
{
    QPixmap pixmapIcon = Application::iconManager()->iconForUrl(KUrl(icon)).pixmap(16);
    setFixedSize(16, 16);
    setPixmap(pixmapIcon);
}


IconLabel::IconLabel(const KIcon &icon, QWidget *parent)
    : QLabel(parent)
{
    QPixmap pixmapIcon = icon.pixmap(16);
    setFixedSize(16, 16);
    setPixmap(pixmapIcon);
}

// ---------------------------------------------------------------


TextLabel::TextLabel(const QString &text, const QString &textToPointOut, QWidget *parent)
        : QLabel(parent)
{
    QString t = text;
    const bool wasItalic = t.startsWith("<i>");
    if (wasItalic)
        t.remove(QRegExp("<[/ib]*>"));

    t = Qt::escape(t);
    QString ss = Qt::escape(textToPointOut);
    t.replace(QRegExp('(' + ss + ')', Qt::CaseInsensitive), "<b>\\1</b>");

    if (wasItalic)
        t = QL1S("<i>") + t + QL1S("</i>");

    setText(t);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
}


//--------------------------------------------------------------------------------------------


PreviewListItem::PreviewListItem(const UrlSearchItem &item, const QString &text, QWidget *parent)
        : ListItem(item, parent)
{
    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->setSpacing(4);

    QLabel *previewLabelIcon = new QLabel(this);
    previewLabelIcon->setFixedSize(45, 33);
    new PreviewLabel(item.url, 38, 29, previewLabelIcon);
    IconLabel* icon = new IconLabel(item.url, previewLabelIcon);
    icon->move(27, 16);
    hLayout->addWidget(previewLabelIcon);

    QVBoxLayout *vLayout = new QVBoxLayout;
    vLayout->setMargin(0);

    QString title = item.title;
    if (title.isEmpty())
    {
        title = item.url;
        title = title.remove("http://");
        title.truncate(title.indexOf("/"));
    }

    vLayout->addWidget(new TextLabel(title, text, this));
    vLayout->addWidget(new TextLabel("<i>" + item.url + "</i>", text, this));

    hLayout->addLayout(vLayout);

    hLayout->addWidget(new TypeIconLabel(item.type, this));

    setLayout(hLayout);
}


// ---------------------------------------------------------------


PreviewLabel::PreviewLabel(const QString &url, int width, int height, QWidget *parent)
        : QLabel(parent)
{
    setFixedSize(width, height);
    setFrameStyle(QFrame::StyledPanel | QFrame::Raised);

    KUrl u = KUrl(url);
    if (WebSnap::existsImage(KUrl(u)))
    {
        QPixmap preview;
        preview.load(WebSnap::imagePathFromUrl(u));
        setPixmap(preview.scaled(width, height, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
    }
}


// ---------------------------------------------------------------


SearchListItem::SearchListItem(const UrlSearchItem &item, const QString &text, QWidget *parent)
        : ListItem(item, parent)
        , m_text(text)
{
    m_iconLabel = new IconLabel(SearchEngine::buildQuery(UrlResolver::searchEngine(), ""), this);
    QString query = SearchEngine::extractQuery(text);
    m_titleLabel = new TextLabel(searchItemTitle(item.title, query), query, this);
    m_engineBar = new EngineBar(UrlResolver::searchEngine(), parent);

    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->setSpacing(4);

    hLayout->addWidget(m_iconLabel);
    hLayout->addWidget(m_titleLabel);
    hLayout->addWidget(new QLabel(i18n("Engines: "), this));
    hLayout->addWidget(m_engineBar);
    hLayout->addWidget(new TypeIconLabel(item.type, this));

    setLayout(hLayout);

    connect(m_engineBar, SIGNAL(searchEngineChanged(KService::Ptr)), this, SLOT(changeSearchEngine(KService::Ptr)));
}


QString SearchListItem::text()
{
    return m_text;
}


QString SearchListItem::searchItemTitle(QString engine, QString text)
{
    return QString(i18nc("%1=search engine, e.g. Google, Wikipedia %2=text to search for", "Search %1 for %2", engine, text));
}


void SearchListItem::changeSearchEngine(KService::Ptr engine)
{
    m_titleLabel->setText(searchItemTitle(engine->name(), m_text));
    m_iconLabel->setPixmap( Application::iconManager()->iconForUrl(KUrl(engine->property("Query").toString())).pixmap(16) );
    m_url = SearchEngine::buildQuery(engine, m_text);
    UrlResolver::setSearchEngine(engine);
}


void SearchListItem::nextItemSubChoice()
{
    m_engineBar->selectNextEngine();
}


// -----------------------------------------------------------------------------------------------


EngineBar::EngineBar(KService::Ptr selectedEngine, QWidget *parent)
        : KToolBar(parent)
{
    setIconSize(QSize(16, 16));
    setToolButtonStyle(Qt::ToolButtonIconOnly);

    m_engineGroup = new QActionGroup(this);
    m_engineGroup->setExclusive(true);

    m_engineGroup->addAction(newEngineAction(SearchEngine::defaultEngine(), selectedEngine));
    foreach(KService::Ptr engine, SearchEngine::favorites())
    {
        if (engine->desktopEntryName() != SearchEngine::defaultEngine()->desktopEntryName())
        {
            m_engineGroup->addAction(newEngineAction(engine, selectedEngine));
        }
    }

    addActions(m_engineGroup->actions());
}


KAction *EngineBar::newEngineAction(KService::Ptr engine, KService::Ptr selectedEngine)
{
    QUrl u = engine->property("Query").toUrl();
    KUrl url = KUrl( u.toString( QUrl::RemovePath | QUrl::RemoveQuery ) );
    
    kDebug() << "Engine NAME: " << engine->name() << " URL: " << url;
    KAction *a = new KAction(Application::iconManager()->iconForUrl(url), engine->name(), this);
    a->setCheckable(true);
    if (engine->desktopEntryName() == selectedEngine->desktopEntryName()) a->setChecked(true);
    a->setData(engine->entryPath());
    connect(a, SIGNAL(triggered(bool)), this, SLOT(changeSearchEngine()));
    return a;
}


void EngineBar::changeSearchEngine()
{
    KAction *a = qobject_cast<KAction*>(sender());
    emit searchEngineChanged(KService::serviceByDesktopPath(a->data().toString()));
}


void EngineBar::selectNextEngine()
{
    QList<QAction *> e = m_engineGroup->actions();
    int i = 0;
    while (i < e.count() && !e.at(i)->isChecked())
    {
        i++;
    }

    if (i + 1 == e.count())
    {
        e.at(0)->setChecked(true);
        e.at(0)->trigger();
    }
    else
    {
        e.at(i + 1)->setChecked(true);
        e.at(i + 1)->trigger();
    }
}


// ---------------------------------------------------------------


SuggestionListItem::SuggestionListItem(const UrlSearchItem &item, const QString &text, QWidget *parent)
        : ListItem(item, parent)
        , m_text(item.title)
{
    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->setSpacing(4);

    hLayout->addWidget(new IconLabel(SearchEngine::buildQuery(UrlResolver::searchEngine(), ""), this));
    hLayout->addWidget(new TextLabel(item.title, SearchEngine::extractQuery(text), this));
    hLayout->addWidget(new TypeIconLabel(item.type, this));

    setLayout(hLayout);
}


QString SuggestionListItem::text()
{
    return m_text;
}

// ---------------------------------------------------------------


BrowseListItem::BrowseListItem(const UrlSearchItem &item, const QString &text, QWidget *parent)
        : ListItem(item, parent)
{
    QString url = text;

    kDebug() << text;

    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->setSpacing(4);

    hLayout->addWidget(new IconLabel(item.url, this));
    hLayout->addWidget(new TextLabel(item.url, text, this));
    hLayout->addWidget(new TypeIconLabel(item.type, this));

    setLayout(hLayout);
}


// ---------------------------------------------------------------


ListItem *ListItemFactory::create(const UrlSearchItem &item, const QString &text, QWidget *parent)
{
    ListItem *newItem;

    if (item.type & UrlSearchItem::Browse)
    {
        newItem = new BrowseListItem(item, text, parent);
    }
    else
    {
        if (item.type & UrlSearchItem::Search)
        {
            newItem = new SearchListItem(item, text, parent);
        }
        else
        {

            if (item.type & UrlSearchItem::Suggestion)
            {
                newItem = new SuggestionListItem(item, text, parent);
            }
            else
            {
                newItem = new PreviewListItem(item, text, parent);
            }
        }
    }

    return newItem;
}
