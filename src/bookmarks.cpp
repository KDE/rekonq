/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2007-2008 Trolltech ASA. All rights reserved
* Copyright (C) 2008 by Andrea Diamantini <adjam7 at gmail dot com>
*
*
* This program is free software; you can redistribute it
* and/or modify it under the terms of the GNU General
* Public License as published by the Free Software Foundation;
* either version 2, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* ============================================================ */



// Local Includes
#include "bookmarks.h"
#include "bookmarks.moc"

#include "mainwindow.h"
#include "webview.h"

// KDE Includes
#include <KMimeType>
#include <KMenu>

OwnBookMarks::OwnBookMarks(KMainWindow *parent)
    : QObject(parent)
    , KBookmarkOwner()
{
    m_parent = qobject_cast<MainWindow*>( parent );
    connect( this, SIGNAL( openUrl( const KUrl &) ) , parent , SLOT( loadUrl( const KUrl & ) ) );
}


void OwnBookMarks::openBookmark (const KBookmark & b, Qt::MouseButtons mb, Qt::KeyboardModifiers km)
{
    Q_UNUSED(mb);
    Q_UNUSED(km);
    emit openUrl( b.url() );
}

QString OwnBookMarks::currentUrl() const
{
    QUrl url = m_parent->currentTab()->url();
    return url.path();
}


QString OwnBookMarks::currentTitle() const
{
    QString title = m_parent->windowTitle();
    return title.remove( " - rekonq" );
}


// ------------------------------------------------------------------------------------------------------


BookmarksMenu::BookmarksMenu( KBookmarkManager* manager, KBookmarkOwner* owner, KMenu* menu, KActionCollection* ac )
    : KBookmarkMenu(manager, owner, menu, ac)
{    
}

KMenu* BookmarksMenu::viewContextMenu(QAction* action)
{
    return contextMenu( action );
}


// ------------------------------------------------------------------------------------------------------


BookmarksProvider::BookmarksProvider(KMainWindow* parent)
    : m_parent(parent)
    , m_owner(new OwnBookMarks(parent))
    , m_bmMenu(0)
    , m_bmToolbar(0)
{
    KUrl bookfile = KUrl( "~/.kde/share/apps/konqueror/bookmarks.xml" );    // share konqueror bookmarks
    m_manager = KBookmarkManager::managerForExternalFile( bookfile.path() );
    m_ac = new KActionCollection( this );
}


void BookmarksProvider::provideBmToolbar(KToolBar* toolbar)
{
    m_bmToolbar = toolbar;
    toolbar->setToolButtonStyle( Qt::ToolButtonTextBesideIcon );

    toolbar->setContextMenuPolicy( Qt::CustomContextMenu );
    connect( toolbar, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(contextMenu(const QPoint &)) );

    KBookmarkGroup toolbarGroup = m_manager->toolbar();
    KBookmark bm = toolbarGroup.first();
    while(!bm.isNull())
    {
        if(bm.isGroup())
        {
            // do nothing!
        }
        else
        {
            if(bm.isSeparator())
            {
                toolbar->addSeparator();
            }
            else
            {
                KAction *a = new KBookmarkAction(bm, m_owner, m_ac);
                toolbar->addAction(a);
            }
        }
        // go ahead!
        bm = toolbarGroup.next(bm);
    }
}


KMenu *BookmarksProvider::bookmarksMenu()
{
    KMenu *menu = new KMenu(m_parent);
    m_bmMenu = new BookmarksMenu( m_manager, m_owner, menu, m_ac );
    return menu;
}


void BookmarksProvider::contextMenu(const QPoint & point)
{
    KAction* action = dynamic_cast<KAction*>( m_bmToolbar->actionAt( point ) );
    if(!action)
        return;
    KMenu *menu = m_bmMenu->viewContextMenu(action);
    menu->setAttribute(Qt::WA_DeleteOnClose);
    menu->popup( m_bmToolbar->mapToGlobal( point ));
}


