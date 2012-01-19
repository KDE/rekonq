/***************************************************************************
 *   Copyright (C) 2011-2012 by Andrea Diamantini <adjam7@gmail.com>                            *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#ifndef WEB_VIEW_H
#define WEB_VIEW_H


// Local Includes
#include "webpage.h"

// KDE Includes
#include <KWebView>

// Qt Includes
#include <QUrl>


class WebView : public KWebView
{
    Q_OBJECT
    
public:
    explicit WebView(const QUrl &url, QWidget *parent = 0);

private Q_SLOTS:
    void setTitle(const QString &);
    void setIcon();
    void menuRequested(const QPoint &);
    void openLinkInDefaultBrowser();
};

#endif // WEB_VIEW_H
