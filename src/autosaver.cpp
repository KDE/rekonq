/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the demonstration applications of the Qt Toolkit.
**
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License versions 2.0 or 3.0 as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file.  Please review the following information
** to ensure GNU General Public Licensing requirements will be met:
** http://www.fsf.org/licensing/licenses/info/GPLv2.html and
** http://www.gnu.org/copyleft/gpl.html.  In addition, as a special
** exception, Nokia gives you certain additional rights. These rights
** are described in the Nokia Qt GPL Exception version 1.3, included in
** the file GPL_EXCEPTION.txt in this package.
**
** Qt for Windows(R) Licensees
** As a special exception, Nokia, as the sole copyright holder for Qt
** Designer, grants users of the Qt/Eclipse Integration plug-in the
** right for the Qt/Eclipse Integration to link to functionality
** provided by Qt Designer and its related libraries.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
**
****************************************************************************/

#include "autosaver.h"

#include <QtCore/QDir>
#include <QtCore/QCoreApplication>
#include <QtCore/QMetaObject>
#include <QtDebug>

#define AUTOSAVE_IN  1000 * 3  // seconds
#define MAXWAIT      1000 * 15 // seconds

AutoSaver::AutoSaver(QObject *parent) : QObject(parent)
{
    Q_ASSERT(parent);
}

AutoSaver::~AutoSaver()
{
    if (m_timer.isActive())
        qWarning() << "AutoSaver: still active when destroyed, changes not saved.";
}

void AutoSaver::changeOccurred()
{
    if (m_firstChange.isNull())
        m_firstChange.start();

    if (m_firstChange.elapsed() > MAXWAIT) {
        saveIfNeccessary();
    } else {
        m_timer.start(AUTOSAVE_IN, this);
    }
}

void AutoSaver::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == m_timer.timerId()) {
        saveIfNeccessary();
    } else {
        QObject::timerEvent(event);
    }
}

void AutoSaver::saveIfNeccessary()
{
    if (!m_timer.isActive())
        return;
    m_timer.stop();
    m_firstChange = QTime();
    if (!QMetaObject::invokeMethod(parent(), "save", Qt::DirectConnection)) {
        qWarning() << "AutoSaver: error invoking slot save() on parent";
    }
}

