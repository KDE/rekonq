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

// Qt Includes
#include <QMimeData>

// KDE includes
#include <KBookmarkGroup>
#include <KLocalizedString>


BtmItem::BtmItem(const KBookmark &bm)
    : m_parent(0)
    , m_kbm(bm)
{
}


BtmItem::~BtmItem()
{
    qDeleteAll(m_children);
}


QVariant BtmItem::data( int role ) const
{
    if( m_kbm.isNull() )
        return QVariant();  // should only happen for root item

    if( role == Qt::DisplayRole )
        return m_kbm.text();
    if( role == Qt::DecorationRole )
        return KIcon( m_kbm.icon() );
    if( role == Qt::UserRole )
        return m_kbm.url();
    if( role == Qt::ToolTipRole)
    {
        QString tooltip = "";
	
        if(!m_kbm.text().isEmpty())
	{
            tooltip += m_kbm.text();
	}
        if(m_kbm.isGroup())
	{
            tooltip += " [" + QString::number(childCount()) + " " + i18n("Items") + "]";
	}
        if(!m_kbm.url().url().isEmpty())
        {
            if(!tooltip.isEmpty())
                tooltip += "\n";
            tooltip += m_kbm.url().url();
        }
        return tooltip;
    }

    return QVariant();
}


int BtmItem::row() const
{
    if(m_parent)
        return m_parent->m_children.indexOf( const_cast< BtmItem* >( this ) );
    return 0;
}


int BtmItem::childCount() const
{
    return m_children.count();
}


BtmItem* BtmItem::child( int n )
{
    Q_ASSERT(n>=0);
    Q_ASSERT(n<childCount());

    return m_children.at(n);
}


BtmItem* BtmItem::parent() const
{
    return m_parent;
}


void BtmItem::appendChild(BtmItem *child)
{
    if( !child )
        return;

    child->m_parent = this;
    m_children << child;
}


void BtmItem::clear()
{
    qDeleteAll(m_children);
    m_children.clear();
}

KBookmark BtmItem::getBkm() const
{
    return m_kbm;
}

// -------------------------------------------------------------------------------------


BookmarksTreeModel::BookmarksTreeModel(QObject *parent)
    : QAbstractItemModel(parent)
    , m_root(0)
{
    resetModel();
    connect( this, SIGNAL(bookmarksUpdated()), parent, SLOT(loadFoldedState()));
    connect( Application::bookmarkProvider()->bookmarkManager(), SIGNAL( changed(QString,QString) ), this, SLOT( bookmarksChanged() ) );
    connect( parent, SIGNAL(saveOnlyRequested()), this, SLOT(saveOnly()) );
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
    Qt::ItemFlags flags = QAbstractItemModel::flags(index);
    
    if(!index.isValid())
        return flags | Qt::ItemIsDropEnabled;

    flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled;

    if(bookmarkForIndex(index).isGroup())
        flags |= Qt::ItemIsDropEnabled;
    
    return flags;
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


void BookmarksTreeModel::bookmarksChanged()
{
    resetModel();
    emit bookmarksUpdated();
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
        else
        {
            Application::bookmarkProvider()->completionObject()->addItem(bm.url().url());
            //QMessageBox::information(new QWidget(), "", bm.url().url());
        }

        node->appendChild( newChild );
        bm = bmg.next( bm );
    }
}


KBookmark BookmarksTreeModel::bookmarkForIndex(const QModelIndex index) const
{
    return static_cast<BtmItem*>(index.internalPointer())->getBkm();
}


void BookmarksTreeModel::saveOnly()
{
    disconnect(Application::bookmarkProvider()->bookmarkManager(), SIGNAL(changed(QString,QString)), this, SLOT(bookmarksChanged()));
    connect(Application::bookmarkProvider()->bookmarkManager(), SIGNAL(changed(QString,QString)), this, SLOT(reconnectManager()));
    Application::bookmarkProvider()->bookmarkManager()->emitChanged();
}


void BookmarksTreeModel::reconnectManager()
{
    connect(Application::bookmarkProvider()->bookmarkManager(), SIGNAL( changed(QString,QString) ), this, SLOT(bookmarksChanged()));
}


Qt::DropActions BookmarksTreeModel::supportedDropActions () const
{
    return Qt::MoveAction;
}


QStringList BookmarksTreeModel::mimeTypes () const
{
    return KBookmark::List::mimeDataTypes();
}


QMimeData* BookmarksTreeModel::mimeData( const QModelIndexList & indexes ) const
{
    QMimeData *mimeData = new QMimeData;

    QByteArray addresse = bookmarkForIndex(indexes.first()).address().toLatin1();
    mimeData->setData( "application/rekonq-bookmark", addresse);
    bookmarkForIndex(indexes.first()).populateMimeData(mimeData);

    return mimeData;
}


bool BookmarksTreeModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex & parent)
{
    if(action == Qt::MoveAction)
    {
        if(data->hasFormat("application/rekonq-bookmark"))
        {
            QByteArray addresses = data->data("application/rekonq-bookmark");
            KBookmark bookmark = Application::bookmarkProvider()->bookmarkManager()->findByAddress(QString::fromLatin1(addresses.data()));

            QModelIndex destIndex = index(row, column, parent);

            KBookmark dropDestBookmark;
            if(destIndex.isValid())
                dropDestBookmark = bookmarkForIndex(destIndex);

            KBookmarkGroup root = Application::bookmarkProvider()->rootGroup();
            if(parent.isValid())
                root = bookmarkForIndex(parent).toGroup();

            if(!destIndex.isValid())
            {
                if(!parent.isValid()) // Drop into a blank area
                {
                    Application::bookmarkProvider()->rootGroup().deleteBookmark(bookmark);
                    Application::bookmarkProvider()->rootGroup().addBookmark(bookmark);
                }
                else // Drop at the last item of the group or directly on the main item of the group
                {
                    root.deleteBookmark(bookmark);
                    root.addBookmark(bookmark);
                }
            }

            else
            {
                if(row == -1)
                {
                    root.deleteBookmark(bookmark);
                    root.addBookmark(bookmark);
                }
                else // A classic drop
                {
                    root.moveBookmark(bookmark, root.previous(dropDestBookmark));
                }
            }

            Application::bookmarkProvider()->bookmarkManager()->emitChanged(root);
        }
    }
    return true;
}
