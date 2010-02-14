/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2009 by Nils Weigel <nehlsen at gmail dot com>
* Copyright (C) 2010 by Andrea Diamantini <adjam7 at gmail dot com>
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
#include "bookmarkstreemodel.h"
#include "bookmarkstreemodel.moc"

// Local Includes
#include "application.h"
#include "bookmarksmanager.h"

// KDE includes
#include <KBookmarkGroup>
#include <KLocalizedString>


class BookmarksTreeModel::BtmItem
{
public:
    BtmItem(const KBookmark &bm)
        : m_parent(0)
        , m_kbm(bm)
    {
    }
    
    
    ~BtmItem()
    {
        qDeleteAll(m_children);
    }


    QVariant data( int role = Qt::DisplayRole ) const
    {
        if( m_kbm.isNull() )
            return QVariant();  // should only happen for root item

        if( role == Qt::DisplayRole )
            return m_kbm.text();
        if( role == Qt::DecorationRole )
            return KIcon( m_kbm.icon() );
        if( role == Qt::UserRole )
            return m_kbm.url();

        return QVariant();
    }


    int row() const
    {
        if(m_parent)
            return m_parent->m_children.indexOf( const_cast< BtmItem* >( this ) );
        return 0;
    }


    int childCount() const
    {
        return m_children.count();
    }


    BtmItem* child( int n )
    {
        Q_ASSERT(n>=0);
        Q_ASSERT(n<childCount());

        return m_children.at(n);
    }


    BtmItem* parent() const
    {
        return m_parent;
    }


    void appendChild(BtmItem *child)
    {
        if( !child )
            return;

        child->m_parent = this;
        m_children << child;
    }


    void clear()
    {
        qDeleteAll(m_children);
        m_children.clear();
    }

private:
    BtmItem *m_parent;
    QList< BtmItem* > m_children;
    KBookmark m_kbm;
};


// -------------------------------------------------------------------------------------


BookmarksTreeModel::BookmarksTreeModel(QObject *parent)
    : QAbstractItemModel(parent)
    , m_root(0)
{
    resetModel();
    connect( Application::bookmarkProvider()->bookmarkManager(), SIGNAL( changed(QString,QString) ), this, SLOT( bookmarksChanged(QString) ) );
    connect( Application::bookmarkProvider()->bookmarkManager(), SIGNAL( bookmarksChanged(QString) ), this, SLOT( bookmarksChanged(QString) ) );
}


BookmarksTreeModel::~BookmarksTreeModel()
{
    delete m_root;
}


int BookmarksTreeModel::rowCount(const QModelIndex &parent) const
{
    BtmItem *parentItem = 0;
    if( !parent.isValid() ) 
    {
        parentItem = m_root;
    }
    else 
    {
        parentItem = static_cast< BtmItem* >( parent.internalPointer() );
    }

    return parentItem->childCount();
}


int BookmarksTreeModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    // name
    return 1;
}


QVariant BookmarksTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if( orientation == Qt::Horizontal 
        && role == Qt::DisplayRole 
        && section == 0 
      )
        return i18n( "Bookmark" );

    return QVariant();
}


Qt::ItemFlags BookmarksTreeModel::flags(const QModelIndex &index) const
{
    Q_UNUSED(index)
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}


QModelIndex BookmarksTreeModel::index(int row, int column, const QModelIndex &parent) const
{
    if( !hasIndex( row, column, parent ) ) 
    {
        return QModelIndex();
    }

    BtmItem *parentItem;

    if( !parent.isValid() ) 
    {
        parentItem = m_root;
    }
    else 
    {
        parentItem = static_cast< BtmItem* >( parent.internalPointer() );
    }

    BtmItem *childItem = parentItem->child( row );
    if( childItem ) 
    {
        return createIndex( row, column, childItem );
    }

    return QModelIndex();
}


QModelIndex BookmarksTreeModel::parent(const QModelIndex &index) const
{
    if( !index.isValid() ) 
    {
        return QModelIndex();
    }

    BtmItem *childItem = static_cast< BtmItem* >( index.internalPointer() );
    BtmItem *parentItem = childItem->parent();

    if( parentItem == m_root ) 
    {
        return QModelIndex();
    }

    return createIndex( parentItem->row(), 0, parentItem );
}


QVariant BookmarksTreeModel::data(const QModelIndex &index, int role) const
{
    if( !index.isValid() ) 
    {
        return QVariant();
    }

    BtmItem *node = static_cast< BtmItem* >( index.internalPointer() );
    if( node && node == m_root ) 
    {
        if( role == Qt::DisplayRole )
            return i18n( "Bookmarks" );
        if( role == Qt::DecorationRole )
            return KIcon( "bookmarks" );
    }
    else 
    {
        if( node )
            return node->data( role );
    }

    return QVariant();
}


void BookmarksTreeModel::bookmarksChanged( const QString &groupAddress )
{
    if( groupAddress.isEmpty() )
    {
        resetModel();
        return;
    }

    BtmItem *node = m_root;
    QModelIndex nodeIndex;

    QStringList indexChain( groupAddress.split( '/', QString::SkipEmptyParts) );
    foreach( const QString &sIndex, indexChain ) 
    {
        bool ok;
        int i = sIndex.toInt( &ok );
        if( !ok )
            break;

        if( i < 0 || i >= node->childCount() )
            break;

        node = node->child( i );
        nodeIndex = index( i, 0, nodeIndex );
    }
    emit dataChanged( index( 0, 0, nodeIndex ), index( node->childCount(), 0, nodeIndex ) );
}


void BookmarksTreeModel::resetModel()
{
    setRoot(Application::bookmarkProvider()->rootGroup());
}


void BookmarksTreeModel::setRoot(KBookmarkGroup bmg)
{
    delete m_root;
    m_root = new BtmItem(KBookmark());

    if( bmg.isNull() )
        return;

    populate( m_root, bmg );
    reset();
}


void BookmarksTreeModel::populate( BtmItem *node, KBookmarkGroup bmg)
{
    node->clear();

    if( bmg.isNull() )
        return;

    KBookmark bm = bmg.first();
    while( !bm.isNull() ) 
    {
        BtmItem *newChild = new BtmItem( bm );
        if( bm.isGroup() )
            populate( newChild, bm.toGroup() );

        node->appendChild( newChild );
        bm = bmg.next( bm );
    }
}
