/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2013 by Andrea Diamantini <adjam7 at gmail dot com>
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



#ifndef REKONQ_WINDOW_H
#define REKONQ_WINDOW_H


// Rekonq Includes
#include "rekonq_defines.h"

// Local Includes
#include "rwindow.h"
#include "tabwidget.h"

#include "bookmarkspanel.h"
#include "historypanel.h"

// Qt Includes
#include <QSplitter>
#include <QWeakPointer>

// Forward Declarations
class TabBar;

class WebPage;
class WebWindow;


class RekonqWindow : public RWindow
{
    Q_OBJECT

public:
    explicit RekonqWindow(bool withTab = true, bool PrivateBrowsingMode = false, QWidget *parent = 0);
    explicit RekonqWindow(WebPage *pg, QWidget *parent = 0);

    virtual ~RekonqWindow();

    TabWidget *tabWidget();
    TabBar *tabBar();
    WebWindow *currentWebWindow() const;

    bool isPrivateBrowsingMode();

private:
    void init();
        
public Q_SLOTS:
    void loadUrl(const KUrl &, Rekonq::OpenType type = Rekonq::CurrentTab, TabHistory *history = 0);

private Q_SLOTS:
    void showBookmarksPanel(bool);
    void showHistoryPanel(bool);
    void registerWindow();
    
private:
    TabWidget *_tabWidget;
    
    QSplitter *_splitter;
    
    QWeakPointer<HistoryPanel> _historyPanel;
    QWeakPointer<BookmarksPanel> _bookmarksPanel;
};

#endif // REKONQ_WINDOW_H
