/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2009 by Andrea Diamantini <adjam7 at gmail dot com>
*
*
* This program is free software; you can redistribute it
* and/or modify it under the terms of the GNU General
* Public License as published by the Free Software Foundation;
* either version 2, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* ============================================================ */



#ifndef HISTORYDIALOG_H
#define HISTORYDIALOG_H


// Ui includes
#include "ui_history.h"

// KDE Includes
#include <KDialog>

class KUrl;
class QPoint;
class QWidget;
class HistoryManager;

class HistoryDialog : public KDialog
{
    Q_OBJECT

signals:
    void openUrl(const KUrl &url);

public:
    explicit HistoryDialog(QWidget *parent = 0, HistoryManager *history = 0);

private slots:
    void customContextMenuRequested(const QPoint &pos);
    void open();
    void copy();

private:
    Ui::historyWidget *m_historyWidg;
};

#endif
