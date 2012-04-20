/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2009 by Jakub Wieczorek <faw217@gmail.com>
* Copyright (C) 2010-2011 by Lionel Chauvin <megabigbug@yahoo.fr>
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


#ifndef OPENSEARCHREADER_H
#define OPENSEARCHREADER_H


// Rekonq Includes
#include "rekonq_defines.h"

// Qt Includes
#include <QXmlStreamReader>

// Forward Declarations
class OpenSearchEngine;


class OpenSearchReader : public QXmlStreamReader
{
public:
    OpenSearchReader();

    OpenSearchEngine *read(const QByteArray &data);
    OpenSearchEngine *read(QIODevice *device);

private:
    OpenSearchEngine *read();
};

#endif // OPENSEARCHREADER_H
