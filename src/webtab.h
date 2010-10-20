/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2008-2010 by Andrea Diamantini <adjam7 at gmail dot com>
* Copyright (C) 2009-2010 by Lionel Chauvin <megabigbug@yahoo.fr>
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


#ifndef WEBTAB_H
#define WEBTAB_H


// Rekonq Includes
#include "rekonq_defines.h"

// Local Includes
#include "webview.h"

// Qt Includes
#include <QtGui/QWidget>

// Forward Declarations
class UrlBar;
class PreviewSelectorBar;
class WalletBar;
class NotificationBar;


class REKONQ_TESTS_EXPORT WebTab : public QWidget
{
    Q_OBJECT

public:
    explicit WebTab(QWidget *parent = 0);
    ~WebTab();

    inline WebView *view() { return _view; }
    UrlBar *urlBar() { return _bar; }
    inline WebPage *page() { return view()->page(); }
    inline int progress() { return _progress; }

    KUrl url();
    void createPreviewSelectorBar(int index);
    void insertBar(NotificationBar* bar);

    bool hasRSSInfo();
    bool isPageLoading();

private slots:
    void updateProgress(int progress);
    void loadFinished(bool);

    void createWalletBar(const QString &, const QUrl &);
    void showRSSInfo(QPoint pos);

signals:
    void loadProgressing();

private:
    WebView *_view;
    UrlBar *_bar;
    
    int _progress;

    QWeakPointer<WalletBar> _walletBar;
    QWeakPointer<PreviewSelectorBar> _previewSelectorBar;
};

#endif
