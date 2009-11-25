/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2007-2008 Trolltech ASA. All rights reserved
* Copyright (C) 2008-2009 by Andrea Diamantini <adjam7 at gmail dot com>
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
#include "autosaver.h"
#include "autosaver.moc"

// KDE Includes
#include <KDebug>

// Qt Includes
#include <QtCore/QMetaObject>


#define AUTOSAVE_IN  1000 * 3  // seconds
#define MAXWAIT      1000 * 15 // seconds


AutoSaver::AutoSaver(QObject *parent) : QObject(parent)
{
    Q_ASSERT(parent);
}


AutoSaver::~AutoSaver()
{
    if (m_timer.isActive())
    {
        kWarning() << "AutoSaver: still active when destroyed, changes not saved.";
    }
}


void AutoSaver::changeOccurred()
{
    if (m_firstChange.isNull())
        m_firstChange.start();

    if (m_firstChange.elapsed() > MAXWAIT)
    {
        saveIfNeccessary();
    }
    else
    {
        m_timer.start(AUTOSAVE_IN, this);
    }
}


void AutoSaver::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == m_timer.timerId())
    {
        saveIfNeccessary();
    }
    else
    {
        QObject::timerEvent(event);
    }
}


void AutoSaver::saveIfNeccessary()
{
    if (!m_timer.isActive())
        return;
    m_timer.stop();
    m_firstChange = QTime();
    if (!QMetaObject::invokeMethod(parent(), "save", Qt::DirectConnection))
    {
        kWarning() << "AutoSaver: error invoking slot save() on parent";
    }
}

