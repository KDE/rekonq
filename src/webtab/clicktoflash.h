/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2009 by Benjamin C. Meyer <ben@meyerhome.net>
* Copyright (C) 2010-2011 by Matthieu Gicquel <matgic78@gmail.com>
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


#ifndef CLICKTOFLASH_H
#define CLICKTOFLASH_H


// Rekonq Includes
#include "rekonq_defines.h"

// Qt Includes
#include <QUrl>
#include <QWidget>

// Forward Declarations
class QWebElement;


class REKONQ_TESTS_EXPORT ClickToFlash : public QWidget
{
    Q_OBJECT

public:
    explicit ClickToFlash(const QUrl &pluginUrl, QWidget *parent = 0);

Q_SIGNALS:
    void signalLoadClickToFlash(bool);

private Q_SLOTS:
    void load();

private:
    bool checkElement(QWebElement el);

    /**
    used to find the right QWebElement between the ones of the different plugins
    */
    const QUrl m_url;
};

#endif // CLICKTOFLASH_H
