/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2009-2013 by Andrea Diamantini <adjam7 at gmail dot com>
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
#include "iconmanager.h"

#include "urlsuggester.h"
#include "completionwidget.h"

#include "websnap.h"
#include "searchengine.h"

// KDE Includes
#include <KIcon>
#include <KAction>
#include <kio/jobclasses.h>
#include <kio/scheduler.h>

// Qt Includes
#include <QApplication>
#include <QActionGroup>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSizePolicy>
#include <QPixmap>
#include <QStylePainter>
#include <QMouseEvent>
#include <QWebSettings>
#include <QFile>
#include <QTextDocument>
#include <QBitArray>


ListItem::ListItem(const UrlSuggestionItem &item, QWidget *parent)
    : QWidget(parent)
    , m_option()
    , m_url(item.url)
{
    m_option.initFrom(this);
    m_option.direction = Qt::LeftToRight;

    // use the same application palette (hence, the same colors)
    // Qt docs says that using this cctor is possible & fast (qt:qpalette)
    QPalette p(QApplication::palette());
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
    e->accept();
}


KUrl ListItem::url()
{
    return m_url;
}


QString ListItem::text()
{
    return m_url.prettyUrl();
}


void ListItem::nextItemSubChoice()
{
    // will be override
}


// ---------------------------------------------------------------


TypeIconLabel::TypeIconLabel(int type, QWidget *parent)
    : QLabel(parent)
{
    setMinimumWidth(16);
    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->setMargin(0);
    hLayout->setAlignment(Qt::AlignRight);
    setLayout(hLayout);

    if (type & UrlSuggestionItem::Search)
        hLayout->addWidget(getIcon("edit-find"));
    if (type & UrlSuggestionItem::Browse)
        hLayout->addWidget(getIcon("applications-internet"));
    if (type & UrlSuggestionItem::Bookmark)
        hLayout->addWidget(getIcon("rating"));
    if (type & UrlSuggestionItem::History)
        hLayout->addWidget(getIcon("view-history"));
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


static QString highlightWordsInText(const QString &text, const QStringList &words)
{
    QString ret = text;
    QBitArray boldSections(ret.size());
    Q_FOREACH(const QString & wordToPointOut, words)
    {
        int index = ret.indexOf(wordToPointOut, 0, Qt::CaseInsensitive);
        while (index > -1)
        {
            boldSections.fill(true, index, index + wordToPointOut.size());
            index = ret.indexOf(wordToPointOut, index + wordToPointOut.size(), Qt::CaseInsensitive);
        }
    }

    if (boldSections.isEmpty())
        return ret;

    int numSections = 0;
    for (int i = 0; i < boldSections.size() - 1; ++i)
    {
        if (boldSections.testBit(i) && !boldSections.testBit(i + 1))
            ++numSections;
    }
    if (boldSections.testBit(boldSections.size() - 1)) //last char was still part of a bold section
        ++numSections;
    const int tagLength = 7; // length of "<b>" and "</b>" we're going to add for each bold section.
    ret.reserve(ret.size() + numSections * tagLength);
    bool bold = false;
    for (int i = boldSections.size() - 1; i >= 0; --i)
    {
        if (!bold && boldSections.testBit(i))
        {
            ret.insert(i + 1, QL1S("</b>"));
            bold = true;
        }
        else if (bold && !boldSections.testBit(i))
        {
            ret.insert(i + 1, QL1S("<b>"));
            bold = false;
        }
    }
    if (bold)
        ret.insert(0, QL1S("<b>"));
    return ret;
}


TextLabel::TextLabel(const QString &text, const QString &textToPointOut, QWidget *parent)
    : QLabel(parent)
{
    setTextFormat(Qt::RichText);
    setMouseTracking(false);
    QString t = text;
    const bool wasItalic = t.startsWith(QL1S("<i>"));
    if (wasItalic)
        t.remove(QRegExp(QL1S("<[/ib]*>")));
    t = Qt::escape(t);
    QStringList words = Qt::escape(textToPointOut.simplified()).split(QL1C(' '));
    t = highlightWordsInText(t, words);
    if (wasItalic)
        t = QL1S("<i style=color:\"#555\">") + t + QL1S("</i>");
    setText(t);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
}


TextLabel::TextLabel(QWidget *parent)
    : QLabel(parent)
{
    setTextFormat(Qt::RichText);
    setMouseTracking(false);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
}


void TextLabel::setEngineText(const QString &engine, const QString &text)
{
    setText(i18nc("%1=search engine, e.g. Google, Wikipedia %2=text to search for", "Search %1 for <b>%2</b>", engine, Qt::escape(text)));
}


// ---------------------------------------------------------------


PreviewListItem::PreviewListItem(const UrlSuggestionItem &item, const QString &text, QWidget *parent)
    : ListItem(item, parent)
{
    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->setSpacing(4);

    // icon
    hLayout->addWidget(new TypeIconLabel(item.type, this));

    // url + text
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

    setLayout(hLayout);
}


// ---------------------------------------------------------------


SearchListItem::SearchListItem(const UrlSuggestionItem &item, const QString &text, QWidget *parent)
    : ListItem(item, parent)
    , m_text(text)
{
    m_titleLabel = new TextLabel(this);
    m_titleLabel->setEngineText(item.description, item.title);

    KService::Ptr engine = SearchEngine::fromString(text);
    if (!engine)
        engine = SearchEngine::defaultEngine();

    m_engineBar = new EngineBar(engine, parent);

    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->setSpacing(4);

    hLayout->addWidget(new TypeIconLabel(item.type, this));
    hLayout->addWidget(m_titleLabel);
    hLayout->addWidget(new QLabel(i18n("Engines:"), this));
    hLayout->addWidget(m_engineBar);

    setLayout(hLayout);

    connect(m_engineBar, SIGNAL(searchEngineChanged(KService::Ptr)), this, SLOT(changeSearchEngine(KService::Ptr)));
}


QString SearchListItem::text()
{
    return m_text;
}


void SearchListItem::changeSearchEngine(KService::Ptr engine)
{
    // NOTE: This to let rekonq loading text typed in the requested engine on click.
    // There probably is a better way to do it. I just cannot see it now...

    // remove the xx: part...
    QString separator = SearchEngine::delimiter();
    
    QString text = m_text.contains(separator)
        ? m_text.section(separator, 1, 1)
        : m_text;

    // create a new item && load it...
    UrlSuggestionItem item = UrlSuggestionItem(UrlSuggestionItem::Search, SearchEngine::buildQuery(engine, text), text);

    SearchListItem sItem(item, text, this);
    emit itemClicked(&sItem, Qt::LeftButton, Qt::NoModifier);
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

    if (SearchEngine::defaultEngine().isNull())
        return;
    
    m_engineGroup->addAction(newEngineAction(SearchEngine::defaultEngine(), selectedEngine));
    Q_FOREACH(const KService::Ptr & engine, SearchEngine::favorites())
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
    KUrl url = KUrl(u.toString(QUrl::RemovePath | QUrl::RemoveQuery));

    KAction *a = new KAction(IconManager::self()->engineFavicon(url), engine->name(), this);
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


BrowseListItem::BrowseListItem(const UrlSuggestionItem &item, const QString &text, QWidget *parent)
    : ListItem(item, parent)
{
    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->setSpacing(4);

    hLayout->addWidget(new TypeIconLabel(item.type, this));
    hLayout->addWidget(new TextLabel(item.url, text, this));

    setLayout(hLayout);
}


// ---------------------------------------------------------------


ListItem *ListItemFactory::create(const UrlSuggestionItem &item, const QString &text, QWidget *parent)
{
    if (item.type & UrlSuggestionItem::Search)
    {
        return new SearchListItem(item, text, parent);
    }

    if (item.type & UrlSuggestionItem::Browse)
    {
        return new BrowseListItem(item, text, parent);
    }

    if (item.type & UrlSuggestionItem::History)
    {
        return new PreviewListItem(item, text, parent);
    }

    if (item.type & UrlSuggestionItem::Bookmark)
    {
        return new PreviewListItem(item, text, parent);
    }

    return new PreviewListItem(item, text, parent);
}
