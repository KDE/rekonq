/* ============================================================
*
* This is a part of the GSoC project 2011 - Fancy Bookmarking
*
* Copyright (c) 2011-2012 by Phaneendra Hegde <pnh.pes@gmail.com>
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


#ifndef RESOURCELINKDIALOG_H
#define RESOURCELINKDIALOG_H

//Qt includes
#include <QListView>

//kde includes
#include <KDialog>
#include <KLineEdit>
#include <KConfigDialog>


namespace Nepomuk2
{
class Resource;

namespace Query
{
class Query;
}

class ResourceLinkDialog : public KDialog
{
    Q_OBJECT

public:
    explicit ResourceLinkDialog(Nepomuk2::Resource& nfoResource, QWidget* parent = 0);
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
