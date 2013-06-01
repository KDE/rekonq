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
#include "extensionpopup.h"

// KDE Includes
#include <KStandardDirs>
#include <KUrl>

// Qt Includes
#include <QCursor>
#include <QFile>
#include <QTextStream>

// 3rd party
#include <qjson/parser.h> // QJson
#include <qjson/serializer.h>


Extension::Extension(const QString &extPath, const QString &id,  bool enabled, QObject *parent)
    : QObject(parent)
    , _extensionPath(extPath)
    , _id(id)
    , _browserAction(0)
    , _pageAction(0)
    , _enabled(enabled)
{
}


bool Extension::load()
{
    // parse manifest.json
    QFile manjson(_extensionPath + _id + QL1S("/manifest.json"));
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
    // Browser Actions
    QVariant browserActionVar = _manifest["browser_action"];
    if (!browserActionVar.isNull())
    {
        QVariantMap browserActionMap = browserActionVar.toMap();
        
        KIcon icon;
        QString defIcon = browserActionMap["default_icon"].toString();
        if (defIcon.isEmpty())
        {
            icon = KIcon("edit-bomb");
        }
        else
        {      
            QString iconPath = _extensionPath + _id + QL1C('/') + defIcon;
            icon = KIcon(QIcon(iconPath));
        }
        
        QString title = browserActionMap["default_title"].toString();

        _browserAction = new KAction(icon, title, this);
        
        QVariant popupAction = browserActionMap["default_popup"];
        if (popupAction.isNull())
            connect(_browserAction, SIGNAL(triggered()), this, SLOT(triggerExtension()));
        else
            connect(_browserAction, SIGNAL(triggered()), this, SLOT(triggerBrowserActionPopup()));
    }

    // Page Actions
    QVariant pageActionVar = _manifest["page_action"];
    if (!pageActionVar.isNull())
    {
        QVariantMap pageActionMap = pageActionVar.toMap();

        KIcon icon;
        QString defIcon = pageActionMap["default_icon"].toString();
        if (defIcon.isEmpty())
        {
            icon = KIcon("edit-bomb");
        }
        else
        {      
            QString iconPath = _extensionPath + _id + QL1C('/') + defIcon;
            icon = KIcon(QIcon(iconPath));
        }

        QString title = pageActionMap["default_title"].toString();

        _pageAction = new KAction(icon, title, this);
        QVariant popupAction = pageActionMap["default_popup"];
        if (popupAction.isNull())
            connect(_pageAction, SIGNAL(triggered()), this, SLOT(triggerExtension()));
        else
            connect(_pageAction, SIGNAL(triggered()), this, SLOT(triggerPageActionPopup()));
    }
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
    
    return icon;
}


void Extension::triggerExtension()
{
    if (!isEnabled())
        return;

    QVariant backgroundVar = _manifest["background"];
    QVariantMap backgroundMap = backgroundVar.toMap();
    
    bool isPersistent = backgroundMap["persistent"].toBool();
    QStringList scripts = backgroundMap["scripts"].toStringList();
    kDebug() << "isPersistent: " << isPersistent;
    kDebug() << "scripts: " << scripts;
}


void Extension::triggerBrowserActionPopup()
{
    if (!isEnabled())
        return;
    
    QVariant browserActionVar = _manifest["browser_action"];
    QVariantMap browserActionMap = browserActionVar.toMap();
    QVariant popupFile = browserActionMap["default_popup"];

    KUrl u = KUrl(_extensionPath + _id + '/' + popupFile.toString());
    kDebug() << "Url: " << u;
    launchPopup(u);
}


void Extension::triggerPageActionPopup()
{
    if (!isEnabled())
        return;
    
    QVariant pageActionVar = _manifest["page_action"];
    QVariantMap pageActionMap = pageActionVar.toMap();
    QVariant popupFile = pageActionMap["default_popup"];

    KUrl u = KUrl(_extensionPath + _id + '/' + popupFile.toString());
    kDebug() << "Url: " << u;
    launchPopup(u);    
}


void Extension::launchPopup(const KUrl &u)
{
    ExtensionPopup *popup = new ExtensionPopup(u);
    popup->showAt(QCursor::pos());    
}
