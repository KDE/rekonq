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
#include "historymodels.h"
#include "lineedit.h"
#include "mainwindow.h"
#include "webview.h"

// KDE Includes
#include <KDebug>
#include <KCompletionBox>
#include <KUrl>

// Qt Includes
#include <QtGui/QPainter>
#include <QtGui/QPaintEvent>
#include <QtGui/QPalette>


QColor UrlBar::s_defaultBaseColor;


UrlBar::UrlBar(QWidget *parent)
        : KHistoryComboBox(true, parent)
        , m_lineEdit(new LineEdit)
        , m_progress(0)
{
    setUrlDropsEnabled(true);
    setAutoDeleteCompletionObject(true);

    //cosmetic
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setMinimumWidth(180);
    
    setTrapReturnKey(true);

    setupLineEdit();

    // add every item to history
    connect(this, SIGNAL(returnPressed(const QString&)), SLOT(slotActivated(const QString&)));
    connect(completionBox(), SIGNAL(activated(const QString&)), SLOT(slotActivated(const QString&)));

    connect(this, SIGNAL(cleared()), SLOT(slotCleared()));

    // setup completion box
    setCompletionObject( Application::historyManager()->completionObject() );
    
    // set dropdown list background
    QPalette p = view()->palette();
    p.setColor(QPalette::Base, palette().color(QPalette::Base));
    view()->setPalette(p);
}


UrlBar::~UrlBar()
{
}


void UrlBar::selectAll() const
{
    lineEdit()->selectAll();
}


KUrl UrlBar::url() const
{
    return m_currentUrl;
}


KLineEdit *UrlBar::lineEdit() const
{
    return m_lineEdit;
}


void UrlBar::setupLineEdit()
{
    // Make m_lineEdit background transparent
    QPalette p = m_lineEdit->palette();
    p.setColor(QPalette::Base, Qt::transparent);
    m_lineEdit->setPalette(p);

    if (!s_defaultBaseColor.isValid())
    {
        s_defaultBaseColor = palette().color(QPalette::Base);
    }

    setLineEdit(m_lineEdit);

    // Make the lineedit consume the Qt::Key_Enter event...
    lineEdit()->setTrapReturnKey(true);

    lineEdit()->setHandleSignals(true);

    // clear the URL bar
    lineEdit()->clear();
}


void UrlBar::setUrl(const QUrl& url)
{
    if(url.scheme() == "rekonq")
    {
        setFocus();
        return;
    }
    m_currentUrl = url;
    slotUpdateUrl();
}


void UrlBar::setProgress(int progress)
{
    m_progress = progress;
    repaint();
}


void UrlBar::slotUpdateUrl()
{
    if (count())
    {
        changeUrl(0, Application::icon(m_currentUrl), m_currentUrl);
    }
    else
    {
        insertUrl(0, Application::icon(m_currentUrl), m_currentUrl);
    }

    setCurrentIndex(0);

    // important security consideration: always display the beginning
    // of the url rather than its end to prevent spoofing attempts.
    // Must be AFTER setCurrentIndex
    if (!hasFocus())
    {
        lineEdit()->setCursorPosition(0);
    }
}


void UrlBar::slotActivated(const QString& url)
{
    if (url.isEmpty())
        return;

    setUrl(url);

    Application::historyManager()->addHistoryEntry(url);

    emit activated(m_currentUrl);
}


void UrlBar::slotCleared()
{
    // clear the history on user's request from context menu
    clear();
    Application::historyManager()->clear();
}


void UrlBar::slotLoadFinished(bool)
{
    // reset progress bar after small delay
    m_progress = 0;
    QTimer::singleShot(200, this, SLOT(repaint()));
}


void UrlBar::slotUpdateProgress(int progress)
{
    m_progress = progress;
    repaint();
}


void UrlBar::paintEvent(QPaintEvent *event)
{
    // set background color of UrlBar
    QPalette p = palette();
    p.setColor(QPalette::Base, s_defaultBaseColor);
    setPalette(p);

    KHistoryComboBox::paintEvent(event);

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
        painter.setBrush(generateGradient(loadingColor, height()));
        painter.setPen(Qt::transparent);

        QRect backgroundRect = lineEdit()->frameGeometry();
        int mid = backgroundRect.width() * m_progress / 100;
        QRect progressRect(backgroundRect.x(), backgroundRect.y(), mid, backgroundRect.height());
        painter.drawRect(progressRect);
        painter.end();
    }
}


void UrlBar::focusOutEvent(QFocusEvent *event)
{
    // set back last loaded url in case user cleared it
    if (!m_currentUrl.equals(KUrl(lineEdit()->text()))) setUrl(m_currentUrl);

    KHistoryComboBox::focusOutEvent(event);
}


QSize UrlBar::sizeHint() const
{
    return lineEdit()->sizeHint();
}


QLinearGradient UrlBar::generateGradient(const QColor &color, int height)
{
    QColor base = s_defaultBaseColor;
    base.setAlpha(0);
    QColor barColor = color;
    barColor.setAlpha(200);
    QLinearGradient gradient(0, 0, 0, height);
    gradient.setColorAt(0, base);
    gradient.setColorAt(0.25, barColor.lighter(120));
    gradient.setColorAt(0.5, barColor);
    gradient.setColorAt(0.75, barColor.lighter(120));
    gradient.setColorAt(1, base);
    return gradient;
}


void UrlBar::setBackgroundColor(QColor c)
{
    s_defaultBaseColor=c;
    repaint();
}


bool UrlBar::isLoading()
{
    if(m_progress == 0)
    {
        return false;
    }
    return true;
}
