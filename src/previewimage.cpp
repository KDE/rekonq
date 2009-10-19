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
#include "previewimage.h"
#include "previewimage.moc"

// Local Includes
#include "application.h"
#include "history.h"
#include "rekonq.h"
#include "mainwindow.h"
#include "mainview.h"

// KDE Includes
#include <KUrl>
#include <KStandardDirs>
#include <KDebug>
#include <KMenu>
#include <KAction>
#include <KLocale>

// Qt Includes
#include <QFile>
#include <QMovie>
#include <QMouseEvent>
#include <QHBoxLayout>


PreviewImage::PreviewImage(const QUrl &url, int index, bool isFavorite)
    : QLabel()
    , ws(0)
    , m_url(0)
    , loadingSnapshot(false)
    , m_isFavorite(isFavorite)
    , m_index(index)
    , m_button(0)
{
    loadUrlPreview(url);
    setAlignment(Qt::AlignCenter);
}


PreviewImage::~PreviewImage()
{
    delete ws;
}



void PreviewImage::loadUrlPreview(const QUrl& url)
{
    m_url = url;

    if(url.isEmpty())
    {
        showEmptyPreview();
        return;
    }

    m_savePath = KStandardDirs::locateLocal("cache", QString("thumbs/") + guessNameFromUrl(m_url) + ".png", true);

    if(QFile::exists(m_savePath))
    {
        m_pixmap.load(m_savePath);
        setPixmap(m_pixmap);
    }
    else
    {
        loadingSnapshot = true;
        ws = new WebSnap( url );
        connect(ws, SIGNAL(finished()), this, SLOT(snapFinished()));

        QString path = KStandardDirs::locate("appdata", "pics/busywidget.gif");

        // load an animation waiting for site preview
        QMovie *movie = new QMovie(path, QByteArray(), this);
        movie->setSpeed(50);
        setMovie(movie);
        movie->start();
    }
}


void PreviewImage::snapFinished()
{
    loadingSnapshot = false;
    QMovie *m = movie();
    delete m;
    setMovie(0);

    m_pixmap = ws->previewImage();
    setPixmap(m_pixmap);

    m_pixmap.save(m_savePath);

    if(m_index > -1)
    {
        // Update title
        QStringList names = ReKonfig::previewNames();
        // update url (for added thumbs)
        QStringList urls = ReKonfig::previewUrls();

        // stripTrailingSlash to be sure to get the same string for same adress
        urls.replace(m_index, ws->snapUrl().toString(QUrl::StripTrailingSlash));
        names.replace(m_index, ws->snapTitle());

        ReKonfig::setPreviewNames(names);
        ReKonfig::setPreviewUrls(urls);

        ReKonfig::self()->writeConfig();
    }
}


void PreviewImage::showEmptyPreview()
{
    if(!m_isFavorite)
        return;

    clear();

    QHBoxLayout *layout = new QHBoxLayout(this);
    m_button = new QToolButton(this);
    m_button->setDefaultAction(historyMenu());
    m_button->setPopupMode(QToolButton::InstantPopup);
    m_button->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    m_button->setText(i18n("Add Preview"));
    m_button->setAutoRaise(true);
    m_button->setIconSize(QSize(48, 48));
    layout->addWidget(m_button);
}


void PreviewImage::mouseDoubleClickEvent(QMouseEvent *event)
{
    kDebug() << "no double click over here, thanks :D";
    Q_UNUSED(event);
}


void PreviewImage::mouseMoveEvent(QMouseEvent *event)
{
    kDebug() << "moving mouse over preview image";
    Q_UNUSED(event)
}


void PreviewImage::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
       // if(m_isFavorite)
       // {
            Application::instance()->loadUrl(m_url);

            //TODO: what is that ? :
       /* }
        else
        {
            MainView *mv = Application::instance()->mainWindow()->mainView();
            int actualIndex = mv->currentIndex();

            kDebug() << "Actual index: " << actualIndex;
            kDebug() << "m_index: " << m_index;

            mv->slotCloseTab(actualIndex);
            mv->setCurrentIndex(m_index);
        }
        */
        return;
    };
    QLabel::mousePressEvent(event);
}


void PreviewImage::mouseReleaseEvent(QMouseEvent *event)
{
    kDebug() << "NO000... don't leave your finger from the button!!";
    Q_UNUSED(event)
}


void PreviewImage::contextMenuEvent(QContextMenuEvent* event)
{
    if(!m_isFavorite)
        return;

    if(loadingSnapshot)
       return;

    KMenu menu(this);
    KAction *a;

    if(!m_url.isEmpty())
    {
        a = new KAction(KIcon("edit-delete"), i18n("Remove Thumbnail"), this);
        connect(a, SIGNAL(triggered(bool)), this, SLOT(removeMe()));
        menu.addAction(a);

        a = new KAction(KIcon("view-refresh"), i18n("Refresh Thumbnail"), &menu);
        connect(a, SIGNAL(triggered(bool)), this, SLOT(refreshPreview()));
        menu.addAction(a);
    }
    menu.addAction(historyMenu());

    menu.exec(mapToGlobal(event->pos()));
}


KActionMenu* PreviewImage::historyMenu()
{
    KActionMenu *histMenu = new KActionMenu(KIcon("insert-image"), i18n("Set page to preview"), this);
    QList<HistoryItem> history =  Application::historyManager()->history();

    if(history.isEmpty())
    {
        KAction *a = new KAction(i18n("History is empty"), this);
        a->setEnabled(false);
        histMenu->addAction(a);
        return histMenu;
    }

    int maxItems = 15;
    for (int i = 0; i < maxItems && i < history.size() ; ++i) 
    {
        HistoryItem it = history.at(i);
        KAction *a = new KAction(Application::icon(it.url), it.title, this);
        a->setData(it.url);
        connect(a, SIGNAL(triggered(bool)), this, SLOT(setUrlFromAction()));
        histMenu->addAction(a);
    }

    return histMenu;
}


void PreviewImage::removeMe()
{
    QStringList names = ReKonfig::previewNames();
    QStringList urls = ReKonfig::previewUrls();

    int index = urls.indexOf(QRegExp(m_url.toString(QUrl::StripTrailingSlash), Qt::CaseSensitive, QRegExp::FixedString));

    urls.replace(index, QString(""));
    names.replace(index, QString(""));

    ReKonfig::setPreviewNames(names);
    ReKonfig::setPreviewUrls(urls);

    // sync file data
    ReKonfig::self()->writeConfig();

    showEmptyPreview();

    m_url = "";
}


void PreviewImage::setUrlFromAction()
{
    KAction *a = qobject_cast<KAction*>(sender());
    KUrl url = KUrl(a->data().toString());

    if(m_button)
    {
        layout()->deleteLater();
        m_button->menu()->deleteLater();
        m_button->deleteLater();
    }
    loadUrlPreview(url);
}


void PreviewImage::refreshPreview()
{
    QString path = KStandardDirs::locateLocal("cache", QString("thumbs/") + guessNameFromUrl(m_url) + ".png", true);
    QFile::remove(path);
    loadUrlPreview(m_url);
}


QString PreviewImage::guessNameFromUrl(QUrl url)
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
