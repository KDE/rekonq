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


#ifndef COMPLETION_WIDGET_H
#define COMPLETION_WIDGET_H


// Local Includes
#include "application.h"
#include "urlresolver.h"
#include "listitem.h"

// KDE Includes
#include <KLineEdit>

// Qt Includes
#include <QFrame>


class CompletionWidget : public QFrame
{
    Q_OBJECT

public:
    CompletionWidget(QWidget *parent);

    void insertSearchList(const UrlSearchList &list);
    void popup();
    void clear();

    virtual bool eventFilter(QObject *obj, QEvent *ev);
    void setVisible(bool visible);
    
private slots:
    void itemChosen(ListItem *item, Qt::MouseButton = Qt::LeftButton);

signals:
    void chosenUrl(const KUrl &, Rekonq::OpenType);

private:
    void sizeAndPosition();
    void up();
    void down();

    QWidget *_parent;

    UrlSearchList _list;
    int _currentIndex;
};

#endif // COMPLETION_WIDGET_H
