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
#include "extensionmanager.h"
#include "extensionmanager.moc"

// Local Includes
#include "extension.h"
#include "extensionwidget.h"

#include "application.h"
#include "rekonqwindow.h"

// KDE Includes
#include <KDialog>
#include <KLocalizedString>
#include <KStandardDirs>

// Qt Includes
#include <QDir>
#include <QPointer>


// ----------------------------------------------------------------------------------------------


QWeakPointer<ExtensionManager> ExtensionManager::s_extensionManager;


ExtensionManager *ExtensionManager::self()
{
    if (s_extensionManager.isNull())
    {
        s_extensionManager = new ExtensionManager(qApp);
    }
    return s_extensionManager.data();
}


// ----------------------------------------------------------------------------------------------



ExtensionManager::ExtensionManager(QObject *parent)
    : QObject(parent)
{
    initExtensions();
}


void ExtensionManager::initExtensions()
{
    // FIXME: Just for the first tests...
    QString extensionPath = KStandardDirs::locateLocal("appdata" , "extensions");
    if (!QFile::exists(extensionPath))
        return;
           
    _extensionConfig = KSharedConfig::openConfig("extensions", KConfig::SimpleConfig, "appdata");
    
    QStringList extIdList = _extensionConfig->groupList();
    
    Q_FOREACH(const QString &idString, extIdList)
    {
        KConfigGroup extGroup(_extensionConfig, idString);
        QString extPath = extGroup.readEntry("path", QString());
        bool enabled = extGroup.readEntry("enabled", false);
        Extension *ext = addExtension(extPath, idString, enabled);
    }
}


void ExtensionManager::showSettings()
{
    QPointer<KDialog> dialog = new KDialog(rApp->rekonqWindow());
    dialog->setCaption(i18nc("@title:window", "Extension Manager"));
    dialog->setButtons(KDialog::Ok | KDialog::Cancel);

    ExtensionWidget widget;

    dialog->setMainWidget(&widget);
    dialog->exec();
    dialog->deleteLater();
   
    rApp->rekonqWindow()->updateToolBars();
}


ExtensionList ExtensionManager::extensionList()
{
    ExtensionList list;

    ExtensionIterator it;
    for(it = _extensionMap.constBegin(); it != _extensionMap.constEnd(); ++it)
    {
        list << it.value();
    }

    return list;
}


Extension *ExtensionManager::extension(const QString &id)
{
    Extension *ext = _extensionMap[id];
    return ext;
}


Extension* ExtensionManager::addExtension(const QString& extPath, const QString& id, bool enabled)
{
    Extension* ext = new Extension(extPath, id, enabled, this);
    if (ext->load())
    {
        kDebug() << "Loaded extension with id: " << id;
        _extensionMap[id] = ext;
        return ext;
    }
    kDebug() << "Problems load extension with id: " << id;
    return 0;
}


QList<QAction *> ExtensionManager::browserActionList()
{
    QList<QAction *> actionList;
    Q_FOREACH(Extension *ext, _extensionMap)
    {
        if (!ext->isEnabled())
            continue;
        
        KAction *a = ext->browserAction();
        if (a)
            actionList << a;
    }

    return actionList;
}


QList<QAction *> ExtensionManager::pageActionList()
{
    QList<QAction *> actionList;
    Q_FOREACH(Extension *ext, _extensionMap)
    {
        if (!ext->isEnabled())
            continue;
        
        KAction *a = ext->pageAction();
        if (a)
            actionList << a;
    }

    return actionList;
}


void ExtensionManager::addExtensionCapabilitiesToFrame(QWebFrame *frame)
{
    JSApi *apiObj = new JSApi;
    frame->addToJavaScriptWindowObject("chrome", apiObj);
}
