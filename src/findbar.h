/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2008-2009 by Andrea Diamantini <adjam7 at gmail dot com>
* Copyright (C) 2009 by Lionel Chauvin <megabigbug@yahoo.fr>
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


#ifndef FINDBAR_H
#define FINDBAR_H


// KDE Includes
#include <KLineEdit>

// Qt Includes
#include <QtGui/QWidget>
#include <QtGui/QCheckBox>
#include <QtGui/QKeyEvent>

// Forward Declarations
class KMainWindow;
class QKeyEvent;
class QString;


class FindBar : public QWidget
{
    Q_OBJECT

public:
    FindBar(KMainWindow *mainwindow);
    ~FindBar();
    KLineEdit *lineEdit() const;
    bool matchCase() const;

public slots:
    void clear();
    void show();
    void notifyMatch(bool match);

protected Q_SLOTS:
    void keyPressEvent(QKeyEvent* event);

signals:
    void searchString(const QString &);

private:
    KLineEdit *m_lineEdit;
    QCheckBox *m_matchCase;

};

#endif
