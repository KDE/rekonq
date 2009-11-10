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


#ifndef REKONQ_NEW_TAB_PAGE
#define REKONQ_NEW_TAB_PAGE


// KDE Includes
#include <KUrl>

// Qt Includes
#include <QtCore/QObject>
#include <QtCore/QString>

// Forward Includes
class KBookmark;


class NewTabPage
{
    
public:
    NewTabPage();
    ~NewTabPage();

    /**
     *  This is the unique NewTabPage public method. It takes an
     *  about: url and loads the corresponding part of the 
     *  new tab page
     */
    QString newTabPageCode(const KUrl &url = KUrl("rekonq:home"));
     
protected:  // these are the function to build the new tab page
    
    QString browsingMenu(const KUrl &currentUrl);

    QString favoritesPage();
    QString lastVisitedPage();
    QString historyPage();
    QString bookmarksPage();
    QString closedTabsPage();

private:
    QString createBookItem(const KBookmark &bookmark);

    QString m_htmlFilePath;
};

#endif // REKONQ_NEW_TAB_PAGE
