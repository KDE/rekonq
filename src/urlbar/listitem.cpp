/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2009-2011 by Andrea Diamantini <adjam7 at gmail dot com>
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
#include <kio/jobclasses.h>
#include <kio/scheduler.h>

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
    QPalette p(rApp->palette());
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

    if (type & UrlSearchItem::Search)
        hLayout->addWidget(getIcon("edit-find"));
    if (type & UrlSearchItem::Browse)
        hLayout->addWidget(getIcon("applications-internet"));
    if (type & UrlSearchItem::Bookmark)
        hLayout->addWidget(getIcon("rating"));
    if (type & UrlSearchItem::History)
        hLayout->addWidget(getIcon("view-history"));
    if (type & UrlSearchItem::Suggestion)
        hLayout->addWidget(getIcon("help-hint"));
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
    QPixmap pixmapIcon = rApp->iconManager()->iconForUrl(KUrl(icon)).pixmap(16);
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


static QString highlightWordsInText(const QString &text, const QStringList &words)
{
    QString ret = text;
    QBitArray boldSections(ret.size());
    foreach(const QString &wordToPointOut, words)
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
        t = QL1S("<i>") + t + QL1S("</i>");
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


DescriptionLabel::DescriptionLabel(const QString &text, QWidget *parent)
        : QLabel(parent)
{
    QString t = text;
    const bool wasItalic = t.startsWith(QL1S("<i>"));
    if (wasItalic)
        t.remove(QRegExp("<[/ib]*>"));

    if (wasItalic)
        t = QL1S("<i>") + t + QL1S("</i>");

    setWordWrap(false); //TODO: why setWordWrap(true) make items have a strange behavior ?
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


ImageLabel::ImageLabel(const QString &url, int width, int height, QWidget *parent)
        : QLabel(parent),
        m_url(url)
{
    setFixedSize(width, height);
    if (WebSnap::existsImage(KUrl(url)))
    {
        QPixmap pix;
        pix.load(WebSnap::imagePathFromUrl(url));
        setPixmap(pix);
    }
    else
    {
        KIO::TransferJob *job = KIO::get(KUrl(url), KIO::NoReload, KIO::HideProgressInfo);
        connect(job,  SIGNAL(data(KIO::Job *, const QByteArray &)),
                this, SLOT(slotData(KIO::Job*, const QByteArray&)));
        connect(job,  SIGNAL(result(KJob *)),
                this, SLOT(slotResult(KJob *)));
    }
}


void ImageLabel::slotData(KIO::Job *job, const QByteArray &data)
{
    Q_UNUSED(job);
    m_data.append(data);
}


void ImageLabel::slotResult(KJob *)
{
    QPixmap pix;
    if (!pix.loadFromData(m_data))
        kDebug() << "error while loading image: ";
    setPixmap(pix);
    pix.save(WebSnap::imagePathFromUrl(m_url), "PNG");
}


// ---------------------------------------------------------------


SearchListItem::SearchListItem(const UrlSearchItem &item, const QString &text, QWidget *parent)
        : ListItem(item, parent)
        , m_text(text)
{
    m_iconLabel = new IconLabel(SearchEngine::buildQuery(UrlResolver::searchEngine(), ""), this);
    m_titleLabel = new TextLabel(this);
    m_titleLabel->setEngineText(UrlResolver::searchEngine()->name(), item.title);
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


void SearchListItem::changeSearchEngine(KService::Ptr engine)
{
    UrlResolver::setSearchEngine(engine);
    emit updateList();
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
    foreach(const KService::Ptr &engine, SearchEngine::favorites())
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

    kDebug() << "Engine NAME: " << engine->name() << " URL: " << url;
    KAction *a = new KAction(rApp->iconManager()->iconForUrl(url), engine->name(), this);
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

    hLayout->addWidget(new IconLabel(item.url, this));
    hLayout->addWidget(new TextLabel(item.title, text, this));
    hLayout->addWidget(new TypeIconLabel(item.type, this));

    setLayout(hLayout);
}


QString SuggestionListItem::text()
{
    return m_text;
}


// ---------------------------------------------------------------


VisualSuggestionListItem::VisualSuggestionListItem(const UrlSearchItem &item, const QString &text, QWidget *parent)
        : ListItem(item, parent)
        , m_text(item.title)
{

    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->setSpacing(4);
    QLabel *previewLabelIcon = new QLabel(this);

    if (!item.image.isEmpty())
    {
        previewLabelIcon->setFixedSize(item.image_width + 10, item.image_height + 10);
        new ImageLabel(item.image, item.image_width, item.image_height, previewLabelIcon);
        IconLabel* icon = new IconLabel(item.url, previewLabelIcon);
        icon->move(item.image_width - 10,  item.image_height - 10);
    }
    else
    {
        previewLabelIcon->setFixedSize(18, 18);
        new IconLabel(item.url, previewLabelIcon);
    }

    hLayout->addWidget(previewLabelIcon);
    QVBoxLayout *vLayout = new QVBoxLayout;
    vLayout->setMargin(0);
    vLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::MinimumExpanding));
    vLayout->addWidget(new TextLabel(item.title, text, this));
    DescriptionLabel *d = new DescriptionLabel("", this);
    vLayout->addWidget(d);
    vLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::MinimumExpanding));
    hLayout->addLayout(vLayout);
    hLayout->addWidget(new TypeIconLabel(item.type, this));
    setLayout(hLayout);
    d->setText("<i>" + item.description + "</i>");
}


QString VisualSuggestionListItem::text()
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
    if (item.type & UrlSearchItem::Search)
    {
        kDebug() << "Search";
        return new SearchListItem(item, text, parent);
    }

    if (item.type & UrlSearchItem::Browse)
    {
        kDebug() << "Browse";
        return new BrowseListItem(item, text, parent);
    }

    if (item.type & UrlSearchItem::History)
    {
        kDebug() << "History";
        return new PreviewListItem(item, text, parent);
    }

    if (item.type & UrlSearchItem::Bookmark)
    {
        kDebug() << "Bookmark";
        return new PreviewListItem(item, text, parent);
    }

    if (item.type & UrlSearchItem::Suggestion)
    {
        kDebug() << "ITEM URL: " << item.url;
        if (item.description.isEmpty())
        {
            kDebug() << "Suggestion";
            return new SuggestionListItem(item, text, parent);
        }

        kDebug() << "Visual Suggestion";
        return new VisualSuggestionListItem(item, text, parent);
    }

    kDebug() << "Undefined";
    return new PreviewListItem(item, text, parent);
}
