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


#ifndef PRIVACY_WIDGET_H
#define PRIVACY_WIDGET_H


// Rekonq Includes
#include "rekonq_defines.h"

// Ui Includes
#include "ui_settings_privacy.h"

// Qt Includes
#include <QWidget>


class PrivacyWidget : public QWidget, private Ui::privacy
{
    Q_OBJECT

public:
    explicit PrivacyWidget(QWidget *parent = 0);

    void save();
    void reload();

    bool changed();

Q_SIGNALS:
    void changed(bool);

private Q_SLOTS:
    void hasChanged();

    void launchCacheSettings();
    void launchCookieSettings();
    void showPassExceptions();

private:
    bool _changed;
};

#endif // PRIVACY_WIDGET_H
