/* ============================================================
 *
 * This file is a part of the reKonq project
 *
 * Copyright (C) 2008 by Andrea Diamantini <adjam7 at gmail dot com>
 *
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "searchbar.h"


#include <unistd.h>
#include <QLineEdit>
#include <QApplication>
#include <kaction.h>
#include <kconfig.h>
#include <ksharedconfig.h>
#include <kdebug.h>
#include <kdesktopfile.h>
#include <kgenericfactory.h>
#include <kglobal.h>
#include <khtml_part.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmimetype.h>
#include <kprotocolmanager.h>
#include <kstandarddirs.h>
#include <kurifilter.h>
#include <kservice.h>
#include <kactioncollection.h>

#include <qpainter.h>
#include <qmenu.h>
#include <qtimer.h>
#include <qstyle.h>
#include <QPixmap>
#include <QMouseEvent>




SearchBar::SearchBar() :
    m_searchCombo(0),
    m_searchMode(UseSearchProvider),
    m_urlEnterLock(false),
    m_process(0)
{
    m_searchCombo = new SearchBarCombo(0);
    m_searchCombo->lineEdit()->installEventFilter(this);
    connect(m_searchCombo, SIGNAL(activated(const QString &)), SLOT(startSearch(const QString &)));
    connect(m_searchCombo, SIGNAL(iconClicked()), SLOT(showSelectionMenu()));
    m_searchCombo->setWhatsThis(i18n("Search Bar<p>"
                                     "Enter a search term. Click on the icon to change search mode or provider.</p>"));

    m_popupMenu = 0;

    m_searchComboAction = actionCollection()->addAction("toolbar_search_bar");
    m_searchComboAction->setText(i18n("Search Bar"));
    m_searchComboAction->setDefaultWidget(m_searchCombo);
    m_searchComboAction->setShortcutConfigurable(false);


    KAction *a = actionCollection()->addAction("focus_search_bar");
    a->setText(i18n("Focus Searchbar"));
    a->setShortcut(Qt::CTRL+Qt::Key_S);
    connect(a, SIGNAL(triggered()), this, SLOT(focusSearchbar()));

    configurationChanged();

    // parent is the KonqMainWindow and we want to listen to PartActivateEvent events.
    parent->installEventFilter(this);
}



SearchBar::~SearchBar()
{
    KConfigGroup config(KGlobal::config(), "SearchBar");
    config.writeEntry("Mode", (int) m_searchMode);
    config.writeEntry("CurrentEngine", m_currentEngine);

    delete m_searchCombo;
    m_searchCombo = 0L;
    delete m_process;
    m_process=0L;
}

static QChar delimiter()
{
    static QChar s_delimiter = 0;
    if (s_delimiter == 0) {
        KConfig _config("kuriikwsfilterrc", KConfig::NoGlobals);
        KConfigGroup config(&_config, "General");
        s_delimiter = config.readEntry("KeywordDelimiter", int(':'));
    }
    return s_delimiter;
}



void SearchBar::nextSearchEntry()
{
    if (m_searchMode == FindInThisPage) {
        m_searchMode = UseSearchProvider;
        if (!m_searchEngines.isEmpty()) {
            m_currentEngine = m_searchEngines.first();
        } else {
            m_currentEngine = "google";
        }
    } else {
        int index = m_searchEngines.indexOf(m_currentEngine);
        ++index;
        if (index >= m_searchEngines.count()) {
            m_searchMode = FindInThisPage;
        } else {
            m_currentEngine = m_searchEngines.at(index);
        }
    }
    setIcon();
}

void SearchBar::previousSearchEntry()
{
    if (m_searchMode == FindInThisPage) {
        m_searchMode = UseSearchProvider;
        if (!m_searchEngines.isEmpty()) {
            m_currentEngine = m_searchEngines.last();
        } else {
            m_currentEngine = "google";
        }
    } else {
        int index = m_searchEngines.indexOf(m_currentEngine);
        if (index == 0) {
            m_searchMode = FindInThisPage;
        } else {
            --index;
            m_currentEngine = m_searchEngines.at(index);
        }
    }
    setIcon();
}

void SearchBar::startSearch(const QString &search)
{
    if ( m_urlEnterLock || search.isEmpty() )
        return;
   if (m_searchMode == UseSearchProvider) {
        m_urlEnterLock = true;
        KService::Ptr service;
        KUriFilterData data;
        QStringList list;
        list << "kurisearchfilter" << "kuriikwsfilter";

        service = KService::serviceByDesktopPath(QString("searchproviders/%1.desktop").arg(m_currentEngine));
        if (service) {
            const QString searchProviderPrefix = service->property("Keys").toStringList().first() + delimiter();
            data.setData(searchProviderPrefix + search);
        }

        if (!service || !KUriFilter::self()->filterUri(data, list)) {
            data.setData(QLatin1String("google") + delimiter() + search);
            KUriFilter::self()->filterUri(data, list);
        }

        if (QApplication::keyboardModifiers() & Qt::ControlModifier) {
            KParts::OpenUrlArguments arguments;
            KParts::BrowserArguments browserArguments;
            browserArguments.setNewTab(true);
            if (ext)
                emit ext->createNewWindow(data.uri(), arguments, browserArguments);
        } else {
            if (ext) {
                emit ext->openUrlRequest(data.uri());
                m_part->widget()->setFocus(); // #152923
            }
        }
    }

    m_searchCombo->addToHistory(search);
    m_searchCombo->setItemIcon(0, m_searchIcon);

    m_urlEnterLock = false;
}

void SearchBarPlugin::setIcon()
{
    if (m_searchMode == FindInThisPage) {
        m_searchIcon = SmallIcon("edit-find");
    } else {
        KService::Ptr service;
        KUriFilterData data;
        QStringList list;
        list << "kurisearchfilter" << "kuriikwsfilter";

        service = KService::serviceByDesktopPath(QString("searchproviders/%1.desktop").arg(m_currentEngine));
        if (service) {
            const QString searchProviderPrefix = service->property("Keys").toStringList().first() + delimiter();
            data.setData(searchProviderPrefix + "some keyword");
        }

        if (service && KUriFilter::self()->filterUri(data, list))
        {
            QString iconPath = KStandardDirs::locate("cache", KMimeType::favIconForUrl(data.uri()) + ".png");
            if (iconPath.isEmpty()) {
                m_searchIcon = SmallIcon("unknown");
            } else {
                m_searchIcon = QPixmap(iconPath);
            }
        }
        else
        {
            m_searchIcon = SmallIcon("google");
        }
    }

    // Create a bit wider icon with arrow
    QPixmap arrowmap = QPixmap(m_searchIcon.width()+5,m_searchIcon.height()+5);
    arrowmap.fill(m_searchCombo->lineEdit()->palette().color(m_searchCombo->lineEdit()->backgroundRole()));
    QPainter p(&arrowmap);
    p.drawPixmap(0, 2, m_searchIcon);
    QStyleOption opt;
    opt.state = QStyle::State_None;
    opt.rect = QRect(arrowmap.width()-6, arrowmap.height()-5, 6, 5);
    m_searchCombo->style()->drawPrimitive(QStyle::PE_IndicatorArrowDown, &opt, &p, m_searchCombo);
    p.end();
    m_searchIcon = arrowmap;

    m_searchCombo->setIcon(m_searchIcon);
}

void SearchBar::showSelectionMenu()
{
    if (!m_popupMenu) {
        KUriFilterData data;
        QStringList list;
        list << "kurisearchfilter" << "kuriikwsfilter";

        m_popupMenu = new QMenu(m_searchCombo);
        m_popupMenu->setObjectName("search selection menu");
        m_popupMenu->addAction(KIcon("edit-find"), i18n("Find in This Page"), this, SLOT(useFindInThisPage()));
        m_popupMenu->addSeparator();

        int i=-1;
        for (QStringList::ConstIterator it = m_searchEngines.begin(); it != m_searchEngines.end(); ++it) {
            i++;
            KService::Ptr service = KService::serviceByDesktopPath(QString("searchproviders/%1.desktop").arg(*it));
            if (!service) {
                continue;
            }
            const QString searchProviderPrefix = service->property("Keys").toStringList().first() + delimiter();
            data.setData(searchProviderPrefix + "some keyword");

            if (KUriFilter::self()->filterUri(data, list))
            {
                QIcon icon;
                QString iconPath = KStandardDirs::locate("cache", KMimeType::favIconForUrl(data.uri()) + ".png");
                if (iconPath.isEmpty()) {
                    icon = KIcon("unknown");
                } else {
                    icon = QPixmap(iconPath);
                }
                QAction* action = m_popupMenu->addAction(icon, service->name());
                action->setData(qVariantFromValue(i));
            }
        }

        m_popupMenu->addSeparator();
        m_popupMenu->addAction(KIcon("preferences-web-browser-shortcuts"), i18n("Select Search Engines..."),
                               this, SLOT(selectSearchEngines()));
        connect(m_popupMenu, SIGNAL(triggered(QAction *)), SLOT(useSearchProvider(QAction *)));
    }
    m_popupMenu->popup(m_searchCombo->mapToGlobal(QPoint(0, m_searchCombo->height() + 1)));
}

void SearchBar::useFindInThisPage()
{
    m_searchMode = FindInThisPage;
    setIcon();
}

void SearchBar::useSearchProvider(QAction *action)
{
    bool ok = false;
    const int id = action->data().toInt(&ok);
    if(!ok) {
        // Not a search engine entry selected
        return;
    }
    m_searchMode = UseSearchProvider;
    m_currentEngine = m_searchEngines.at(id);
    setIcon();
    m_searchCombo->lineEdit()->selectAll();
}

void SearchBar::selectSearchEngines()
{
    m_process = new KProcess;

    *m_process << "kcmshell4" << "ebrowsing";

    connect(m_process, SIGNAL(finished(int,QProcess::ExitStatus)), SLOT(searchEnginesSelected(int,QProcess::ExitStatus)));

    m_process->start();
    if(!m_process->waitForStarted())
    {
        kDebug(1202) << "Couldn't invoke kcmshell";
        delete m_process;
        m_process = 0;
    }
}

void SearchBar::searchEnginesSelected(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitCode);
    if(exitStatus == QProcess::NormalExit) {
        KConfigGroup config(KGlobal::config(), "SearchBar");
        config.writeEntry("CurrentEngine", m_currentEngine);
        config.sync();
        configurationChanged();
    }
    delete m_process;
    m_process = 0;
}

void SearchBar::configurationChanged()
{
    KConfigGroup config(KSharedConfig::openConfig("kuriikwsfilterrc"), "General");
    const QString engine = config.readEntry("DefaultSearchEngine", "google");

    QStringList favoriteEngines;
    favoriteEngines << "google" << "google_groups" << "google_news" << "webster" << "dmoz" << "wikipedia";
    favoriteEngines = config.readEntry("FavoriteSearchEngines", favoriteEngines);

    delete m_popupMenu;
    m_popupMenu = 0;
    m_searchEngines.clear();
    m_searchEngines << engine;
    for (QStringList::ConstIterator it = favoriteEngines.begin(); it != favoriteEngines.end(); ++it)
        if(*it!=engine)
            m_searchEngines << *it;

    if(engine.isEmpty()) {
        m_providerName = "Google";
    } else {
        KDesktopFile file("services", "searchproviders/" + engine + ".desktop");
        m_providerName = file.readName();
    }

    config = KConfigGroup(KGlobal::config(), "SearchBar");
    m_searchMode = (SearchModes) config.readEntry("Mode", (int) UseSearchProvider);
    m_currentEngine = config.readEntry("CurrentEngine", engine);

    if (m_currentEngine.isEmpty())
        m_currentEngine = "google";

    setIcon();
}



void SearchBar::updateComboVisibility()
{
    if (!m_part|| m_searchComboAction->associatedWidgets().isEmpty())
    {
        m_searchCombo->setPluginActive(false);
        m_searchCombo->hide();
    } else {
        m_searchCombo->setPluginActive(true);
        m_searchCombo->show();
    }
}



void SearchBar::focusSearchbar()
{
    m_searchCombo->setFocus(Qt::ShortcutFocusReason);
}



SearchBarCombo::SearchBarCombo(QWidget *parent) :
    KHistoryComboBox(parent),
    m_pluginActive(true)
{
    setDuplicatesEnabled(false);
    setFixedWidth(180);
    connect(this, SIGNAL(cleared()), SLOT(historyCleared()));

    Q_ASSERT(useCompletion());

    KConfigGroup config(KGlobal::config(), "SearchBar");
    QStringList list = config.readEntry( "History list", QStringList() );
    list.prepend(QString()); // empty item
    setHistoryItems(list, true);
    Q_ASSERT(currentText().isEmpty()); // KHistoryComboBox calls clearEditText
}

const QPixmap &SearchBarCombo::icon() const
{
    return m_icon;
}

void SearchBarCombo::setIcon(const QPixmap &icon)
{
    m_icon = icon;
    const QString editText = currentText();
    if (count() == 0) {
        insertItem(0, m_icon, 0);
    } else {
        for(int i = 0; i < count(); i++) {
            setItemIcon(i, m_icon);
        }
    }
    setEditText(editText);
}

int SearchBarCombo::findHistoryItem(const QString &searchText)
{
    for(int i = 0; i < count(); i++) {
        if (itemText(i) == searchText) {
            return i;
        }
    }

    return -1;
}

void SearchBarCombo::mousePressEvent(QMouseEvent *e)
{
    QStyleOptionComplex opt;
    int x0 = QStyle::visualRect(layoutDirection(), style()->subControlRect(QStyle::CC_ComboBox, &opt, QStyle::SC_ComboBoxEditField, this), rect()).x();

    if (e->x() > x0 + 2 && e->x() < lineEdit()->x()) {
        emit iconClicked();

        e->accept();
    } else {
        KHistoryComboBox::mousePressEvent(e);
    }
}

void SearchBarCombo::historyCleared()
{
    setIcon(m_icon);
}

void SearchBarCombo::setPluginActive(bool pluginActive)
{
    m_pluginActive = pluginActive;
}

void SearchBarCombo::show()
{
    if (m_pluginActive) {
        KHistoryComboBox::show();
    }
}

SearchBarCombo::~SearchBarCombo()
{
    KConfigGroup config(KGlobal::config(), "SearchBar");
    config.writeEntry( "History list", historyItems() );
}

#include "searchbar.moc"
