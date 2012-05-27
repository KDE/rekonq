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


#ifndef RESOURCELINKDIALOG_H
#define RESOURCELINKDIALOG_H

//Qt includes
#include <QListView>

//kde includes
#include <KDialog>
#include <KLineEdit>
#include <KConfigDialog>

namespace Nepomuk
{
    class Resource;
    namespace Query {
        class Query;
    }

    class ResourceLinkDialog : public KDialog
    {
        Q_OBJECT

    public:
        explicit ResourceLinkDialog( Nepomuk::Resource& nfoResource, QWidget* parent = 0 );
        virtual ~ResourceLinkDialog();
        void setRelatedResources();

    private Q_SLOTS:
        void dynamicSearchingSlot();
        void resourceSelectedSlot(int);
        void linkResourceSlot();
        void unlinkResourceSlot();
        void createNewResourceSlot();
        void showContextMenu(const QPoint&);


    private:
        class Private;
        Private* const d;

    };

}

#endif // RESOURCELINKDIALOG_H
