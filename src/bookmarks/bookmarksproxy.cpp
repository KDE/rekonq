#include "bookmarksproxy.h"

BookmarksProxy::BookmarksProxy( QObject *parent ):
	QSortFilterProxyModel( parent )
{
}

bool BookmarksProxy::filterAcceptsRow( int source_row, const QModelIndex &source_parent ) const
{
	QModelIndex idx = sourceModel()->index( source_row, 0, source_parent );

// 	return idx.data().toString().contains( filterRegExp() );
	return recursiveMatch( idx );
}

bool BookmarksProxy::recursiveMatch( const QModelIndex &index ) const
{
	if( index.data().toString().contains( filterRegExp() ) ) {
		return true;
	}

	for( int childRow = 0; childRow < sourceModel()->rowCount( index ); ++childRow ) {
		if( recursiveMatch( sourceModel()->index( childRow, 0, index ) ) ) {
			return true;
		}
	}

	return false;
}
