/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2009 by Nils Weigel <nehlsen at gmail dot com>
* Copyright (C) 2010-2011 by Andrea Diamantini <adjam7 at gmail dot com>
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


#ifndef URLFILTERPROXYMODEL_H
#define URLFILTERPROXYMODEL_H


// Rekonq Includes
#include "rekonq_defines.h"

// Qt Includes
#include <QSortFilterProxyModel>


/**
 * QSortFilterProxyModel hides all children which parent doesn't
 * match the filter. This class is used to change this behavior.
 * If a url matches the filter it'll be shown,
 * even if it's parent doesn't match it.
 */
class REKONQ_TESTS_EXPORT UrlFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    explicit UrlFilterProxyModel(QObject *parent = 0);

protected:
    virtual bool filterAcceptsRow(const int source_row, const QModelIndex &source_parent) const;

    // returns true if index or any of his children match the filter
    bool recursiveMatch(const QModelIndex &index) const;
};

#endif // URLFILTERPROXYMODEL_H
