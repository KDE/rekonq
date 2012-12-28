/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2009-2012 by Andrea Diamantini <adjam7 at gmail dot com>
* Copyright (C) 2010 by Matthieu Gicquel <matgic78 at gmail dot com>
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


#ifndef WEB_PLUGIN_FACTORY_H
#define WEB_PLUGIN_FACTORY_H


// Rekonq Includes
#include "rekonq_defines.h"

// KDE Includes
#include <KWebPluginFactory>


class REKONQ_TESTS_EXPORT WebPluginFactory : public KWebPluginFactory
{
    Q_OBJECT

public:
    explicit WebPluginFactory(QObject *parent);

    virtual QObject *create(const QString &_mimeType,
                            const QUrl &url,
                            const QStringList &argumentNames,
                            const QStringList &argumentValues) const;

Q_SIGNALS:
    void signalLoadClickToFlash(bool) const;

public Q_SLOTS:
    void setLoadClickToFlash(bool load);

private:
    /**
        When true, force loading of next flash animation (don't show clicktoflash)
        We use signals/slots to set this property because QWebPluginFactory::create is const
    */
    bool _loadClickToFlash;
};

#endif // WEB_PLUGIN_FACTORY_H
