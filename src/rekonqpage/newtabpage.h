/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2009-2012 by Andrea Diamantini <adjam7 at gmail dot com>
* Copyright (C) 2010 by Matthieu Gicquel <matgic78 at gmail dot com>
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


#ifndef REKONQ_NEW_TAB_PAGE
#define REKONQ_NEW_TAB_PAGE


// Rekonq Includes
#include "rekonq_defines.h"

// KDE Includes
#include <KUrl>

// Qt Includes
#include <QObject>
#include <QString>
#include <QWebElement>

// Forward Declarations
class KBookmark;
class QWebFrame;


class REKONQ_TESTS_EXPORT NewTabPage : public QObject
{
    Q_OBJECT

public:
    explicit NewTabPage(QWebFrame *frame);

    /**
     * This method takes an rekonq: url and loads
     * the corresponding part of the new tab page
     */
    void generate(const KUrl &url = KUrl("rekonq:home"));

private:
    // these are the "high-level" functions to build the new tab page.
    // Basically, you call browsingMenu + one of the *Page methods
    // to load a page
    void browsingMenu(const KUrl &currentUrl);

    void favoritesPage();
    void historyPage(const QString & filter = QString());
    void bookmarksPage();
    void closedTabsPage();
    void downloadsPage(const QString & filter = QString());
    void tabsPage();

    void loadPageForUrl(const KUrl &url, const QString & filter = QString());

    // --------------------------------------------------------------------------
    // "low-level" functions
    // we use these to create the pages over

    // Previews handling
    QWebElement emptyPreview(int index);
    QWebElement validPreview(int index, const KUrl &url, const QString &title);
    QWebElement tabPreview(int winIndex, int tabIndex, const KUrl &url, const QString &title);
    QWebElement closedTabPreview(int index, const KUrl &url, const QString &title);

    void reloadPreview(int index);
    void removePreview(int index);

    void setupPreview(QWebElement e, int index, bool showControls);
    void setupTabPreview(QWebElement e, int winIndex, int tabIndex);

    void createBookmarkItem(const KBookmark &bookmark, QWebElement parent);
    void createBookmarkGroup(const KBookmark &bookmark, QWebElement parent);

    QWebElement createLinkItem(const QString &title, const QString &urlString, const QString &iconPath, int groupOrSize) const;
    QWebElement createFormItem(const QString &title, const QString &urlString) const;

    /**
     * This function helps to get faster a new markup of one type,
     * it isn't easy to create one with QWebElement.
     *
     * It gets it in the #models div of home.html.
     * It works for all elements defined here.
     *
     */
    inline QWebElement markup(const QString &selector) const
    {
        return m_root.document().findFirst("#models > " + selector).clone();
    }

    QString checkTitle(const QString &title, int max = 20);

    void updateWindowIcon();

    void initJS();
    void saveFavorites();

private:
    QString m_html;
    QWebElement m_root;

    bool m_showFullHistory;
};

#endif // REKONQ_NEW_TAB_PAGE
