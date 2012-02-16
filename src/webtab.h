/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2008-2012 by Andrea Diamantini <adjam7 at gmail dot com>
* Copyright (C) 2009-2011 by Lionel Chauvin <megabigbug@yahoo.fr>
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
#include "webpage.h"
#include "webview.h"

// KDE Includes
#include <KParts/Part>

// Qt Includes
#include <QtGui/QWidget>

// Forward Declarations
class UrlBar;
class PreviewSelectorBar;
class WalletBar;
class NotificationBar;
class QPoint;


class REKONQ_TESTS_EXPORT WebTab : public QWidget
{
    Q_OBJECT

public:
    explicit WebTab(QWidget *parent = 0);
    ~WebTab();

    WebView *view() const
    {
        return m_webView;
    }
    UrlBar *urlBar() const
    {
        return m_urlBar;
    }
    WebPage *page() const
    {
        return view()->page();
    }

    inline int progress() const
    {
        return m_progress;
    }

    KUrl url();
    void createPreviewSelectorBar(int index);

    void hideSelectorBar();

    bool hasRSSInfo();

    bool isPageLoading();
    bool hasNewSearchEngine();

    KParts::ReadOnlyPart *part()
    {
        return m_part;
    }

    void setPart(KParts::ReadOnlyPart *p, const KUrl &u);


private Q_SLOTS:
    void updateProgress(int progress);
    void resetProgress();

    void createWalletBar(const QString &, const QUrl &);
    void showRSSInfo(const QPoint &pos);
    void showSearchEngine(const QPoint &pos);
    void openSearchEngineAdded();

    void showMessageBar();

Q_SIGNALS:
    void loadProgressing();
    void titleChanged(const QString &);

private:
    KUrl extractOpensearchUrl(QWebElement e);

    WebView *const m_webView;
    UrlBar *const m_urlBar;

    int m_progress;

    QWeakPointer<WalletBar> m_walletBar;
    QWeakPointer<PreviewSelectorBar> m_previewSelectorBar;

    KParts::ReadOnlyPart *m_part;
};

#endif
