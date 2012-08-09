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
#include "rekonqmenu.h"

#include <KActionCollection>
#include <KMenu>
#include <KStandardDirs>
#include <KToolBar>

#include <QFile>
#include <QString>
#include <QWidget>

#include <QDomDocument>
#include <QDomElement>


QWidget *RekonqFactory::createWidget(const QString &name, QWidget *parent, KActionCollection *ac)
{
    QString xmlFilePath;
    xmlFilePath = KStandardDirs::locateLocal( "data", "rekonq/rekonqui.rc");
    if (!QFile::exists(xmlFilePath))
    {
        xmlFilePath = KStandardDirs::locate( "data", "rekonq/rekonqui.rc");
        kDebug() << "general xmlfile: " << xmlFilePath;
    }

    QFile xmlFile(xmlFilePath);

    QDomDocument document("rekonqui.rc");
    document.setContent(&xmlFile, false);

    // Toolbars ----------------------------------------------------------------------
    QDomNodeList elementToolbarList = document.elementsByTagName(QL1S("ToolBar"));
    if (elementToolbarList.isEmpty())
    {
        kDebug() << "ELEMENT TOOLBAR LIST EMPTY. RETURNING NULL";
        return 0;
    }

    for(unsigned int i = 0; i < elementToolbarList.length(); ++i)
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
            fillToolbar(b, node, ac);
            return b;
        }
        else
        {
            KToolBar *b = new KToolBar(parent, false , false);
            fillToolbar(b, node, ac);
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

    for(unsigned int i = 0; i < elementMenuList.length(); ++i)
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
            fillMenu(m, node, ac);
            return m;
        }
        else
        {
            KMenu *m = new KMenu(parent);
            fillMenu(m, node, ac);
            return m;
        }
        
    }
    
    kDebug() << "NO WIDGET RETURNED";
    return 0;
}


void RekonqFactory::fillToolbar(KToolBar *b, QDomNode node, KActionCollection *ac)
{
    QDomElement element = node.toElement();
    
    if (element.hasAttribute("iconSize"))
    {
        int iconSize = element.attribute("iconSize").toInt();
        b->setIconDimensions(iconSize);
    }

    if (element.hasAttribute("iconText"))
    {
        if(element.attribute("iconText").toLower() == QL1S("icononly"))
        {
            b->setToolButtonStyle(Qt::ToolButtonIconOnly);
        }

        if(element.attribute("iconText").toLower() == QL1S("textonly"))
        {
            b->setToolButtonStyle(Qt::ToolButtonTextOnly);
        }

        if(element.attribute("iconText").toLower() == QL1S("icontextright"))
        {
            b->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        }

        if(element.attribute("iconText").toLower() == QL1S("textundericon"))
        {
            b->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        }

        if(element.attribute("iconText").toLower() == QL1S("followstyle"))
        {
            b->setToolButtonStyle(Qt::ToolButtonFollowStyle);
        }
    }

    QDomNodeList childrenList = node.childNodes();

    for(unsigned int i = 0; i < childrenList.length(); ++i)
    {
        QDomElement el = childrenList.at(i).toElement();

        if (el.tagName() == QL1S("Action"))
        {
            const QString actionName = el.attribute("name");
            QAction *a = ac->action(actionName);
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


void RekonqFactory::fillMenu(KMenu *m, QDomNode node, KActionCollection *ac)
{
    QDomNodeList childrenList = node.childNodes();

    for(unsigned int i = 0; i < childrenList.length(); ++i)
    {
        QDomElement el = childrenList.at(i).toElement();

        if (el.tagName() == QL1S("Action"))
        {
            const QString actionName = el.attribute("name");
            QAction *a = ac->action(actionName);
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
            KMenu *subm = qobject_cast<KMenu *>(createWidget(menuName, m, ac));
            m->addMenu(subm);
        }

        if (el.tagName() == QL1S("text"))
        {
            const QString menuTitle = el.text();
            m->setTitle(menuTitle);
        }

    }
}

