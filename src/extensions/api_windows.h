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


#ifndef API_WINDOWS_H
#define API_WINDOWS_H


// Rekonq Includes
#include "rekonq_defines.h"

// Qt Includes
#include <QObject>

// Forward Declarations
class RekonqWindow;

// typedefs
typedef quint32 WindowId;


class REKONQ_TESTS_EXPORT Windows : public QObject
{
    Q_OBJECT

public:
    Windows(QObject *parent = 0);

    /**
     * Searches and eventually returns a pointer
     * to the window with id winId
     */
    RekonqWindow *windowFor(int windowId);
    
public Q_SLOTS:
    // For the public API implemented here
    // see: http://developer.chrome.com/extensions/windows.html

    QVariantMap get(WindowId windowId, QVariantMap getInfo = QVariantMap() /*, function callback */ );
    
    QVariantMap getCurrent(QVariantMap getInfo /*, function callback */ );

    QVariantMap getLastFocused(QVariantMap getInfo /*, function callback */ );
    
    QVariantMap getAll(QVariantMap getInfo /*, function callback */ );

    void create(QVariantMap createData /*, function callback */ );

    QVariantMap update(WindowId windowId, QVariantMap updateInfo /*, function callback */);

    QVariantMap remove(WindowId windowId /*, function callback */);
    
    
Q_SIGNALS:
    void onCreated(QVariantMap window);
    void onRemoved(WindowId windowId);
    void onFocusChanged(WindowId windowId);    
};

#endif // API_WINDOWS_H
