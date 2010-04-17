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

// Local Includes
#include "urlresolver.h"
#include "application.h"

// KDE Includes
#include <KIcon>
#include <KStandardDirs>
#include <KDebug>
#include <QActionGroup>
#include <KConfigGroup>
#include <KIcon>

// Qt Includes
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QSizePolicy>
#include <QPixmap>
#include <QStylePainter>
#include <QMouseEvent>
#include <QWebSettings>
#include <QFile>


ListItem::ListItem(const UrlSearchItem &item, QWidget *parent)
    : QWidget(parent)
    , m_option()
    , m_url(item.url)
{
    setAutoFillBackground(true);

    m_option.initFrom(this);
    m_option.direction = Qt::LeftToRight;

    QPalette p(palette());
    p.setColor(QPalette::Base, Qt::white); // TODO: choose the correct color
    
    p.setColor(QPalette::AlternateBase, QColor(247,247,247)); // TODO: choose the correct color
    setPalette(p);

    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->setSpacing(4);
    setLayout(hLayout);

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
    
    if( m_option.state.testFlag(QStyle::State_Selected) ||  m_option.state.testFlag(QStyle::State_MouseOver))
    {
        QPainter painter(this);
        m_option.rect=QRect(QPoint(),size());
        style()->drawPrimitive(QStyle::PE_PanelItemViewItem, &m_option, &painter, this);
    }
    
    QWidget::paintEvent(event);
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
    emit itemClicked(this, e->button());
    QWidget::mousePressEvent(e);
}


KUrl ListItem::url()
{
    return m_url;
}

void ListItem::nextItemSubChoice()
{
    //will be override
}


// ---------------------------------------------------------------


TypeIcon::TypeIcon(int type, QWidget *parent)
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
}


QLabel *TypeIcon::getIcon(QString icon)
{
    QLabel *iconLabel = new QLabel;
    iconLabel->setFixedSize(16,16);  
    QPixmap pixmap = KIcon(icon).pixmap(16);
    iconLabel->setPixmap(pixmap);
    return iconLabel;
}


// ---------------------------------------------------------------


ItemIcon::ItemIcon(const QString &icon, QWidget *parent)
    : QLabel(parent)
{
    QPixmap pixmapIcon = KIcon(QWebSettings::iconForUrl(icon)).pixmap(16);
    if (pixmapIcon.isNull())
    {
        pixmapIcon = KIcon("text-html").pixmap(16);
    }

    setFixedSize(16,16);
    setPixmap(pixmapIcon);
}


// ---------------------------------------------------------------


ItemText::ItemText(const QString &text, const QString &textToPointOut, QWidget *parent)
    : QLabel(parent)
{
    QString t = text;
    if (!textToPointOut.isEmpty())
        t = t.replace(QRegExp("(" + textToPointOut + ")", Qt::CaseInsensitive), "<b>\\1</b>");

    setText(t);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
}


//--------------------------------------------------------------------------------------------


PreviewListItem::PreviewListItem(const UrlSearchItem &item, const QString &text, QWidget *parent)
    : ListItem(item, parent)
{
    QLabel *previewLabelIcon = new QLabel;
    previewLabelIcon->setFixedSize(45,33);
    new ItemPreview(item.url.url(), 38, 29, previewLabelIcon);
    ItemIcon* icon = new ItemIcon(item.url.url(), previewLabelIcon);
    icon->move(27, 16);
    layout()->addWidget(previewLabelIcon);
  
    QVBoxLayout *vLayout = new QVBoxLayout; 
    vLayout->setMargin(0);
    ((QHBoxLayout *)layout())->addLayout(vLayout);
    vLayout->addWidget(new ItemText(item.title, text));
    vLayout->addWidget(new ItemText("<i>" + item.url.url() + "</i>", text));
    layout()->addWidget(new TypeIcon(item.type));
}


// ---------------------------------------------------------------


ItemPreview::ItemPreview(const QString &url, int width, int height, QWidget *parent)
    : QLabel(parent)
{
    setFixedSize(width, height);
    setFrameStyle(QFrame::StyledPanel | QFrame::Raised);

    QString path = KStandardDirs::locateLocal("cache", QString("thumbs/") + guessNameFromUrl( QUrl(url) ) + ".png", true);
    if(QFile::exists(path))
    {     
        QPixmap preview;
        preview.load(path);
        setPixmap(preview.scaled(width, height, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
    }
}


//TODO: REMOVE DUPLICATE CODE WITH PREVIEWIMAGE
QString ItemPreview::guessNameFromUrl(QUrl url)
{
    QString name = url.toString( QUrl::RemoveScheme | QUrl::RemoveUserInfo | QUrl::StripTrailingSlash );
    
    // TODO learn Regular Expressions :)
    // and implement something better here..
    name.remove('/');
    name.remove('&');
    name.remove('.');
    name.remove('-');
    name.remove('_');
    name.remove('?');
    name.remove('=');
    name.remove('+');
    
    return name;
}


// ---------------------------------------------------------------


QString SearchListItem::m_currentEngine = "";


SearchListItem::SearchListItem(const UrlSearchItem &item, const QString &text, QWidget *parent)
    : ListItem(item, parent)
    , m_text(text)
{
    if (m_currentEngine == "") m_currentEngine = EngineBar::defaultEngine();
    
    m_iconLabel = new ItemIcon("edit-find", this); //TODO: get the default engine icon
    m_titleLabel = new ItemText(searchItemTitle(m_currentEngine, text));
    m_engineBar = new EngineBar(text, m_currentEngine, this);
    
    // without this it will not work :)
    m_url = m_engineBar->url();
    
    layout()->addWidget(m_iconLabel);
    layout()->addWidget(m_titleLabel);
    layout()->addWidget(new QLabel("Engines: "));
    layout()->addWidget(m_engineBar);
    layout()->addWidget(new TypeIcon(item.type));
    
    connect(m_engineBar, SIGNAL(searchEngineChanged(QString, QString)), this, SLOT(changeSearchEngine(QString, QString)));
}


QString SearchListItem::searchItemTitle(QString engine, QString text)
{
    return QString("Search "+ engine +" for <b>"+text+"</b>");
}


void SearchListItem::changeSearchEngine(QString url, QString engine)
{
    m_titleLabel->setText(searchItemTitle(engine,m_text));
    m_iconLabel->setPixmap(Application::icon( KUrl(url) ).pixmap(16));
    QString url2 = url.replace("\\{@}",m_text);
    m_url = KUrl(url2);
    m_currentEngine = engine;
}


void SearchListItem::nextItemSubChoice()
{
    m_engineBar->selectNextEngine();
}


EngineBar::EngineBar(const QString &text, const QString &selectedEngine, QWidget *parent)
    : KToolBar(parent)
{   
    setIconSize(QSize(16,16));
    setToolButtonStyle(Qt::ToolButtonIconOnly);
    
    m_engineGroup = new QActionGroup(this);
    m_engineGroup->setExclusive(true);
    
    KConfig config("kuriikwsfilterrc"); //Share with konqueror
    KConfigGroup cg = config.group("General");
    QStringList favoriteEngines;
    favoriteEngines << "wikipedia" << "google"; //defaults
    favoriteEngines = cg.readEntry("FavoriteSearchEngines", favoriteEngines);
    QString defaultEngine = cg.readEntry("DefaultSearchEngine", "google");
    KService::Ptr service;
    
    service = KService::serviceByDesktopPath(QString("searchproviders/%1.desktop").arg(defaultEngine));
    m_engineGroup->addAction(newEngineAction(service, selectedEngine));
    
    // set url;
    QString url = service->property("Query").toString();
    url = url.replace("\\{@}",text);
    m_url = KUrl(url);
    
    Q_FOREACH(const QString &engine, favoriteEngines)
    {
        if(!engine.isEmpty())
        {
            service = KService::serviceByDesktopPath(QString("searchproviders/%1.desktop").arg(engine));
            if(service && service->desktopEntryName()!=defaultEngine)
            {
                m_engineGroup->addAction(newEngineAction(service, selectedEngine));
            }
        }
    }
    
    addActions(m_engineGroup->actions());
}


QString EngineBar::defaultEngine()
{    
    KConfig config("kuriikwsfilterrc"); //Share with konqueror
    KConfigGroup cg = config.group("General");
    QString d = cg.readEntry("DefaultSearchEngine", "google");
    KService::Ptr service = KService::serviceByDesktopPath(QString("searchproviders/%1.desktop").arg(d));
    return service->name();
}


KAction *EngineBar::newEngineAction(KService::Ptr service, QString selectedEngine)
{
    KAction *a = new KAction(Application::icon(m_url), service->name(), this);
    a->setCheckable(true);
    if (service->name()==selectedEngine) a->setChecked(true);
    
    QString url = service->property("Query").toString();
    
    a->setData(QStringList() << url << service->name());
    connect(a, SIGNAL(triggered(bool)), this, SLOT(changeSearchEngine()));

    return a;
}


void EngineBar::changeSearchEngine()
{
    KAction *a = qobject_cast<KAction*>(sender());
    QStringList list = a->data().toStringList();    
    emit searchEngineChanged(list.first(), list.last());
}


void EngineBar::selectNextEngine()
{
     QList<QAction *> e = m_engineGroup->actions();
     int i = 0;
     while(i<e.count() && !e.at(i)->isChecked())
     {
         i++;
     }
     
     if (i+1 == e.count())
     {
         e.at(0)->setChecked(true);
         e.at(0)->trigger();
     }
     else
     {
         e.at(i+1)->setChecked(true);  
         e.at(i+1)->trigger();
     }
}


// ---------------------------------------------------------------


BrowseListItem::BrowseListItem(const UrlSearchItem &item, const QString &text, QWidget *parent)
    : ListItem(item, parent)
{
    QString url = text;
    layout()->addWidget(new ItemIcon(item.url.url()));
    layout()->addWidget(new ItemText("Browse <i>http://" + url.remove("http://") + "</i>", text));
    layout()->addWidget(new TypeIcon(item.type));
}


// ---------------------------------------------------------------


ListItem *ListItemFactory::create(const UrlSearchItem &item, const QString &text, QWidget *parent)
{
    ListItem *newItem;
    
    if (item.type & UrlSearchItem::Browse)
    {
        newItem = new BrowseListItem(item, text, parent);
    }
    else if (item.type & UrlSearchItem::Search)
    {
        newItem = new SearchListItem(item, text, parent);
    }
    else
    {
        newItem = new PreviewListItem(item, text, parent);
    }
    
    return newItem;
}
