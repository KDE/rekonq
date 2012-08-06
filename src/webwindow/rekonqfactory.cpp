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
    kDebug() << "local xmlfile: " << xmlFilePath;
    if (!QFile::exists(xmlFilePath))
    {
        xmlFilePath = KStandardDirs::locate( "data", "rekonq/rekonqui.rc");
        kDebug() << "general xmlfile: " << xmlFilePath;
    }

    QFile xmlFile(xmlFilePath);

    QDomDocument document("rekonqui.rc");
    document.setContent(&xmlFile, false);

    // Toolbars ----------------------------------------------------------------------
    QDomNodeList elementList = document.elementsByTagName(QL1S("ToolBar"));
    if (elementList.isEmpty())
    {
        return 0;
    }

    for(unsigned int i = 0; i < elementList.length(); ++i)
    {
        QDomElement element = elementList.at(i).toElement();

        if (element.attribute("name") != name)
            continue;

        if (element.attribute("deleted").toLower() == "true")
        {
            return 0;
        }
        KToolBar *bar = new KToolBar(parent, false, false);
        QDomNodeList actionList = element.elementsByTagName(QL1S("Action"));
        
        for(unsigned int j = 0; j < actionList.length(); ++j)
        {
            QDomElement actionElement = actionList.at(j).toElement();
            const QString actionName = actionElement.attribute("name");
            QAction *a = ac->action(actionName);
            if (a)
                bar->addAction(a);
        }

        return bar;
    }

    // Rekonq Menu ----------------------------------------------------------------------
    QDomNodeList elementMenuList = document.elementsByTagName(QL1S("Menu"));
    if (elementMenuList.isEmpty())
    {
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
            return 0;
        }

        if (name == QL1S("rekonqMenu"))
        {
            RekonqMenu *m = new RekonqMenu(parent);
            kDebug() << "filling menu...";
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
                kDebug() << "ADDING ACTION " << actionName << " to menu " << m;
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

