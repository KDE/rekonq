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


#ifndef WEB_PLUGIN_FACTORY_H
#define WEB_PLUGIN_FACTORY_H


// KDE Includes
#include <KWebPluginFactory>

// Qt Includes
#include <QtCore/QList>
#include <QtGui/QWidget>


class WebPluginFactory : public KWebPluginFactory
{
Q_OBJECT

public:
    WebPluginFactory(QObject *parent);
    ~WebPluginFactory();

    virtual QObject *create(const QString &mimeType,
                            const QUrl &url,
                            const QStringList &argumentNames,
                            const QStringList &argumentValues) const;

    virtual QList<Plugin> plugins() const;
    
signals:

    void signalLoadClickToFlash(bool) const;
    
public slots:
    void setLoadClickToFlash(bool load);
    
private:
    /**
        When true, force loading of next flash animation (don't show clicktoflash)
        We use signals/slots to set this property because QWebPluginFactory::create is const
    */
    bool loadClickToFlash;
};

#endif // WEB_PLUGIN_FACTORY_H
