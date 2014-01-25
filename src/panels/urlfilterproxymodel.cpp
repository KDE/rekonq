/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2009 by Nils Weigel <nehlsen at gmail dot com>
* Copyright (C) 2010-2014 by Andrea Diamantini <adjam7 at gmail dot com>
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


// Self Includes
#include "urlfilterproxymodel.h"


UrlFilterProxyModel::UrlFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    setFilterCaseSensitivity(Qt::CaseInsensitive);
}


bool UrlFilterProxyModel::filterAcceptsRow(const int source_row, const QModelIndex &source_parent) const
{
    return recursiveMatch(sourceModel()->index(source_row, 0, source_parent));
}


bool UrlFilterProxyModel::recursiveMatch(const QModelIndex &index) const
{
    if (index.data().toString().contains(filterRegExp()))
        return true;

    int numChildren = sourceModel()->rowCount(index);
    for (int childRow = 0; childRow < numChildren; ++childRow)
    {
        if (recursiveMatch(sourceModel()->index(childRow, 0, index)))
            return true;
    }

    return false;
}
