/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2007-2008 Trolltech ASA. All rights reserved
* Copyright (C) 2008-2011 by Andrea Diamantini <adjam7 at gmail dot com>
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


#ifndef AUTOSAVER_H
#define AUTOSAVER_H


// Rekonq Includes
#include "rekonq_defines.h"

// Qt Includes
#include <QObject>

// Forward Declarations
class QBasicTimer;
class QTime;


/**
 * This class emits the saveNeeded() signal.
 * It will wait several seconds after changeOccurred() to combine
 * multiple changes preventing continuous writing to disk.
 */
class AutoSaver : public QObject
{
    Q_OBJECT

public:
    explicit AutoSaver(QObject *parent);
    virtual ~AutoSaver();

    /**
     * Emits the saveNeeded() signal if there's been any change since we last saved.
     */
    void saveIfNeccessary();

Q_SIGNALS:
    void saveNeeded();

public Q_SLOTS:
    void changeOccurred();

protected:
    virtual void timerEvent(QTimerEvent *event);

private:
    void save();

    QBasicTimer *m_timer;
    QTime *m_firstChange;
};

#endif // AUTOSAVER_H
