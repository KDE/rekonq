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


#ifndef EXTENSION_H
#define EXTENSION_H

// Rekonq Includes
#include "rekonq_defines.h"

// KDE Includes
#include <KAction>

// Qt Includes
#include <QObject>


class REKONQ_TESTS_EXPORT Extension : public QObject
{
    Q_OBJECT

public:
    explicit Extension(const QString &id, QObject *parent = 0);

    bool load();
    
    bool isEnabled() { return _enabled; };
    void setEnabled(bool b) { _enabled = b; };

    QString id() { return _id; };

    QString name() const;
    QString version() const;
    QString description() const;
    QString icon() const;
    
    KAction *browserAction() { return _browserAction; };
    KAction *pageAction() { return _pageAction; };

private:
    void init();

private Q_SLOTS:
    void triggerExtension();
    void triggerPopup();
    
private:
    QVariantMap _manifest;
    
    QString _id;

    KAction *_browserAction;
    KAction *_pageAction;

    bool _enabled;
};


#endif // EXTENSION_MANAGER_H
