/* ============================================================
 *
 * This file is a part of the reKonq project
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

#include "browsermainwindow.h"
#include "webview.h"

// KDE Includes
#include <KMimeType>

OwnBookMarks::OwnBookMarks(KMainWindow *parent)
    : QObject(parent)
    , KBookmarkOwner()
{
    m_parent = qobject_cast<BrowserMainWindow*>( parent );
    connect( this, SIGNAL( openUrl( const QUrl &) ) , parent , SLOT( loadUrl( const QUrl & ) ) );
}


void OwnBookMarks::openBookmark (const KBookmark & b, Qt::MouseButtons , Qt::KeyboardModifiers )
{
    emit openUrl( (QUrl)b.url() );
}

QString OwnBookMarks::currentUrl() const
{
    QUrl url = m_parent->currentTab()->url();
    return url.path();
}


QString OwnBookMarks::currentTitle() const
{
    QString title = m_parent->windowTitle();
    return title.remove( " - reKonq" );
}


// -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------


BookmarksMenu::BookmarksMenu(KMainWindow *parent)
    : KMenu(parent)
    , m_owner( new OwnBookMarks( parent ) )
{
    KUrl bookfile = KUrl( "~/.kde/share/apps/konqueror/bookmarks.xml" );    // share konqueror bookmarks
    m_manager = KBookmarkManager::managerForExternalFile( bookfile.path() );

    m_ac = new KActionCollection( this );

    m_menu = new KBookmarkMenu( m_manager , m_owner, this, m_ac );
}
