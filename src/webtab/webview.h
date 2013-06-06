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


#ifndef WEBVIEW_H
#define WEBVIEW_H


// Rekonq Includes
#include "rekonq_defines.h"

// KDE Includes
#include <KWebView>

// Qt Includes
#include <QTime>
#include <QPoint>
#include <QPixmap>
#include <QWebHitTestResult>

// Forward Declarations
class WebPage;
class WebTab;

class QLabel;
class QTimer;


class REKONQ_TESTS_EXPORT WebView : public KWebView
{
    Q_OBJECT

public:

    enum ContextType
    {
        EmptySelection  = 0x00000000,
        LinkSelection   = 0x00000001,
        ImageSelection  = 0x00000010,
        TextSelection   = 0x00000100
    };

    explicit WebView(QWidget *parent, bool isPrivateBrowsing);
    ~WebView();

    WebPage *page();
    void setPage(WebPage *);

    WebTab *parentTab();

    void load(const QUrl &url);
    void load(const QNetworkRequest &req,
              QNetworkAccessManager::Operation op = QNetworkAccessManager::GetOperation,
              const QByteArray & body = QByteArray());

public Q_SLOTS:
    void reload();
    
protected:
    bool popupSpellMenu(QContextMenuEvent *event);
    void contextMenuEvent(QContextMenuEvent *event);

    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);

    void wheelEvent(QWheelEvent *event);

    void dropEvent(QDropEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);
    void dragMoveEvent(QDragMoveEvent * event);

    void paintEvent(QPaintEvent *event);

private Q_SLOTS:
    void search();

    void openLinkHere();
    void openLinkInNewTab();
    void openLinkInNewWindow();
    void openLinkInPrivateWindow();

    void bookmarkLink();
    void spellCheck();
    void spellCheckerCorrected(const QString& original, int pos, const QString& replacement);
    void spellCheckerMisspelling(const QString& text, int pos);
    void slotSpellCheckDone(const QString&);
    void sendByMail();

    void saveImage();
    void viewImage(Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers);
    void slotCopyImageLocation();

    void scrollFrameChanged();
    void scrollTick();

    void setupSmoothScrolling(int posY);

    void stopSmoothScrolling();

    void accessKeyShortcut();
    void hideAccessKeys();

    void loadStarted();

    void blockImage();

    void guessHoveredLink(QPoint);

private:
    bool checkForAccessKey(QKeyEvent *event);
    void showAccessKeys();
    void makeAccessKeyLabel(const QChar &accessKey, const QWebElement &element);

Q_SIGNALS:
    void loadUrl(const KUrl &, const Rekonq::OpenType &);

    void zoomChanged(int);
    void openPreviousInHistory();
    void openNextInHistory();

private:
    QPoint m_clickPos;
    QWebHitTestResult m_contextMenuHitResult;

    // Spell Checking
    int m_spellTextSelectionStart;
    int m_spellTextSelectionEnd;

    // Auto Scroll
    QTimer *const m_autoScrollTimer;
    int m_verticalAutoScrollSpeed;
    int m_horizontalAutoScrollSpeed;
    bool m_isViewAutoScrolling;
    QPixmap m_autoScrollIndicator;

    // Smooth Scroll
    QTimer *const m_smoothScrollTimer;
    QTime m_smoothScrollTime;
    bool m_smoothScrollBottomReached;
    int m_dy;
    int m_smoothScrollSteps;
    bool m_isViewSmoothScrolling;

    // Access Keys
    QList<QLabel*> m_accessKeyLabels;
    QHash<QChar, QWebElement> m_accessKeyNodes;
    bool m_accessKeysPressed;
    bool m_accessKeysActive;

    bool m_isExternalLinkHovered;

    WebTab *m_parentTab;

    bool m_isPrivateBrowsing;
};

#endif
