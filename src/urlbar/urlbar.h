/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2008-2009 by Andrea Diamantini <adjam7 at gmail dot com>
* Copyright (C) 2009 by Domrachev Alexandr <alexandr.domrachev@gmail.com>
* Copyright (C) 2009 by Paweł Prażak <pawelprazak at gmail dot com>
* Copyright (C) 2009 by Lionel Chauvin <megabigbug@yahoo.fr>
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
    IconButton(QWidget *parent = 0);
    
signals:
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
        KGet    = 0x00000001,
        RSS     = 0x00000010,
        SSL     = 0x00000100,
        BK      = 0x00001000
    };

    explicit UrlBar(QWidget *parent = 0);
    ~UrlBar();

    void activateSuggestions(bool);

public slots:
    void setQUrl(const QUrl &url);

private slots:
    void activated(const KUrl& url, Rekonq::OpenType = Rekonq::CurrentTab);

    void loadFinished();
    void loadTyped(const QString &);

    void clearRightIcons();
    
    void detectTypedString(const QString &);
    void suggest();

    void showBookmarkInfo(const QPoint &pos);
    void onBookmarksChanged();

    void refreshFavicon();
    
protected:
    void paintEvent(QPaintEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void focusInEvent(QFocusEvent *event);
    void dropEvent(QDropEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *);
    void resizeEvent(QResizeEvent *);

private:
    IconButton *addRightIcon(UrlBar::icon);

    QWeakPointer<CompletionWidget> _box;
    WebTab *_tab;

    IconButton *_icon;
    IconButtonPointerList _rightIconsList;

    QTimer *_suggestionTimer;
};


#endif
