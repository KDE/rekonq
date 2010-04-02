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


UrlBar::UrlBar(QWidget *parent)
    : KComboBox(true, parent)
    , m_lineEdit(new LineEdit)
    , m_box(new CompletionWidget(this))
    , _tab(0)
    , _privateMode(false)
{
    //cosmetic
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setMinimumWidth(180);

    // signal handlings
    setTrapReturnKey(true);
    setUrlDropsEnabled(true);
    
    // Make m_lineEdit background transparent
    QPalette p = m_lineEdit->palette();
    p.setColor(QPalette::Base, Qt::transparent);
    m_lineEdit->setPalette(p);

    setLineEdit(m_lineEdit);

    // clear the URL bar
    m_lineEdit->clear();
    // load urls on activated urlbar signal
    connect(this, SIGNAL(returnPressed(const QString&)), SLOT(activated(const QString&)));
    
    installEventFilter(m_box);
    connect(m_box, SIGNAL(chosenUrl(const QString&, Rekonq::OpenType)), SLOT(activated(const QString&, Rekonq::OpenType)));
}


UrlBar::~UrlBar()
{
}


void UrlBar::selectAll() const
{
    m_lineEdit->selectAll();
}


KUrl UrlBar::url() const
{
    return m_currentUrl;
}


void UrlBar::setUrl(const QUrl& url)
{
    if(url.scheme() == "about")
    {
        m_currentUrl = KUrl();
        updateUrl();
        setFocus();
    }
    else
    {
        m_currentUrl = KUrl(url);
        updateUrl();
    }

}


void UrlBar::updateUrl()
{
    // Don't change my typed url...
    // FIXME this is not a proper solution (also if it works...)
    if(hasFocus())
    {
        kDebug() << "Don't change my typed url...";
        return;
    }

    KIcon icon;
    if(m_currentUrl.isEmpty()) 
    {
        icon = KIcon("arrow-right");
    }
    else 
    {
        icon = Application::icon(m_currentUrl);
    }

    if (count())
    {
        changeUrl(0, icon, m_currentUrl);
    }
    else
    {
        insertUrl(0, icon, m_currentUrl);
    }

    setCurrentIndex(0);

    // important security consideration: always display the beginning
    // of the url rather than its end to prevent spoofing attempts.
    // Must be AFTER setCurrentIndex
    if (!hasFocus())
    {
        m_lineEdit->setCursorPosition(0);
    }
}


void UrlBar::activated(const QString& urlString, Rekonq::OpenType type)
{
    disconnect(this, SIGNAL(editTextChanged(const QString &)), this, SLOT(suggestUrls(const QString &)));
    
    if (urlString.isEmpty())
        return;

    clearFocus();
    setUrl(urlString);
    Application::instance()->loadUrl(m_currentUrl, type);
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
    p.setColor(QPalette::Base, backgroundColor);
    setPalette(p);

    KComboBox::paintEvent(event);

    if (!hasFocus())
    {
        QPainter painter(this);

        QColor loadingColor;
        if (m_currentUrl.scheme() == QLatin1String("https"))
        {
            loadingColor = QColor(248, 248, 100);
        }
        else
        {
            loadingColor = QColor(116, 192, 250);
        }
        int progr = _tab->progress();
        
        backgroundColor.setAlpha(0);
        backgroundColor.setAlpha(200);
        QLinearGradient gradient(0, 0, width(), height() );
        gradient.setColorAt(0, loadingColor);
        gradient.setColorAt(((double)progr)/100, backgroundColor);
        
        painter.setBrush( gradient );
        painter.setPen(Qt::transparent);


        QRect backgroundRect = m_lineEdit->frameGeometry();
        int mid = backgroundRect.width() * progr / 100;
        QRect progressRect(backgroundRect.x(), backgroundRect.y(), mid, backgroundRect.height());
        painter.drawRect(progressRect);
        painter.end();
    }
}


QSize UrlBar::sizeHint() const
{
    return m_lineEdit->sizeHint();
}


void UrlBar::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Escape)
    {
        m_box->hide();
        return;
    }
    
    // this handles the Modifiers + Return key combinations
    QString currentText = m_lineEdit->text().trimmed();
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
            m_lineEdit->setText(url.toString());
        }
    }
    
    KComboBox::keyPressEvent(event);
}


void UrlBar::suggestUrls(const QString &text)
{   
    if (!hasFocus())
    {
        return;
    }

    if(text.isEmpty())
    {
        m_box->hide();
        return;
    }

    UrlResolver res(text);
    UrlSearchList list = res.orderedSearchItems();

    if(list.count() > 0)
    {
        m_box->clear();
        m_box->insertSearchList(list);
        m_box->popup();
    }
}

void UrlBar::focusInEvent(QFocusEvent *event)
{
    // activate suggestions on edit text
    connect(this, SIGNAL(editTextChanged(const QString &)), this, SLOT(suggestUrls(const QString &)));
    
    KComboBox::focusInEvent(event);
}


void UrlBar::setCurrentTab(WebTab *tab)
{
    if(_tab)
        disconnect(_tab->view(), SIGNAL(urlChanged(const QUrl &)), this, SLOT(setUrl(const QUrl &)));
    _tab = tab;
    connect(_tab->view(), SIGNAL(urlChanged(const QUrl &)), this, SLOT(setUrl(const QUrl &)));

    // update it now (the first time)
    setUrl( _tab->url() );
    update();
}


void UrlBar::setPrivateMode(bool on)
{
    _privateMode = on;
}
