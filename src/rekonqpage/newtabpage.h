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
#include <QWebElement>

// Forward Includes
class KBookmark;
class WebPage;


class NewTabPage : public QObject
{
Q_OBJECT
public:
    NewTabPage(QWebFrame *frame);
    ~NewTabPage();

    /**
     *  This is the unique NewTabPage public method. It takes an
     *  about: url and loads the corresponding part of the 
     *  new tab page
     */
    void generate(const KUrl &url = KUrl("about:home"));
    
protected slots:
    void snapFinished();
    void removePreview(int index);
     
protected:  // these are the function to build the new tab page
    void browsingMenu(const KUrl &currentUrl);

    void favoritesPage();
    QWebElement emptyPreview(int index);
    QWebElement loadingPreview(int index, KUrl url);
    QWebElement validPreview(int index, KUrl url, QString title);
    
    /** This function takes a QwebElement with the .thumbnail structure.
        It hides the "remove" and "modify" buttons->
    */
    void hideControls(QWebElement e);
    void showControls(QWebElement e);
    
    
    void historyPage();
    void bookmarksPage();
    void closedTabsPage();

private:
    void createBookItem(const KBookmark &bookmark, QWebElement parent);
    
    /** This function helps to get faster a new markup of one type,it isn't easy to create one with QWebElement.
        It gets it in the #models div of home.html.
        It works for all elements defined here.
    */
    inline QWebElement markup(QString selector) 
    {
       return m_root.document().findFirst("#models > " + selector).clone();
    }
    
    QString checkTitle(QString title);

    QString m_html;
    
    QWebElement m_root;
};

#endif // REKONQ_NEW_TAB_PAGE
