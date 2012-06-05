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


#ifndef REKONQ_VIEW_H
#define REKONQ_VIEW_H


// Local Includes
#include "webview.h"

// Qt Includes
#include <QWidget>

// Forward Declarations
class WalletBar;

class WebPage;

class QLabel;
class QTimer;


class RekonqView : public QWidget
{
    Q_OBJECT

public:
    explicit RekonqView(QWidget *parent = 0);
    ~RekonqView();

    WebView *view();
    WebPage *page();

    KUrl url();

    bool hasRSSInfo();

public Q_SLOTS:
    void loadUrl(const KUrl& url);

private Q_SLOTS:
    void setTitle(const QString &);
    void setIcon();

    void createWalletBar(const QString &, const QUrl &);
    void notifyMessage(const QString &msg);

Q_SIGNALS:
    void loadProgressing();

private:
    WebView *m_webView;

    QWeakPointer<WalletBar> m_walletBar;

    QLabel *m_popup;
    QTimer *m_hidePopupTimer;
};

#endif
