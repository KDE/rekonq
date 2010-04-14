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


#ifndef LISTITEM_H
#define LISTITEM_H


// Qt Includes
#include <QWidget>
#include <QLayout>
#include <QStyleOptionViewItemV4>

// Forward Declarations
class UrlSearchItem;
class KUrl;


class ListItem : public QWidget
{
    Q_OBJECT

public:
    ListItem(const UrlSearchItem &item, QWidget *parent = 0);
    ~ListItem();

    void activate();
    void deactivate();

signals:
    void itemClicked(ListItem *item, Qt::MouseButton);

protected:
   virtual void paintEvent(QPaintEvent *event);
   virtual void enterEvent(QEvent *);
   virtual void leaveEvent(QEvent *);
   virtual void mousePressEvent(QMouseEvent *e);

private:
    QString guessNameFromUrl(KUrl url);
    void insertIcon(QLayout *layout, QString icon);
    
    QStyleOptionViewItemV4 _option;
};


#endif // LISTITEM_H
