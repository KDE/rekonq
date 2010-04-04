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

// Local Includes
#include "application.h"
#include "lineedit.h"
#include "mainwindow.h"
#include "webview.h"
#include "urlresolver.h"

// KDE Includes
#include <KDebug>
#include <KCompletionBox>
#include <KUrl>

// Qt Includes
#include <QPainter>
#include <QPaintEvent>
#include <QPalette>
#include <QTimer>
#include <QVBoxLayout>

// Defines
#define QL1S(x)  QLatin1String(x)


UrlBar::UrlBar(QWidget *parent)
    : LineEdit(parent)
    , _box(new CompletionWidget(this))
    , _tab(0)
    , _privateMode(false)
{
    // load urls on activated urlbar signal
    connect(this, SIGNAL(returnPressed(const QString&)), this, SLOT(activated(const QString&)));

    // suggestions
    installEventFilter(_box);
    connect(_box, SIGNAL(chosenUrl(const QString&, Rekonq::OpenType)), SLOT(activated(const QString&, Rekonq::OpenType)));
}


UrlBar::~UrlBar()
{
    delete _box;
}


void UrlBar::setQUrl(const QUrl& url)
{
    if(url.scheme() == QL1S("about") )
    {
        iconButton()->updateIcon( KIcon("arrow-right") );
        setFocus();
    }
    else
    {
        LineEdit::setUrl(url);
        setCursorPosition(0);
        iconButton()->updateIcon( Application::icon(url) );
    }

    updateStyles();
}


void UrlBar::activated(const QString& urlString, Rekonq::OpenType type)
{
    disconnect(this, SIGNAL(textChanged(const QString &)), this, SLOT(suggestUrls(const QString &)));
    
    if (urlString.isEmpty())
        return;

    clearFocus();
    setText(urlString);
    Application::instance()->loadUrl(urlString, type);
}


void UrlBar::paintEvent(QPaintEvent *event)
{
    QColor backgroundColor;
    if( _privateMode )
    {
        backgroundColor = QColor(192, 192, 192);  // gray
    }
    else
    {
        backgroundColor = Application::palette().color(QPalette::Base);
    }
    
    // set background color of UrlBar
    QPalette p = palette();

    int progr = _tab->progress();
    if (progr == 0) 
    {
        p.setBrush(QPalette::Base, backgroundColor);
    } 
    else 
    {
        QColor loadingColor;
        if ( _tab->url().scheme() == QLatin1String("https"))
        {
            loadingColor = QColor(248, 248, 100);
        }
        else
        {
            loadingColor = QColor(116, 192, 250);
        }
    
    
        QLinearGradient gradient(0, 0, width(), 0);
        gradient.setColorAt(0, loadingColor);
        gradient.setColorAt(((double)progr)/100, backgroundColor);
        p.setBrush(QPalette::Base, gradient);
    }
    setPalette(p);
    
    LineEdit::paintEvent(event);
}


void UrlBar::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Escape)
    {
        _box->hide();
        return;
    }
    
    // this handles the Modifiers + Return key combinations
    QString currentText = text().trimmed();
    if ((event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return)
        && !currentText.startsWith(QLatin1String("http://"), Qt::CaseInsensitive))
    {
        QString append;
        if (event->modifiers() == Qt::ControlModifier)
        {
            append = QLatin1String(".com");
        }
        else if (event->modifiers() == (Qt::ControlModifier | Qt::ShiftModifier))
        {
            append = QLatin1String(".org");
        }
        else if (event->modifiers() == Qt::ShiftModifier)
        {
            append = QLatin1String(".net");
        }

        QUrl url(QLatin1String("http://www.") + currentText);
        QString host = url.host();
        if (!host.endsWith(append, Qt::CaseInsensitive))
        {
            host += append;
            url.setHost(host);
            setText(url.toString());
        }
    }
    
    LineEdit::keyPressEvent(event);
}


void UrlBar::suggestUrls(const QString &text)
{   
    if (!hasFocus())
    {
        return;
    }

    if(text.isEmpty())
    {
        _box->hide();
        return;
    }

    UrlResolver res(text);
    UrlSearchList list = res.orderedSearchItems();

    if(list.count() > 0)
    {
        _box->clear();
        _box->insertSearchList(list);
        _box->popup();
    }
}


void UrlBar::focusInEvent(QFocusEvent *event)
{
    // activate suggestions on edit text
    connect(this, SIGNAL(textChanged(const QString &)), this, SLOT(suggestUrls(const QString &)));
    
    LineEdit::focusInEvent(event);
}


void UrlBar::setCurrentTab(WebTab *tab)
{
    if(_tab)
    {
        disconnect(_tab->view(), SIGNAL(urlChanged(const QUrl &)), this, SLOT(setQUrl(const QUrl &)));
        disconnect(_tab->view(), SIGNAL(loadFinished(bool)), this, SLOT(loadFinished()));
        disconnect(_tab->page(), SIGNAL(validSSLInfo(bool)), this, SLOT(setTrustedHost(bool)));
        disconnect(iconButton(), SIGNAL(clicked()), _tab->page(), SLOT(showSSLInfo()));
    }
    _tab = tab;
    connect(_tab->view(), SIGNAL(urlChanged(const QUrl &)), this, SLOT(setQUrl(const QUrl &)));
    connect(_tab->view(), SIGNAL(loadFinished(bool)), this, SLOT(loadFinished()));
    connect(_tab->page(), SIGNAL(validSSLInfo(bool)), this, SLOT(setTrustedHost(bool)));
    connect(iconButton(), SIGNAL(clicked()), _tab->page(), SLOT(showSSLInfo()));
            
    // update it now (the first time)
    updateStyles();
    _tab->view()->setFocus();
    setQUrl( _tab->url() );
}


void UrlBar::setPrivateMode(bool on)
{
    _privateMode = on;
}


void UrlBar::loadFinished()
{
    // show RSS
    
    // show KGet downloads??
    
    // last, but not least
    updateStyles();
}


void UrlBar::setTrustedHost(bool on)
{
    kDebug() << "SET TRUSTED HOST..";
    iconButton()->setIconUrl( _tab->url() , on );
}
