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


#ifndef ADBLOCK_WIDGET_H
#define ADBLOCK_WIDGET_H

// Qt Includes
#include <QCheckBox>
#include <QMenu>
#include <QUrl>


class AdBlockWidget : public QMenu
{
    Q_OBJECT

public:
    explicit AdBlockWidget(const QUrl &url, QWidget *parent = 0);
    virtual ~AdBlockWidget();

    void showAt(const QPoint &pos);

Q_SIGNALS:
    void updateIcon();

private Q_SLOTS:
    void accept();

private:
    QUrl _pageUrl;

    QCheckBox *_chBox;
    bool _isAdblockEnabledHere;
};

#endif
