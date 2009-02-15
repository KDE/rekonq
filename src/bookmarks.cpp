/* ============================================================
 *
 * This file is a part of the rekonq project
 *
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
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


void OwnBookMarks::openBookmark (const KBookmark & b, Qt::MouseButtons , Qt::KeyboardModifiers )
{
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


BookmarksProvider::BookmarksProvider(KMainWindow* parent)
    : m_parent(parent)
    , m_owner(new OwnBookMarks(parent)) 
{
    KUrl bookfile = KUrl( "~/.kde/share/apps/konqueror/bookmarks.xml" );    // share konqueror bookmarks
    m_manager = KBookmarkManager::managerForExternalFile( bookfile.path() );
    m_ac = new KActionCollection( this );
}


void BookmarksProvider::provideBmToolbar(KToolBar* toolbar)
{
    toolbar->setToolButtonStyle( Qt::ToolButtonTextBesideIcon );
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
    KMenu *bmMenu = new KMenu(m_parent);
    new KBookmarkMenu( m_manager, m_owner, bmMenu, m_ac );
    return bmMenu;
}

