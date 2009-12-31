/* ============================================================
*
* This file is a part of the rekonq project
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


#ifndef PREVIEWCHOOSER_H
#define PREVIEWCHOOSER_H


// Local Includes
#include "application.h"

// KDE Includes
#include <KLineEdit>
#include <KDialog>

// Qt Includes
#include <QWidget>
#include <QTreeView>
#include <QStandardItemModel>

// Forward Declarations
class KUrl;


class PreviewChooser : public KDialog
{
Q_OBJECT

public:
    explicit PreviewChooser(int previewIndex, QString url);
    virtual ~PreviewChooser();

signals:
    void urlChoosed(int, const KUrl &);
    
public slots:
    void refreshModel();
    void setUrl(QModelIndex i);
    void buttonClicked(KDialog::ButtonCode code);
    void urlChanged();

private:
    QTreeView *m_treeView;
    QStandardItemModel *m_model;
    
    KLineEdit *m_line;
    
    int m_previewIndex;
};

#endif // PREVIEWCHOOSER_H
