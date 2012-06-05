/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2012 by Andrea Diamantini <adjam7 at gmail dot com>
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
#include "rekonqview.h"
#include "rekonqview.moc"

// Local Includes
#include "walletbar.h"
#include "webpage.h"

// KDE Includes
#include <KWebWallet>
#include <KStandardShortcut>
#include <KMenu>
#include <KActionMenu>
#include <KWebView>
#include <KDebug>
#include <KMessageBox>
#include <KRun>
#include <klocalizedstring.h>
#include <KSharedConfig>
#include <KConfigGroup>

// Qt Includes
#include <QVBoxLayout>
#include <QWebElement>
#include <QWebFrame>
#include <QByteArray>
#include <QDataStream>
#include <QLabel>
#include <QTimer>
#include <QTextDocument>
#include <QStyle>


RekonqView::RekonqView(QWidget *parent)
    : QWidget(parent)
    , m_webView(0)
    , m_popup(new QLabel(this))
    , m_hidePopupTimer(new QTimer(this))

{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QVBoxLayout *l = new QVBoxLayout(this);
    l->setMargin(0);
    l->setSpacing(0);

    l->addWidget(view());
    view()->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // fix focus handling
    setFocusProxy(view());

    KWebWallet *wallet = page()->wallet();

    if (wallet)
    {
        connect(wallet, SIGNAL(saveFormDataRequested(QString, QUrl)),
                this, SLOT(createWalletBar(QString, QUrl)));
    }

    connect(page(), SIGNAL(linkHovered(QString, QString, QString)), this, SLOT(notifyMessage(QString)));

    // setting popup notification
    m_popup->setAutoFillBackground(true);
    m_popup->setMargin(4);
    m_popup->raise();
    m_popup->hide();

    connect(m_hidePopupTimer, SIGNAL(timeout()), m_popup, SLOT(hide()));

    // signal && slots
    connect(view(), SIGNAL(iconChanged()), this, SLOT(setIcon()));
    connect(view(), SIGNAL(titleChanged(QString)), this, SLOT(setTitle(QString)));

}


RekonqView::~RekonqView()
{
    m_walletBar.clear();

    delete m_webView;
}


WebView *RekonqView::view()
{
    if (!m_webView)
    {
        m_webView = new WebView(this);
    }
    return m_webView;
}


WebPage *RekonqView::page()
{
    return view()->page();
}


KUrl RekonqView::url()
{
//     if (page() && page()->isOnRekonqPage())
//     {
//         return page()->loadingUrl();
//     }

    return view()->url();
}


void RekonqView::setTitle(const QString &t)
{
    setWindowTitle(t);
}


void RekonqView::setIcon()
{
    setWindowIcon(view()->icon());
}


void RekonqView::createWalletBar(const QString &key, const QUrl &url)
{
    // check if the url is in the wallet blacklist
    QString urlString = url.toString();

    KSharedConfig::Ptr config = KSharedConfig::openConfig("rekonqrc", KConfig::SimpleConfig, "config");
    KConfigGroup group1(config, "misc");
    QStringList blackList = group1.readEntry("walletBlackList", QStringList());
    if (blackList.contains(urlString))
        return;

    KWebWallet *wallet = page()->wallet();

    KConfigGroup group2(config, "Privacy");
    bool passwordSavingEnabled = group2.readEntry("passwordSavingEnabled", false);

    if (!passwordSavingEnabled)
    {
        wallet->rejectSaveFormDataRequest(key);
        return;
    }

    if (m_walletBar.isNull())
    {
        m_walletBar = new WalletBar(this);
        m_walletBar.data()->onSaveFormData(key, url);
        qobject_cast<QVBoxLayout *>(layout())->insertWidget(0, m_walletBar.data());
        m_walletBar.data()->animatedShow();
    }
    else
    {
        disconnect(wallet);
        qobject_cast<QVBoxLayout *>(layout())->insertWidget(0, m_walletBar.data());
        m_walletBar.data()->animatedShow();
    }

    connect(m_walletBar.data(), SIGNAL(saveFormDataAccepted(QString)),
            wallet, SLOT(acceptSaveFormDataRequest(QString)), Qt::UniqueConnection);
    connect(m_walletBar.data(), SIGNAL(saveFormDataRejected(QString)),
            wallet, SLOT(rejectSaveFormDataRequest(QString)), Qt::UniqueConnection);
}


bool RekonqView::hasRSSInfo()
{
    QWebElementCollection col = page()->mainFrame()->findAllElements("link[type=\"application/rss+xml\"]");
    col.append(page()->mainFrame()->findAllElements("link[type=\"application/atom+xml\"]"));
    if (col.count() != 0)
        return true;

    return false;
}


void RekonqView::loadUrl(const KUrl& url)
{
    if (url.isEmpty())
        return;

    if (!url.isValid())
    {
        KMessageBox::error(0, i18n("Malformed URL:\n%1", url.url(KUrl::RemoveTrailingSlash)));
        return;
    }

    view()->load(url);
}


void RekonqView::notifyMessage(const QString &msg)
{
    // deleting popus if empty msgs
    if (msg.isEmpty())
    {
        m_hidePopupTimer->start(250);
        return;
    }

    m_hidePopupTimer->stop();
    m_hidePopupTimer->start(3000);

    QString msgToShow = Qt::escape(msg);

    const int margin = 4;
    const int halfWidth = width() / 2;

    // Set Popup size
    QFontMetrics fm = m_popup->fontMetrics();
    QSize labelSize(fm.width(msgToShow) + 2 * margin, fm.height() + 2 * margin);

    if (labelSize.width() > halfWidth)
        labelSize.setWidth(halfWidth);

    m_popup->setFixedSize(labelSize);
    m_popup->setText(fm.elidedText(msgToShow, Qt::ElideMiddle, labelSize.width() - 2 * margin));

    // NOTE: while currentFrame should NEVER be null
    // we are checking here its existence cause of bug:264187
    if (!page() || !page()->currentFrame())
        return;

    const bool horizontalScrollbarIsVisible = page()->currentFrame()->scrollBarMaximum(Qt::Horizontal);
    const bool verticalScrollbarIsVisible = page()->currentFrame()->scrollBarMaximum(Qt::Vertical);
    const bool actionBarsVisible = false; //FIXME m_findBar->isVisible() || m_zoomBar->isVisible();

    const int scrollbarExtent = style()->pixelMetric(QStyle::PM_ScrollBarExtent);
    const int hScrollbarSize = horizontalScrollbarIsVisible ? scrollbarExtent : 0;
    const int vScrollbarSize = verticalScrollbarIsVisible ? scrollbarExtent : 0;

    const QPoint mousePos = mapFromGlobal(QCursor::pos());
    const QPoint bottomPoint = geometry().bottomLeft();

    int y = bottomPoint.y() + 1 - 2 * m_popup->height() - hScrollbarSize;   // +1 because bottom() returns top() + height() - 1, see QRect doku
    int x = QRect(QPoint(0, y), labelSize).contains(mousePos) || actionBarsVisible
            ? width() - labelSize.width() - vScrollbarSize
            : 0;

    m_popup->move(x, y);
    m_popup->show();
}
