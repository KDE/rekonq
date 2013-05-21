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


#ifndef EXTENSION_MANAGER_H
#define EXTENSION_MANAGER_H

// Rekonq Includes
#include "rekonq_defines.h"

// Local Includes
#include "jsapi.h"

// Qt Includes
#include <QObject>
#include <QWeakPointer>
#include <QAction>
#include <QWebFrame>

// Forward Declarations
class Extension;

// Typedefs
typedef QHash<QString, Extension*> ExtensionMap;
typedef QHash<QString, Extension*>::const_iterator ExtensionIterator;
typedef QList<Extension *> ExtensionList;


class REKONQ_TESTS_EXPORT ExtensionManager : public QObject
{
    Q_OBJECT

public:
    /**
     * Entry point.
     * Access to ExtensionManager class by using
     * ExtensionManager::self()->thePublicMethodYouNeed()
     */
    static ExtensionManager *self();

    void initExtensions();

    /**
     * Returns an unsorted list of the extensions
     */
    ExtensionList extensionList();

    QList<QAction *> browserActionList();
    QList<QAction *> pageActionList();

    void addExtensionCapabilitiesToFrame(QWebFrame *);

    /**
     * Create an Extension object pointer, load the
     * extension identified by id and add it into ExtensionMap
     */
    Extension* loadExtension(const QString& extPath, const QString& id);

    /**
     * unload the extension identified by id
     * and remove it from the ExtensionMap
     */
    bool unloadExtension(const QString& id);

private:
    explicit ExtensionManager(QObject *parent = 0);    

private Q_SLOTS:
    void showSettings();
    
private:
    ExtensionMap _extensionMap;
    
    static QWeakPointer<ExtensionManager> s_extensionManager;
};


#endif // EXTENSION_MANAGER_H
