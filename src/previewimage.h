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


#ifndef PREVIEW_IMAGE_H
#define PREVIEW_IMAGE_H

// Local Includes
#include "websnap.h"

// KDE Includes
#include <KActionMenu>

// Qt Includes
#include <QLabel>
#include <QImage>
#include <QUrl>
#include <QToolButton>

class PreviewImage : public QLabel
{
    Q_OBJECT

public:
    PreviewImage(const QUrl &url, int index, bool isFavorite);
    ~PreviewImage();
    
    QString guessNameFromUrl(QUrl url);
    
public slots:
    void snapFinished();
    void removeMe();
    void setUrlFromAction();

protected:
    void contextMenuEvent(QContextMenuEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    
    void loadUrlPreview(const QUrl &url);
    KActionMenu *historyMenu();
    void showEmptyPreview();
                
private:
    QPixmap m_pixmap;
    WebSnap *ws;
    
    QUrl m_url;
    QString m_savePath;
    
    bool loadingSnapshot;
    bool m_isFavorite;
    int m_index;
    
    QToolButton *m_button;
};

#endif // PREVIEW_IMAGE_H
