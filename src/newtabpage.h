/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2009-2010 by Andrea Diamantini <adjam7 at gmail dot com>
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
#include <QtCore/QObject>
#include <QtWebKit/QWebElement>

// Forward Declarations
class KBookmark;
class QWebFrame;


class REKONQ_TESTS_EXPORT NewTabPage : public QObject
{
    Q_OBJECT

public:
    NewTabPage(QWebFrame *frame);
    ~NewTabPage();

    /**
     * This method takes an about: url and loads
     * the corresponding part of the new tab page
     */
    void generate(const KUrl &url = KUrl("about:home"));

    /**
     * This method updates thumbs, removing loading previews
     * and providing a real picture
     */
    void snapFinished();

private:
    // these are the "high-level" functions to build the new tab page.
    // Basically, you call browsingMenu + one of the *Page methods
    // to load a page
    void browsingMenu(const KUrl &currentUrl);

    void favoritesPage();
    void historyPage();
    void bookmarksPage();
    void closedTabsPage();
    void downloadsPage();

    // --------------------------------------------------------------------------
    // "low-level" functions
    // we use these to create the pages over

    // Previews handling
    QWebElement emptyPreview(int index);
    QWebElement loadingPreview(int index, const KUrl &url);
    QWebElement validPreview(int index, const KUrl &url, const QString &title);

    void removePreview(int index);

    /**
     * This function takes a QwebElement with the .thumbnail structure,
     * hiding the "remove" and "modify" buttons
     *
     */
    void hideControls(QWebElement e);
    void showControls(QWebElement e);
    void setupPreview(QWebElement e, int index);

    void createBookItem(const KBookmark &bookmark, QWebElement parent);

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

    QString checkTitle(const QString &title);

private:
    QWebElement createLinkItem(const QString &title, const QString &urlString, const QString &iconPath, int groupOrSize) const;

    QString m_html;
    QWebElement m_root;
};

#endif // REKONQ_NEW_TAB_PAGE
