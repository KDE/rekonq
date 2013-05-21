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


#ifndef EXTENSION_POPUP_H
#define EXTENSION_POPUP_H


// Rekonq Includes
#include "rekonq_defines.h"

// Qt Includes
#include <QWidget>
#include <QWebView>
#include <QUrl>


class ExtensionPopup : public QWidget
{
    Q_OBJECT

public:
    ExtensionPopup(const QUrl &, QWidget *parent = 0);

    void showAt(const QPoint &);

private Q_SLOTS:
    void changeContentSize(QSize);

private:
    QWebView *_view;
    QUrl _url;
};

#endif // EXTENSION_POPUP_H
