/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2008-2012 by Andrea Diamantini <adjam7 at gmail dot com>
* Copyright (C) 2009 by Domrachev Alexandr <alexandr.domrachev@gmail.com>
* Copyright (C) 2009 by Paweł Prażak <pawelprazak at gmail dot com>
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


#ifndef URLBAR_H
#define URLBAR_H

// Rekonq Includes
#include "rekonq_defines.h"

// KDE Includes
#include <KUrl>
#include <KLineEdit>

// Qt Includes
#include <QWeakPointer>
#include <QToolButton>

// Forward Declarations
class QWidget;
class CompletionWidget;
class WebTab;
class QTimer;


class IconButton : public QToolButton
{
    Q_OBJECT

public:
    explicit IconButton(QWidget *parent = 0);

Q_SIGNALS:
    void clicked(QPoint);

protected:
    void mouseReleaseEvent(QMouseEvent *event);
};


// Definitions
typedef QList<IconButton *> IconButtonPointerList;


// ------------------------------------------------------------------------------------


class REKONQ_TESTS_EXPORT UrlBar : public KLineEdit
{
    Q_OBJECT

public:

    enum icon
    {
        KGet         = 0x00000001,
        RSS          = 0x00000010,
        BK           = 0x00001000,
        SearchEngine = 0x00010000,
        AdBlock      = 0x01000000
    };

    explicit UrlBar(QWidget *parent = 0);
    ~UrlBar();

public Q_SLOTS:
    void setQUrl(const QUrl &url);

    /**
     * Let us add bookmarks as the major browsers do
     *
     */
    void manageBookmarks();

    void clearUrlbar();

private Q_SLOTS:
    void loadRequestedUrl(const KUrl& url, Rekonq::OpenType = Rekonq::CurrentTab);

    void loadStarted();
    void loadFinished();

    void clearRightIcons();
    void updateRightIcons();

    void detectTypedString(const QString &);
    void suggest();

    void manageStarred(QPoint);
    void manageAdBlock(QPoint);

    void addToFavorites();
    void removeFromFavorites();

    void refreshFavicon();

    void pasteAndGo();
    void pasteAndSearch();
    void delSlot();
    bool isValidURL(QString url);

    /**
     * Load typed url
     */
    void loadTypedUrl();

    void showRSSInfo(QPoint);
    void showSSLInfo(QPoint);

protected:
    void paintEvent(QPaintEvent *event);
    void keyReleaseEvent(QKeyEvent *event);
    void focusInEvent(QFocusEvent *event);
    void dropEvent(QDropEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *);
    void contextMenuEvent(QContextMenuEvent *event);
    void resizeEvent(QResizeEvent *);

Q_SIGNALS:
    void focusIn();

private:
    /**
     * Updates right icon position, given its number in the right icons list
     * and considering rekonq window position/dimension
     */
    void updateRightIconPosition(IconButton *, int);
    IconButton *addRightIcon(UrlBar::icon);

    QWeakPointer<CompletionWidget> _box;
    WebTab *_tab;

    IconButton *_icon;
    IconButtonPointerList _rightIconsList;

    QTimer *_suggestionTimer;
};


#endif
