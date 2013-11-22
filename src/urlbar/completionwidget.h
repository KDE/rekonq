/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2009-2013 by Andrea Diamantini <adjam7 at gmail dot com>
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
#include "urlsuggester.h"

// KDE Includes
#include <KService>

// Qt Includes
#include <QFrame>

// Forward Declarations
class ListItem;


class CompletionWidget : public QFrame
{
    Q_OBJECT

public:
    explicit CompletionWidget(QWidget *parent);

    virtual bool eventFilter(QObject *obj, QEvent *ev);
    void setVisible(bool visible);

    void suggestUrls(const QString &text);

    KUrl activeSuggestion();

private Q_SLOTS:
    void itemChosen(ListItem *item, Qt::MouseButton = Qt::LeftButton, Qt::KeyboardModifiers = Qt::NoModifier);
    void updateSuggestionList(const UrlSuggestionList &list, const QString& text);

Q_SIGNALS:
    void chosenUrl(const KUrl &, Rekonq::OpenType);
    void nextItemSubChoice();

private:
    void insertItems(const UrlSuggestionList &list, const QString& text, int offset = 0);

    void popup();
    void clear();

    void sizeAndPosition();
    void up();
    void down();
    void activateCurrentListItem();

    QWidget *_parent;

    UrlSuggestionList _list;

    int _currentIndex;

    KService::Ptr _searchEngine;

    QString _typedString;
    bool _hasSuggestions;
};

#endif // COMPLETION_WIDGET_H
