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
#include "newresourcedialog.h"

//Nepomuk Includes
#include <Nepomuk/Vocabulary/NCO>
#include <Nepomuk/Vocabulary/PIMO>
#include <Nepomuk/Resource>
#include <Nepomuk/Tag>

//Qt Includes
#include <QPlainTextEdit>
#include <QVBoxLayout>
#include <QLabel>

class Nepomuk::NewResourceDialog::Private
{
public:
    KLineEdit *m_resourceName;
    QPlainTextEdit *m_description;
    QLabel *m_titleResource;
    QLabel *m_desResource;
    Nepomuk::NewResourceDialog *q;
    Nepomuk::Resource m_nofResource;
    int m_index;
};


Nepomuk::NewResourceDialog::NewResourceDialog( int index, Nepomuk::Resource& nfoResource, QWidget* parent ):
    KDialog( parent ),
    d( new Private() )
{
    d->q = this;
    d->m_index = index;
    d->m_nofResource =nfoResource;
    setWindowTitle( i18n( "Link to new Resource" ) );
    setButtonText( Ok, i18n( "Link" ) );
    setMinimumSize( 200, 150 );

    QVBoxLayout *layout = new QVBoxLayout( mainWidget() );
    d->m_resourceName = new KLineEdit( mainWidget() );
    d->m_titleResource = new QLabel( mainWidget() );
    d->m_titleResource->setText( i18n( "* Resource Name:" ) );
    layout->addWidget( d->m_titleResource );
    layout->addWidget( d->m_resourceName );
    d->m_description = new QPlainTextEdit( mainWidget() );
    d->m_desResource = new QLabel( mainWidget() );
    d->m_desResource->setText( i18n( "Description (Optional)" ));
    layout->addWidget( d->m_desResource);
    layout->addWidget( d->m_description );

    connect( this, SIGNAL( okClicked() ), this, SLOT( newResourceSlot() ) );
}


Nepomuk::NewResourceDialog::~NewResourceDialog()
{
    delete d;
}


void Nepomuk::NewResourceDialog::newResourceSlot()
{
    if( d->m_index == 1 ) {
      Nepomuk::Resource newResource( d->m_resourceName->text(), Nepomuk::Vocabulary::PIMO::Person() );
      newResource.addSymbol( "user-identity" );
      d->m_nofResource.addIsRelated( newResource );
    }
    else if( d->m_index == 2 ) {
        Nepomuk::Resource newResource( d->m_resourceName->text(), Nepomuk::Vocabulary::PIMO::Project() );
        newResource.addSymbol( "project-development" );
        d->m_nofResource.addIsRelated( newResource );
    }
    else if( d->m_index == 3 ) {
        Nepomuk::Resource newResource( d->m_resourceName->text(), Nepomuk::Vocabulary::PIMO::Task() );
        newResource.addSymbol( "view-pim-tasks" );
        d->m_nofResource.addIsRelated( newResource );
    }
    else if( d->m_index == 4 ) {
        Nepomuk::Resource newResource( d->m_resourceName->text(), Nepomuk::Vocabulary::PIMO::Location() );
        newResource.addSymbol( "user-location" );
        d->m_nofResource.addIsRelated( newResource );
    }
    else if( d->m_index == 5 ) {
        Nepomuk::Resource newResource( d->m_resourceName->text(), Nepomuk::Vocabulary::PIMO::Note() );
        newResource.addSymbol( "knotes" );
        d->m_nofResource.addIsRelated( newResource );
    }
}
