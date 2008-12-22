/* ============================================================
 *
 * This file is a part of the rekonq project
 *
 * Copyright (C) 2007-2008 Trolltech ASA. All rights reserved
 * Copyright (C) 2008 by Andrea Diamantini <adjam7 at gmail dot com>
 *
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef SETTINGS_H
#define SETTINGS_H

// Local Includes
#include "ui_settings.h"

// KDE Includes
#include <KConfigDialog>

// Qt Includes
#include <QWidget>

class Private;

class SettingsDialog : public KConfigDialog
{
    Q_OBJECT

public:
    SettingsDialog(QWidget *parent = 0);
    ~SettingsDialog();

private:
    Private* const d;
    
private slots:
    void loadDefaults();
    void loadFromSettings();
    void saveToSettings();

    void setHomeToCurrentPage();
    void showCookies();
    void showExceptions();

    void chooseFont();
    void chooseFixedFont();

    void slotOk();
    void slotApply();

private:
    QFont m_standardFont;
    QFont m_fixedFont;
};

#endif // SETTINGS_H

