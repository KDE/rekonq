/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2009 by Andrea Diamantini <adjam7 at gmail dot com>
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


#ifndef KCM_WEBKIT_ADBLOCK_H
#define KCM_WEBKIT_ADBLOCK_H


// UI Includes
#include "ui_webkitadblock.h"

// KDE Includes
#include <kcmodule.h>
#include <ksharedconfig.h>


class KCMWebkitAdblock : public KCModule, private Ui::WebkitAdblock
{
Q_OBJECT

public:
    KCMWebkitAdblock(QWidget *parent, const QVariantList &args);
    ~KCMWebkitAdblock();
    
    void defaults();
    void load();
    void save();
    
private slots:
    void infoLinkActivated(const QString &url);

    void addExpr();
    void removeSelected();
    void importExpr();
    
private:
    KSharedConfig::Ptr _config;
    QString _group;
};

#endif
