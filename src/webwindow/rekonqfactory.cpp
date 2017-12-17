/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2012-2014 by Andrea Diamantini <adjam7 at gmail dot com>
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

// KDE Includes
#include <KAboutData>
#include <KActionCollection>
#include <KHelpMenu>
#include <KLocalizedString>
#include <KToolBar>

// Qt Includes
#include <QFile>
#include <QMenu>
#include <QStandardPaths>
#include <QString>

#include <QWidget>
#include <QMenuBar>

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
        qDebug() << "Unable to open xml file" << sessionFile.fileName();
        return false;
    }

    if (!document.setContent(&sessionFile, false))
    {
        qDebug() << "Unable to parse xml file" << sessionFile.fileName();
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

    qDebug() << "NO ACTION FOUND: " << name;
    return 0;
}

// ---------------------------------------------------------------------------------------------------------


QWidget *RekonqFactory::createWidget(const QString &name, QWidget *parent)
{
    QDomDocument document( QL1S("rekonqui.rc") );
    QString xmlFilePath = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QL1S("/kxmlgui5/rekonq/rekonqui.rc") );

    if (!readDocument(document, xmlFilePath))
        return 0;

    // Toolbars ----------------------------------------------------------------------
    QDomNodeList elementToolbarList = document.elementsByTagName(QL1S("ToolBar"));
    if (elementToolbarList.isEmpty())
    {
        qDebug() << "ELEMENT TOOLBAR LIST EMPTY. RETURNING NULL";
        return 0;
    }

    for (int i = 0; i < elementToolbarList.length(); ++i)
    {
        QDomNode node = elementToolbarList.at(i);
        QDomElement element = node.toElement();

        if (element.attribute( QL1S("name") ) != name)
            continue;

        if (element.attribute( QL1S("deleted") ).toLower() == QL1S("true") )
        {
            qDebug() << "ELEMENT DELETED. RETURNING NULL";
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
        qDebug() << "ELEMENT MENU LIST EMPTY. RETURNING NULL";
        return 0;
    }

    for (int i = 0; i < elementMenuList.length(); ++i)
    {
        QDomNode node = elementMenuList.at(i);
        QDomElement element = node.toElement();
        if (element.attribute( QL1S("name") ) != name)
            continue;

        if (element.attribute( QL1S("deleted") ).toLower() == QL1S("true") )
        {
            qDebug() << "ELEMENT DELETED. RETURNING NULL";
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
            KHelpMenu *m = new KHelpMenu(parent, KAboutData::applicationData());
            return m->menu();
        }
        else
        {
            QMenu *m = new QMenu(parent);
            fillMenu(m, node);
            return m;
        }

    }

    // MenuBar ----------------------------------------------------------------------
    QDomNodeList elementMenuBarList = document.elementsByTagName(QL1S("MenuBar"));
    if (elementMenuBarList.isEmpty())
    {
        qDebug() << "ELEMENT MENUBAR LIST EMPTY. RETURNING NULL";
        return 0;
    }
    
    if (name == QL1S("menuBar"))
    {
        QDomNode node = elementMenuBarList.at(0);
        QDomNodeList menuNodes = node.childNodes();

        QMenuBar *menuBar = new QMenuBar(parent);
        for (int i = 0; i < menuNodes.length(); ++i)
        {
            QDomNode node = menuNodes.at(i);
            if (node.isComment())
                continue;
            
            QDomElement element = node.toElement();
            
            if (element.attribute( QL1S("deleted") ).toLower() == QL1S("true") )
                continue;
            
            if (element.attribute( QL1S("name") ) == QL1S("help"))
            {
                KHelpMenu *m = new KHelpMenu(parent, KAboutData::applicationData());
                menuBar->addMenu(m->menu());
                continue;
            }
            
            QMenu *m = new QMenu(parent);
            fillMenu(m, node);
            menuBar->addMenu(m);
        }
        
        menuBar->hide();
        return menuBar;
    }

    qDebug() << "NO WIDGET RETURNED";
    return 0;
}


void RekonqFactory::updateWidget(QWidget *widg, const QString &name)
{
    QDomDocument document( QL1S("rekonqui.rc") );
    QString xmlFilePath = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QL1S("/kxmlgui5/rekonq/rekonqui.rc") );

    if (!readDocument(document, xmlFilePath))
        return;

    // Toolbars ----------------------------------------------------------------------
    QDomNodeList elementToolbarList = document.elementsByTagName(QL1S("ToolBar"));
    if (elementToolbarList.isEmpty())
    {
        qDebug() << "ELEMENT TOOLBAR LIST EMPTY. RETURNING NULL";
        return;
    }

    for (int i = 0; i < elementToolbarList.length(); ++i)
    {
        QDomNode node = elementToolbarList.at(i);
        QDomElement element = node.toElement();

        if (element.attribute( QL1S("name") ) != name)
            continue;

        if (element.attribute( QL1S("deleted") ).toLower() == QL1S("true") )
        {
            return;
        }

        if (name == QL1S("mainToolBar"))
        {
            fillToolbar(qobject_cast<MainToolBar *>(widg), node);
            return;
        }
    }

    qDebug() << "NO WIDGET RETURNED";
    return;
}


void RekonqFactory::fillToolbar(KToolBar *b, QDomNode node)
{
    b->clear();
    
    QDomElement element = node.toElement();

    if (element.hasAttribute( QL1S("iconSize") ))
    {
        int iconSize = element.attribute( QL1S("iconSize") ).toInt();
        b->setIconDimensions(iconSize);
    }

    if (element.hasAttribute( QL1S("iconText") ))
    {
        if (element.attribute( QL1S("iconText") ).toLower() == QL1S("icononly"))
        {
            b->setToolButtonStyle(Qt::ToolButtonIconOnly);
        }

        if (element.attribute( QL1S("iconText") ).toLower() == QL1S("textonly"))
        {
            b->setToolButtonStyle(Qt::ToolButtonTextOnly);
        }

        if (element.attribute( QL1S("iconText") ).toLower() == QL1S("icontextright"))
        {
            b->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        }

        if (element.attribute( QL1S("iconText") ).toLower() == QL1S("textundericon"))
        {
            b->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        }

        if (element.attribute( QL1S("iconText") ).toLower() == QL1S("followstyle"))
        {
            b->setToolButtonStyle(Qt::ToolButtonFollowStyle);
        }
    }

    QDomNodeList childrenList = node.childNodes();

    for (int i = 0; i < childrenList.length(); ++i)
    {
        QDomElement el = childrenList.at(i).toElement();

        if (el.tagName() == QL1S("Action"))
        {
            const QString actionName = el.attribute( QL1S("name") );
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


void RekonqFactory::fillMenu(QMenu *m, QDomNode node)
{
    QDomNodeList childrenList = node.childNodes();

    for (int i = 0; i < childrenList.length(); ++i)
    {
        QDomElement el = childrenList.at(i).toElement();

        if (el.tagName() == QL1S("Action"))
        {
            const QString actionName = el.attribute( QL1S("name") );
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
            const QString menuName = el.attribute( QL1S("name") );
            QMenu *subm = qobject_cast<QMenu *>(createWidget(menuName, m));
            m->addMenu(subm);
        }

        if (el.tagName() == QL1S("text"))
        {
            const QString menuTitle = i18n(el.text().toUtf8().constData());
            m->setTitle(menuTitle);
        }

    }
}
