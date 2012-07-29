/***************************************************************************
 *   Copyright (C) 2012 by Andrea Diamantini <adjam7@gmail.com>                            *
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


#ifndef TAB_HISTORY
#define TAB_HISTORY


// Qt Includes
#include <QByteArray>
#include <QString>
#include <QWebHistory>


class TabHistory
{
public:
    explicit TabHistory(QWebHistory *h = 0)
    {
        if (h)
        {
            title = h->currentItem().title();
            url = h->currentItem().url().toString();
            QDataStream stream(&history, QIODevice::ReadWrite);
            stream << *h;
        }
    }

    inline bool operator ==(const TabHistory &other) const
    {
        return history == other.history;
    }

    void applyHistory(QWebHistory *h)
    {
        if (h)
        {
            QDataStream stream(&history, QIODevice::ReadOnly);
            stream >> *h;
        }
    }

    QString title;
    QString url;
    QByteArray history;
};

#endif // TAB_HISTORY
