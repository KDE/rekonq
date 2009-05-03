/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2009 by Paweł Prażak <pawelprazak at gmail dot com>
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


#ifndef SIDEPANEL_H
#define SIDEPANEL_H

// Qt Includes
#include <QDockWidget>

// Local Includes
#include "application.h"

class KUrl;
class PanelHistory;


class SidePanel : public QDockWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(SidePanel)

public:
    explicit SidePanel(const QString &title, QWidget *parent = 0, Qt::WindowFlags flags = 0);
    ~SidePanel();

signals:
    void openUrl(const KUrl &);

private:
    PanelHistory *m_panelHistory;

};

#endif // SIDEPANEL_H
