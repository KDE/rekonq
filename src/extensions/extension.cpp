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


// Self Includes
#include "extension.h"
#include "extension.moc"

// Local Includes
#include "application.h"

// KDE Includes
#include <KStandardDirs>

// Qt Includes
#include <QFile>
#include <QTextStream>

// 3rd party
#include <qjson/parser.h> // QJson
#include <qjson/serializer.h>


Extension::Extension(const QString &id, QObject *parent)
    : QObject(parent)
    , _id(id)
    , _browserAction(0)
    , _pageAction(0)
    , _enabled(false)
{
}


bool Extension::load()
{
    QString extensionPath = KStandardDirs::locate("appdata" , "extensions/");

    // parse manifest.json
    QFile manjson(extensionPath + _id + QL1S("/manifest.json"));
    if (!manjson.open(QFile::ReadOnly))
    {
        kDebug() << "CANNOT LOAD EXT WITH ID: " << _id;
        return false;
    }

    QJson::Parser p;
    bool ok;
    QVariant data = p.parse(&manjson, &ok);

    if (!ok)
    {
        kDebug() << "Failed to parse manifest file "
            << manjson.fileName()
            << " error:" << p.errorLine()
            << ": " << p.errorString();
        return false;
    }

    _manifest = data.toMap();
    if (_manifest.empty())
    {
        kDebug() << "Invalid manifest: " << manjson.fileName();
        return false;
    }

    init();
    
    return true;
}


void Extension::init()
{
    QString extensionPath = KStandardDirs::locate("appdata" , "extensions/");
        
    // Browser Actions
    QVariant browserActionVar = _manifest["browser_action"];
    if (!browserActionVar.isNull())
    {
        QVariantMap browserActionMap = browserActionVar.toMap();
        
        QString defIcon = browserActionMap["default_icon"].toString();
        QString iconPath = extensionPath + _id + QL1C('/') + defIcon;
        kDebug() << "BROWSER ICON PATH: " << iconPath;
        KIcon icon = KIcon(QIcon(iconPath));
        
        QString title = browserActionMap["default_title"].toString();

        _browserAction = new KAction(icon, title, this);
        
        QVariant popupAction = browserActionMap["popup"];
        if (popupAction.isNull())
            connect(_browserAction, SIGNAL(triggered()), this, SLOT(triggerExtension()));
        else
            connect(_browserAction, SIGNAL(triggered()), this, SLOT(triggerPopup()));
    }

    // Page Actions
    QVariant pageActionVar = _manifest["page_action"];
    if (!pageActionVar.isNull())
    {
        QVariantMap pageActionMap = pageActionVar.toMap();

        QString defIcon = pageActionMap["default_icon"].toString();
        QString iconPath = extensionPath + _id + QL1C('/') + defIcon;
        kDebug() << "PAGE ICON PATH: " << iconPath;
        KIcon icon = KIcon(QIcon(iconPath));

        QString title = pageActionMap["default_title"].toString();

        _pageAction = new KAction(icon, title, this);
        connect(_pageAction, SIGNAL(triggered()), this, SLOT(triggerExtension()));
    }
    
    // Enabling it
    _enabled = true;
}
    

QString Extension::name() const
{
    return _manifest["name"].toString();
}


QString Extension::version() const
{
    return _manifest["version"].toString();
}


QString Extension::description() const
{
    return _manifest["description"].toString();
}


QString Extension::icon() const
{
    QVariant var = _manifest["icons"];
    QVariantMap iconMap = var.toMap();
    QString icon;
    icon = iconMap["128"].toString();
    if (icon.isEmpty())
        icon = iconMap["64"].toString();
    if (icon.isEmpty())
        icon = iconMap["32"].toString();
    
    return _id + QL1C('/') + icon;
}


void Extension::triggerExtension()
{
    kDebug() << "CIAO CIAO";    
}


void Extension::triggerPopup()
{
    QVariant browserActionVar = _manifest["browser_action"];
    QVariantMap browserActionMap = browserActionVar.toMap();
    QVariant popupFile = browserActionMap["popup"];

    QString extensionPath = KStandardDirs::locate("appdata" , "extensions/");
    KUrl u = KUrl(extensionPath + _id + '/' + popupFile.toString());
    kDebug() << "Url: " << u;
    rApp->loadUrl(u, Rekonq::NewWindow);
}
