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


#include "rekonqfactory.h"

#include "bookmarkstoolbar.h"
#include "maintoolbar.h"
#include "rekonqmenu.h"

#include <KActionCollection>
#include <KCmdLineArgs>
#include <KMenu>
#include <KHelpMenu>
#include <KStandardDirs>
#include <KToolBar>

#include <QFile>
#include <QMenuBar>
#include <QString>
#include <QWidget>

#include <QDomDocument>
#include <QDomElement>


// Only used internally
bool readDocument(QDomDocument & document, const QString & filePath)
{
    QFile sessionFile(filePath);

    if (!sessionFile.exists())
        return false;

    if (!sessionFile.open(QFile::ReadOnly))
    {
        kDebug() << "Unable to open xml file" << sessionFile.fileName();
        return false;
    }

    if (!document.setContent(&sessionFile, false))
    {
        kDebug() << "Unable to parse xml file" << sessionFile.fileName();
        return false;
    }

    return true;
}


// Only used internally
QAction *actionByName(const QString &name)
{
    QList<KActionCollection *> lac = KActionCollection::allCollections();

    // NOTE: last action collection created is surely the one interests us more!
    // So let's start from the end...
    int lac_count = lac.count();
    for (int i = lac_count - 1; i >= 0; i--)
    {
        KActionCollection *ac = lac.at(i);
        QAction *a = ac->action(name);
        if (a)
            return a;
    }

    kDebug() << "NO ACTION FOUND: " << name;
    return 0;
}

// ---------------------------------------------------------------------------------------------------------


QWidget *RekonqFactory::createWidget(const QString &name, QWidget *parent)
{
    QDomDocument document("rekonqui.rc");
    QString xmlFilePath = KStandardDirs::locate("data", "rekonq/rekonqui.rc");

    if (!readDocument(document, xmlFilePath))
        return 0;

    // Toolbars ----------------------------------------------------------------------
    QDomNodeList elementToolbarList = document.elementsByTagName(QL1S("ToolBar"));
    if (elementToolbarList.isEmpty())
    {
        kDebug() << "ELEMENT TOOLBAR LIST EMPTY. RETURNING NULL";
        return 0;
    }

    for (unsigned int i = 0; i < elementToolbarList.length(); ++i)
    {
        QDomNode node = elementToolbarList.at(i);
        QDomElement element = node.toElement();

        if (element.attribute("name") != name)
            continue;

        if (element.attribute("deleted").toLower() == "true")
        {
            kDebug() << "ELEMENT DELETED. RETURNING NULL";
            return 0;
        }

        if (name == QL1S("bookmarkToolBar"))
        {
            BookmarkToolBar *b = new BookmarkToolBar(parent);
            fillToolbar(b, node);
            return b;
        }
        else
        {
            MainToolBar *b = new MainToolBar(parent);
            fillToolbar(b, node);
            return b;
        }
    }

    // Rekonq Menu ----------------------------------------------------------------------
    QDomNodeList elementMenuList = document.elementsByTagName(QL1S("Menu"));
    if (elementMenuList.isEmpty())
    {
        kDebug() << "ELEMENT MENU LIST EMPTY. RETURNING NULL";
        return 0;
    }

    for (unsigned int i = 0; i < elementMenuList.length(); ++i)
    {
        QDomNode node = elementMenuList.at(i);
        QDomElement element = node.toElement();
        if (element.attribute("name") != name)
            continue;

        if (element.attribute("deleted").toLower() == "true")
        {
            kDebug() << "ELEMENT DELETED. RETURNING NULL";
            return 0;
        }

        if (name == QL1S("rekonqMenu"))
        {
            RekonqMenu *m = new RekonqMenu(parent);
            fillMenu(m, node);
            return m;
        }
        else if (name == QL1S("help"))
        {
            KHelpMenu *m = new KHelpMenu(parent, KCmdLineArgs::aboutData());
            return m->menu();
        }
        else
        {
            KMenu *m = new KMenu(parent);
            fillMenu(m, node);
            return m;
        }

    }

    // MenuBar ----------------------------------------------------------------------
    QDomNodeList elementMenuBarList = document.elementsByTagName(QL1S("MenuBar"));
    if (elementMenuBarList.isEmpty())
    {
        kDebug() << "ELEMENT MENUBAR LIST EMPTY. RETURNING NULL";
        return 0;
    }
    
    if (name == QL1S("menuBar"))
    {
        QDomNode node = elementMenuBarList.at(0);
        QDomNodeList menuNodes = node.childNodes();

        QMenuBar *menuBar = new QMenuBar(parent);
        for (unsigned int i = 0; i < menuNodes.length(); ++i)
        {
            QDomNode node = menuNodes.at(i);
            if (node.isComment())
                continue;
            
            QDomElement element = node.toElement();
            
            if (element.attribute("deleted").toLower() == "true")
                continue;
            
            if (element.attribute("name") == QL1S("help"))
            {
                KHelpMenu *m = new KHelpMenu(parent, KCmdLineArgs::aboutData());
                menuBar->addMenu(m->menu());
                continue;
            }
            
            KMenu *m = new KMenu(parent);
            fillMenu(m, node);
            menuBar->addMenu(m);
        }
        
        menuBar->hide();
        return menuBar;
    }

    kDebug() << "NO WIDGET RETURNED";
    return 0;
}


void RekonqFactory::updateWidget(QWidget *widg, const QString &name)
{
    QDomDocument document("rekonqui.rc");
    QString xmlFilePath = KStandardDirs::locate("data", "rekonq/rekonqui.rc");

    if (!readDocument(document, xmlFilePath))
        return;

    // Toolbars ----------------------------------------------------------------------
    QDomNodeList elementToolbarList = document.elementsByTagName(QL1S("ToolBar"));
    if (elementToolbarList.isEmpty())
    {
        kDebug() << "ELEMENT TOOLBAR LIST EMPTY. RETURNING NULL";
        return;
    }

    for (unsigned int i = 0; i < elementToolbarList.length(); ++i)
    {
        QDomNode node = elementToolbarList.at(i);
        QDomElement element = node.toElement();

        if (element.attribute("name") != name)
            continue;

        if (element.attribute("deleted").toLower() == "true")
        {
            return;
        }

        if (name == QL1S("mainToolBar"))
        {
            fillToolbar(qobject_cast<MainToolBar *>(widg), node);
            return;
        }
    }

    kDebug() << "NO WIDGET RETURNED";
    return;
}


void RekonqFactory::fillToolbar(KToolBar *b, QDomNode node)
{
    b->clear();
    
    QDomElement element = node.toElement();

    if (element.hasAttribute("iconSize"))
    {
        int iconSize = element.attribute("iconSize").toInt();
        b->setIconDimensions(iconSize);
    }

    if (element.hasAttribute("iconText"))
    {
        if (element.attribute("iconText").toLower() == QL1S("icononly"))
        {
            b->setToolButtonStyle(Qt::ToolButtonIconOnly);
        }

        if (element.attribute("iconText").toLower() == QL1S("textonly"))
        {
            b->setToolButtonStyle(Qt::ToolButtonTextOnly);
        }

        if (element.attribute("iconText").toLower() == QL1S("icontextright"))
        {
            b->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        }

        if (element.attribute("iconText").toLower() == QL1S("textundericon"))
        {
            b->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        }

        if (element.attribute("iconText").toLower() == QL1S("followstyle"))
        {
            b->setToolButtonStyle(Qt::ToolButtonFollowStyle);
        }
    }

    QDomNodeList childrenList = node.childNodes();

    for (unsigned int i = 0; i < childrenList.length(); ++i)
    {
        QDomElement el = childrenList.at(i).toElement();

        if (el.tagName() == QL1S("Action"))
        {
            const QString actionName = el.attribute("name");
            QAction *a = actionByName(actionName);
            if (a)
            {
                b->addAction(a);
            }

        }

        if (el.tagName() == QL1S("Separator"))
        {
            b->addSeparator();
        }

    }
}


void RekonqFactory::fillMenu(KMenu *m, QDomNode node)
{
    QDomNodeList childrenList = node.childNodes();

    for (unsigned int i = 0; i < childrenList.length(); ++i)
    {
        QDomElement el = childrenList.at(i).toElement();

        if (el.tagName() == QL1S("Action"))
        {
            const QString actionName = el.attribute("name");
            QAction *a = actionByName(actionName);
            if (a)
            {
                m->addAction(a);
            }

        }

        if (el.tagName() == QL1S("Separator"))
        {
            m->addSeparator();
        }

        if (el.tagName() == QL1S("Menu"))
        {
            const QString menuName = el.attribute("name");
            KMenu *subm = qobject_cast<KMenu *>(createWidget(menuName, m));
            m->addMenu(subm);
        }

        if (el.tagName() == QL1S("text"))
        {
            const QString menuTitle = i18n(el.text().toUtf8().constData());
            m->setTitle(menuTitle);
        }

    }
}
