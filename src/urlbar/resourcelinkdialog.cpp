/*
    This is a part of the GSoC project - Fancy Bookmarking
    Copyright 2011 Phaneendra Hegde <pnh.pes@gmail.com>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

//Local Includes
#include "resourcelinkdialog.h"
#include "newresourcedialog.h"

//Qt Includes
#include <QtGui/QGridLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QComboBox>
#include <QtGui/QAbstractItemView>
#include <QLabel>
#include <QListView>
#include <QPushButton>
#include <QColumnView>
#include <QStringListModel>
#include <QStandardItem>
#include <QModelIndexList>
#include <QItemSelectionModel>
#include <QMenu>
#include <QListWidget>

//KDE Includes
#include <KLocale>
#include <KDebug>
#include <KAction>
#include <KIcon>

//Nepomuk Includes
#include <Nepomuk/Utils/SimpleResourceModel>
#include <Nepomuk/Query/Term>
#include <Nepomuk/Query/Result>
#include <Nepomuk/Query/ResourceTypeTerm>
#include <Nepomuk/Query/QueryServiceClient>
#include <Nepomuk/Vocabulary/PIMO>
#include <Nepomuk/Vocabulary/NCO>
#include <Nepomuk/Query/QueryParser>
#include <Nepomuk/Variant>


class Nepomuk::ResourceLinkDialog::Private
{
public:
    void _k_selectionChanged();

    KLineEdit *m_searchBox;
    QListView *m_resourceView;
    QListView *m_linkedResources;
    KAction *m_removeResourceAction;
    QComboBox *m_resourceSelect;
    QLabel *m_resourceLabel;
    QLabel *m_linkedResourceLabel;
    QColumnView *m_leftPanel;
    QStringListModel *m_model;
    QPushButton *m_newResourceButton;
    Utils::SimpleResourceModel *m_resourceModel;
    Utils::SimpleResourceModel *m_linkedResourceModel;
    Nepomuk::ResourceLinkDialog *q;

    Nepomuk::Resource m_nfoResource;

};

void Nepomuk::ResourceLinkDialog::Private::_k_selectionChanged()
{
    q->enableButton( KDialog::User1, !m_resourceView->selectionModel()->selectedRows().isEmpty() );
}


Nepomuk::ResourceLinkDialog::ResourceLinkDialog( Nepomuk::Resource &nfoResource, QWidget* parent ):
    KDialog( parent ),
    d( new Private() )
{
    d->m_nfoResource = nfoResource;
    setWindowTitle( i18n( "Resource Linker" ) );
    setButtons( Ok | User1 | User2 | Cancel );
    enableButtonCancel( true );
    enableButtonOk( true );
    enableButton( User1, false );
    setButtonText( Ok, i18n( "Done" ) );
    setButtonText( User1, i18n( "Link" ) );
    setButtonText( User2, "Unlink" );
    setMinimumSize(400,350);
//    d->m_resourceView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    QGridLayout *mainLayout = new QGridLayout ( mainWidget() );

    d->q = this;

    d->m_linkedResources = new QListView ( mainWidget() );
    d->m_linkedResourceModel = new Utils::SimpleResourceModel( this );
    d->m_linkedResources->setModel( d->m_linkedResourceModel );
    setRelatedResources();

    d->m_searchBox = new KLineEdit ( mainWidget() );
    d->m_searchBox->setPlaceholderText( i18n( "Search resources" ) );
    connect( d->m_searchBox, SIGNAL( textChanged( QString ) ), this, SLOT( dynamicSearchingSlot() ) );

    d->m_resourceView = new QListView ( mainWidget() );
    d->m_resourceView->setToolTip( i18n( " Double click to link resource ") );
    d->m_resourceModel = new Utils::SimpleResourceModel( this );
    d->m_resourceView->setModel( d->m_resourceModel );

    d->m_resourceSelect = new QComboBox( mainWidget() );
    QStringList rlist;
    rlist << i18n( "Any resource" ) << i18n( "Persons" ) << i18n( "Projects" ) << i18n( "Tasks" ) << i18n( "Places" ) << i18n( "Notes" );
    d->m_resourceSelect->addItems( rlist );
    d->m_resourceSelect->setItemIcon(1,KIcon("user-identity"));
    d->m_resourceSelect->setItemIcon(2,KIcon("project-development"));
    d->m_resourceSelect->setItemIcon(3,KIcon("view-pim-tasks"));
    d->m_resourceSelect->setItemIcon(4,KIcon("user-location"));
    d->m_resourceSelect->setItemIcon(5,KIcon("knotes"));
    connect( d->m_resourceSelect, SIGNAL( currentIndexChanged( int ) ), this, SLOT( resourceSelectedSlot( int ) ) );

    d->m_resourceLabel = new QLabel( i18n( "Matching resources:" ), mainWidget() );
    d->m_linkedResourceLabel = new QLabel( i18n( "Linked Resources:" ), mainWidget() );


    d->m_newResourceButton = new QPushButton( mainWidget() );
    d->m_newResourceButton->setText( i18n( "Create New Resource" ) );
    if( d->m_resourceSelect->currentIndex() == 0 ) {
        d->m_newResourceButton->setEnabled( false );
    }
    connect(d->m_newResourceButton, SIGNAL( clicked() ), this, SLOT( createNewResourceSlot() ) );

    QVBoxLayout *vlayoutR = new QVBoxLayout;
    QVBoxLayout *vlayoutL = new QVBoxLayout;
    vlayoutL->addWidget( d->m_searchBox );
    vlayoutL->addWidget( d->m_resourceLabel );
    vlayoutL->addWidget( d->m_resourceView );
    vlayoutR->addWidget( d->m_resourceSelect );
    vlayoutR->addWidget( d->m_linkedResourceLabel );
    vlayoutR->addWidget( d->m_linkedResources );
    vlayoutR->addWidget( d->m_newResourceButton );
    mainLayout->addLayout( vlayoutL, 1 ,1 );
    mainLayout->addLayout(vlayoutR, 1, 2);
    mainLayout->setColumnMinimumWidth(1,100);

//    d->m_linkedResources->setContextMenuPolicy(Qt::CustomContextMenu);
    d->m_linkedResources->setContextMenuPolicy(Qt::CustomContextMenu);

    connect( d->m_resourceView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
                 this, SLOT(_k_selectionChanged()) );
    connect ( d->m_linkedResources->selectionModel(), SIGNAL( selectionChanged( QItemSelection, QItemSelection ) ),
                this, SLOT( _k_selectionChanged() ) );
    connect ( this, SIGNAL( user1Clicked() ), this, SLOT( linkResourceSlot() ) );
    connect ( this, SIGNAL( user2Clicked() ), this, SLOT( unlinkResourceSlot() ) );
    connect ( d->m_resourceView, SIGNAL( doubleClicked(QModelIndex) ), this, SLOT( linkResourceSlot() ) );
    connect (d->m_linkedResources, SIGNAL( customContextMenuRequested(QPoint) ), this, SLOT( showContextMenu(QPoint) ) );
    if( !d->m_linkedResources->selectionModel()->selectedRows().isEmpty() ) {
            enableButton( User2, true );
    }
}

Nepomuk::ResourceLinkDialog::~ResourceLinkDialog()
{
    delete d;
}

void Nepomuk::ResourceLinkDialog::setRelatedResources()
{
    QList<Nepomuk::Resource> relatedResourceList = d->m_nfoResource.isRelateds();
    d->m_linkedResourceModel->setResources(relatedResourceList);

}

void Nepomuk::ResourceLinkDialog::linkResourceSlot()
{
    QModelIndexList selectedResourceList;
    selectedResourceList << d->m_resourceView->selectionModel()->selectedIndexes();
    Q_FOREACH(const QModelIndex& i, selectedResourceList) {
        d->m_resourceView->selectionModel()->setCurrentIndex( i, QItemSelectionModel::NoUpdate );
        d->m_nfoResource.addIsRelated( d->m_resourceModel->resourceForIndex(d->m_resourceView->selectionModel()->currentIndex() ) );
    }
    setRelatedResources();
}

void Nepomuk::ResourceLinkDialog::unlinkResourceSlot()
{
    d->m_nfoResource.removeProperty( Nepomuk::Resource::isRelatedUri(),
                                    d->m_linkedResourceModel->resourceForIndex(
                                                    d->m_linkedResources->selectionModel()->currentIndex() ) );
    setRelatedResources();
}

void Nepomuk::ResourceLinkDialog::showContextMenu(const QPoint &pos)
{
    d->m_removeResourceAction = new KAction(this);
    d->m_removeResourceAction->setText(i18n("&Unlink "));
    d->m_removeResourceAction->setIcon(KIcon("edit-delete"));
    connect (d->m_removeResourceAction, SIGNAL( triggered(bool) ), this, SLOT( unlinkResourceSlot() ) );

    QMenu myMenu;
    QPoint globalPos = d->m_linkedResources->mapToGlobal(pos);
    myMenu.addAction(d->m_removeResourceAction);
    myMenu.exec(globalPos);
}
void Nepomuk::ResourceLinkDialog::createNewResourceSlot()
{
    Nepomuk::NewResourceDialog newResource( d->m_resourceSelect->currentIndex(), d->m_nfoResource );
    //close();
    newResource.exec();
    setRelatedResources();
}
void Nepomuk::ResourceLinkDialog::dynamicSearchingSlot()
{
    Nepomuk::Query::Query query;
    Nepomuk::Query::QueryServiceClient *test;
    switch( d->m_resourceSelect->currentIndex() ) {
    case 1:
        query =  Nepomuk::Query::QueryParser::parseQuery( d->m_searchBox->text() );
        query = query && Nepomuk::Query::ResourceTypeTerm( Nepomuk::Vocabulary::PIMO::Person() );
        test = new Nepomuk::Query::QueryServiceClient( this );
        test->query( query );
        d->m_resourceModel->clear();
        connect(test,SIGNAL( newEntries( QList<Nepomuk::Query::Result> ) ),
                d->m_resourceModel,SLOT( addResults(QList<Nepomuk::Query::Result>)) );
        break;
    case 2:
        query =  Nepomuk::Query::QueryParser::parseQuery( d->m_searchBox->text() );
        query = query && Nepomuk::Query::ResourceTypeTerm( Nepomuk::Vocabulary::PIMO::Project() );
        test = new Nepomuk::Query::QueryServiceClient( this );
        test->query( query );
        d->m_resourceModel->clear();
        connect(test,SIGNAL( newEntries( QList<Nepomuk::Query::Result> ) ),
                d->m_resourceModel,SLOT( addResults(QList<Nepomuk::Query::Result>)) );
        break;
    case 3:
         query = Nepomuk::Query::QueryParser::parseQuery( d->m_searchBox->text() );
         query = query && Nepomuk::Query::ResourceTypeTerm( Nepomuk::Vocabulary::PIMO::Task() );
         test = new Nepomuk::Query::QueryServiceClient( this );
         test->query( query );
         d->m_resourceModel->clear();
         connect(test,SIGNAL( newEntries( QList<Nepomuk::Query::Result> ) ),
                 d->m_resourceModel,SLOT( addResults(QList<Nepomuk::Query::Result>)) );
         break;
    case 4:
         query = Nepomuk::Query::QueryParser::parseQuery( d->m_searchBox->text() );
         query = query && Nepomuk::Query::ResourceTypeTerm( Nepomuk::Vocabulary::PIMO::Location() );
         test = new Nepomuk::Query::QueryServiceClient( this );
         test->query( query );
         d->m_resourceModel->clear();
         connect(test,SIGNAL( newEntries( QList<Nepomuk::Query::Result> ) ),
                 d->m_resourceModel,SLOT( addResults(QList<Nepomuk::Query::Result>)) );
         break;
    case 5:
        query =  Nepomuk::Query::QueryParser::parseQuery( d->m_searchBox->text() );
        query = query && Nepomuk::Query::ResourceTypeTerm( Nepomuk::Vocabulary::PIMO::Note() );
        test = new Nepomuk::Query::QueryServiceClient( this );
        test->query( query );
        d->m_resourceModel->clear();
        connect(test,SIGNAL( newEntries( QList<Nepomuk::Query::Result> ) ),
                d->m_resourceModel,SLOT( addResults(QList<Nepomuk::Query::Result>)) );
        break;
    default:
        break;
    }
}


void Nepomuk::ResourceLinkDialog::resourceSelectedSlot( int index )
{
    enableButton( User1, true );
    d->m_newResourceButton->setEnabled( true );
    if( index == 0 ) {
        d->m_resourceModel->clear();
        d->m_newResourceButton->setEnabled( false );
    }
    //List Personal Contacts
    if( index == 1 ) {
        Nepomuk::Query::Term term = Nepomuk::Query::ResourceTypeTerm( Nepomuk::Vocabulary::PIMO::Person() );
        Nepomuk::Query::Query query( term );
        query.setLimit( 20 );
        QList<Nepomuk::Query::Result>results = Nepomuk::Query::QueryServiceClient::syncQuery( query );
        QList <Nepomuk::Resource> resource;
        Q_FOREACH( const Nepomuk::Query::Result& result, results ) {
            resource.append( result.resource() );
        }
        d->m_resourceModel->setResources( resource );
    }
    //List Projects
    else if( index == 2 ) {
        Nepomuk::Query::Term term = Nepomuk::Query::ResourceTypeTerm( Nepomuk::Vocabulary::PIMO::Project() );
        Nepomuk::Query::Query query( term );
        query.setLimit(20);
        QList<Nepomuk::Query::Result>results = Nepomuk::Query::QueryServiceClient::syncQuery( query );
        QList <Nepomuk::Resource> resource;
        Q_FOREACH( const Nepomuk::Query::Result& result, results ) {
            resource.append( result.resource() );
        }
        d->m_resourceModel->setResources( resource );
    }
    //List Tasks
    else if( index == 3 ) {
        Nepomuk::Query::Term term = Nepomuk::Query::ResourceTypeTerm( Nepomuk::Vocabulary::PIMO::Task() );
        Nepomuk::Query::Query query( term );
        query.setLimit(20);
        QList<Nepomuk::Query::Result>results = Nepomuk::Query::QueryServiceClient::syncQuery( query );
        QList <Nepomuk::Resource> resource;
        Q_FOREACH( const Nepomuk::Query::Result& result, results ) {
            resource.append( result.resource() );
        }
        d->m_resourceModel->setResources( resource );
    }
    //List Places
    else if( index == 4 ) {
        Nepomuk::Query::Term term = Nepomuk::Query::ResourceTypeTerm( Nepomuk::Vocabulary::PIMO::Location() );
        Nepomuk::Query::Query query( term );
        query.setLimit(20);
        QList<Nepomuk::Query::Result>results = Nepomuk::Query::QueryServiceClient::syncQuery( query );
        QList <Nepomuk::Resource> resource;
        Q_FOREACH( const Nepomuk::Query::Result& result, results ) {
            resource.append( result.resource() );
        }
        d->m_resourceModel->setResources( resource );
    }
    //List Notes
    else if( index == 5 ) {
        Nepomuk::Query::Term term = Nepomuk::Query::ResourceTypeTerm( Nepomuk::Vocabulary::PIMO::Note() );
        Nepomuk::Query::Query query( term );
        query.setLimit(20);
        QList<Nepomuk::Query::Result>results = Nepomuk::Query::QueryServiceClient::syncQuery( query );
        QList <Nepomuk::Resource> resource;
        Q_FOREACH( const Nepomuk::Query::Result& result, results ) {
            resource.append( result.resource() );
        }
        d->m_resourceModel->setResources( resource );
    }
}

