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


// Rekonq Includes
#include "rekonq_defines.h"

// Local Includes
#include "listitem.h"

// KDE Includes
#include <KService>

// Qt Includes
#include <QFrame>


class CompletionWidget : public QFrame
{
    Q_OBJECT

public:
    CompletionWidget(QWidget *parent);

    virtual bool eventFilter(QObject *obj, QEvent *ev);
    void setVisible(bool visible);

    KService::Ptr searchEngine()
    {
        return _searchEngine;
    };
    
    void setSearchEngine(KService::Ptr engine)
    {
        _searchEngine = engine;
    };

    void suggestUrls(const QString &text);

private slots:
    void itemChosen(ListItem *item, Qt::MouseButton = Qt::LeftButton, Qt::KeyboardModifiers = Qt::NoModifier);

signals:
    void chosenUrl(const KUrl &, Rekonq::OpenType);
    void nextItemSubChoice();

private:
    void insertSearchList(const UrlSearchList &list, const QString& text);
    void popup();
    void clear();

    void sizeAndPosition();
    void up();
    void down();

    QWidget *_parent;

    UrlSearchList _list;
    int _currentIndex;

    KService::Ptr _searchEngine;

    QString _typedString;
};

#endif // COMPLETION_WIDGET_H
