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


// Self Includes
#include "webpluginfactory.h"
#include "webpluginfactory.moc"

#include "application.h"
#include "mainwindow.h"
#include "previewimage.h"

#include <KDebug>

WebPluginFactory::WebPluginFactory(QObject *parent)
    : QWebPluginFactory(parent)
{
}


WebPluginFactory::~WebPluginFactory()
{
}


QObject *WebPluginFactory::create(const QString &mimeType,
                                  const QUrl &url,
                                  const QStringList &argumentNames,
                                  const QStringList &argumentValues) const
{
    Q_UNUSED(url)
    
    kDebug() << "loading mimeType: " << mimeType;
    
    if(mimeType == QString("application/image-preview") )
    {   
        QString imageUrl;
        for(int i = 0; i<argumentNames.count(); ++i)
        {
            if(argumentNames.at(i) == QString("url"))
            {
                imageUrl = argumentValues.at(i);
                break;
            }
        }
        return new PreviewImage(imageUrl);
    }

    // this let QtWebKit using builtin plugins 
    // to load in example flash contents and so on..
    kDebug() << "No plugins found for" << mimeType << ". Falling back to QtWebKit ones...";
    return 0;
}


QList<QWebPluginFactory::Plugin> WebPluginFactory::plugins() const
{
    QList<QWebPluginFactory::Plugin> plugins;
    
    QWebPluginFactory::Plugin p;
    p.name = "application/image-preview";
    p.description = "plugin for embedding Web snapped images";
    plugins.append(p);
    
    return plugins;
}
