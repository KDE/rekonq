/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2008-2011 by Andrea Diamantini <adjam7 at gmail dot com>
* Copyright (C) 2009-2011 by Lionel Chauvin <megabigbug@yahoo.fr>
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


#ifndef ZOOMBAR_H
#define ZOOMBAR_H


// Rekonq Includes
#include "rekonq_defines.h"

// Qt Includes
#include <QWidget>

// Forward Declarations
class MainWindow;

class QLabel;
class QSlider;
class QToolButton;


class REKONQ_TESTS_EXPORT ZoomBar : public QWidget
{
    Q_OBJECT

public:
    ZoomBar(QWidget *parent);

public Q_SLOTS:
    void show();
    void hide();

    void zoomIn();
    void zoomOut();
    void zoomNormal();
    void setupActions(MainWindow *window);
    void updateSlider(int webview);
    void setValue(int value);
    void toggleVisibility();

private:
    void saveZoomValue(const QString &, int);

Q_SIGNALS:
    void visibilityChanged(bool);

private:
    QToolButton *m_zoomIn;
    QToolButton *m_zoomOut;
    QToolButton *m_zoomNormal;
    QSlider *m_zoomSlider;
    QLabel *m_percentage;
};

#endif
