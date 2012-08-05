/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2012 by Andrea Diamantini <adjam7 at gmail dot com>
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


#ifndef WEB_WINDOW
#define WEB_WINDOW


// Rekonq Includes
#include "rekonq_defines.h"

// KDE Includes
#include <KActionCollection>

// Qt Includes
#include <QWidget>
#include <QAction>

// Forward Declarations
class WebPage;
class WebTab;
class WebView;

class BookmarkToolBar;
class UrlBar;
class RekonqMenu;

class KMenu;
class KToolBar;

class QLabel;
class QPixmap;
class QTimer;
class QUrl;


class WebWindow : public QWidget
{
    Q_OBJECT

public:
    WebWindow(QWidget *parent = 0, WebPage *pg = 0);

    void load(const QUrl &);

    WebPage *page();

    KUrl url() const;
    QString title() const;
    QIcon icon() const;

    UrlBar *urlBar();
    WebTab *view();
    
    QPixmap tabPreview(int width, int height);

    bool isLoading();

    virtual KActionCollection *actionCollection () const;
    QAction *actionByName(const QString &name);
    
private:
    void init();
    void setupActions();
    void setupTools();
    
private Q_SLOTS:
    void webLoadProgress(int);
    void webLoadStarted();
    void webLoadFinished(bool);

    // history related
    void aboutToShowBackMenu();
    void aboutToShowForwardMenu();
    void openActionUrl(QAction *action);
    void openPrevious(Qt::MouseButtons = Qt::LeftButton, Qt::KeyboardModifiers = Qt::NoModifier);
    void openNext(Qt::MouseButtons = Qt::LeftButton, Qt::KeyboardModifiers = Qt::NoModifier);
    void updateHistoryActions();

    /**
     * Notifies a message in a popup
    */
    void notifyMessage(const QString &msg);

Q_SIGNALS:
    void titleChanged(QString);

    void loadStarted();
    void loadProgress(int);
    void loadFinished(bool);

    void pageCreated(WebPage *);

private:
    int _progress;

    WebTab *_tab;
    UrlBar *_bar;

    KToolBar *_mainToolBar;
    BookmarkToolBar *_bookmarksBar;

    KAction *m_loadStopReloadAction;

    KMenu *m_historyBackMenu;
    KMenu *m_historyForwardMenu;

    RekonqMenu *m_rekonqMenu;

    QLabel *m_popup;
    QTimer *m_hidePopupTimer;

    KActionCollection *_ac;
};

#endif // WEB_WINDOW
