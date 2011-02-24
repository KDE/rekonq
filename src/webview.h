/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2008-2011 by Andrea Diamantini <adjam7 at gmail dot com>
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


#ifndef WEBVIEW_H
#define WEBVIEW_H


// Rekonq Includes
#include "rekonq_defines.h"

// KDE Includes
#include <KWebView>

//Qt Includes
#include <QtCore/QTime>

// Forward Declarations
class WebPage;


class REKONQ_TESTS_EXPORT WebView : public KWebView
{
    Q_OBJECT

public:
    explicit WebView(QWidget *parent);
    ~WebView();

    WebPage *page();

    inline QPoint mousePos()
    {
        return m_mousePos;
    }

protected:
    void contextMenuEvent(QContextMenuEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void enterEvent(QEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void wheelEvent(QWheelEvent *event);
    void dropEvent(QDropEvent *event);

private slots:
    void search();

    void printFrame();

    void loadUrlInNewTab(const KUrl &);
    void openLinkInNewWindow();
    void openLinkInNewTab();

    void viewImage(Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers);
    void slotCopyImageLocation();
    void inspect();

    void scrollFrameChanged();
    void scrollTick();

    void setupSmoothScrolling(int posY);

    void stopScrolling();
    void changeWindowIcon();

signals:
    void loadUrl(const KUrl &, const Rekonq::OpenType &);
    void zoomChanged(int);
    void openPreviousInHistory();
    void openNextInHistory();

private:
    QPoint m_mousePos;
    QPoint m_clickPos;

    // Auto Scroll
    QTimer *const m_autoScrollTimer;
    int m_vScrollSpeed;
    int m_hScrollSpeed;
    bool m_canEnableAutoScroll;
    bool m_isAutoScrollEnabled;

    // Smooth Scroll
    QTimer *const m_smoothScrollTimer;
    QTime m_smoothScrollTime;
    bool m_scrollBottom;
    bool m_smoothScrolling;
    int m_dy;
    int m_smoothScrollSteps;
};

#endif
