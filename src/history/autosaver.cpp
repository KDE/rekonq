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


// Self Includes
#include "autosaver.h"
#include "autosaver.moc"

// Qt Includes
#include <QtCore/QMetaObject>
#include <QtCore/QTimerEvent>
#include <QtCore/QBasicTimer>
#include <QtCore/QTime>


const int AUTOSAVE_TIME  = 1000 * 3;  // seconds
const int MAX_TIME_LIMIT = 1000 * 15; // seconds


AutoSaver::AutoSaver(QObject *parent)
    : QObject(parent)
    , m_timer(new QBasicTimer)
    , m_firstChange(new QTime)
{
}


AutoSaver::~AutoSaver()
{
    if(m_timer->isActive())
        kDebug() << "AutoSaver: still active when destroyed, changes not saved.";

    delete m_firstChange;
    delete m_timer;
}


void AutoSaver::saveIfNeccessary()
{
    if(m_timer->isActive())
        save();
}


void AutoSaver::changeOccurred()
{
    if(m_firstChange->isNull())
        m_firstChange->start();

    if(m_firstChange->elapsed() > MAX_TIME_LIMIT)
        save();
    else
        m_timer->start(AUTOSAVE_TIME, this);
}


void AutoSaver::timerEvent(QTimerEvent *event)
{
    if(event->timerId() == m_timer->timerId())
        save();
    else
        QObject::timerEvent(event);
}


void AutoSaver::save()
{
    m_timer->stop();
    delete m_firstChange;
    m_firstChange = new QTime;

    emit saveNeeded();
}
