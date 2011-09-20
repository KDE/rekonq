/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2008 by Benjamin C. Meyer <ben@meyerhome.net>
* Copyright (C) 2008-2011 by Andrea Diamantini <adjam7 at gmail dot com>
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


#include <qtest_kde.h>

#include "mainwindow.h"
#include "mainview.h"
#include "tabbar.h"


/**
 * Subclass that exposes the protected functions.
 */
class SubTabBar : public TabBar
{
public:

    SubTabBar(QWidget *parent) : TabBar(parent) {};

    QSize call_tabSizeHint(int index) const
    {
        return SubTabBar::tabSizeHint(index);
    }

    void call_mouseMoveEvent(QMouseEvent* event)
    {
        return SubTabBar::mouseMoveEvent(event);
    }

    void call_leaveEvent(QEvent* event)
    {
        return SubTabBar::leaveEvent(event);
    }

    void call_mousePressEvent(QMouseEvent* event)
    {
        return SubTabBar::mousePressEvent(event);
    }

    void call_mouseReleaseEvent(QMouseEvent* event)
    {
        return SubTabBar::mouseReleaseEvent(event);
    }
};


// ------------------------------------------------------------------


class TabBarTest : public QObject
{
    Q_OBJECT

public slots:
    void initTestCase();
    void cleanupTestCase();

private slots:
    void tabSizeHint_data();
    void tabSizeHint();

    void mousePress_data();
    void mousePress();

private:
    SubTabBar *_bar;
};


// -------------------------------------------

void TabBarTest::initTestCase()
{
    MainWindow *w = new MainWindow;
    MainView *mv = new MainView(w);
    _bar = new SubTabBar(mv);
}

void TabBarTest::cleanupTestCase()
{
    delete _bar;
}

// -------------------------------------------

void TabBarTest::tabSizeHint_data()
{
    QTest::addColumn<int>("index");

    QTest::newRow("1th") << 0;
    QTest::newRow("2nd") << 1;
    QTest::newRow("3rd") << 2;
    QTest::newRow("4th") << 3;
    QTest::newRow("5th") << 4;
    QTest::newRow("6th") << 5;
    QTest::newRow("7th") << 6;
    QTest::newRow("8th") << 7;
    QTest::newRow("9th") << 8;
    QTest::newRow("10th") << 9;
}


void TabBarTest::tabSizeHint()
{
    QFETCH(int, index);

    QVERIFY(_bar->call_tabSizeHint(index).width() > 0);
}


void TabBarTest::mousePress_data()
{
}


void TabBarTest::mousePress()
{
//     QTest::mousePress(_bar, Qt::MidButton);
// //     QCOMPARE();  ?
//
//     QTest::mousePress(_bar, Qt::LeftButton);
// //     QCOMPARE();  ?
}

// -------------------------------------------

QTEST_KDEMAIN(TabBarTest, GUI)
#include "tabbar_test.moc"
