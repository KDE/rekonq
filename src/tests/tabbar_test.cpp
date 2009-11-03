/*
 * Copyright 2008 Benjamin C. Meyer <ben@meyerhome.net>
 * Copyright 2008 Andrea Diamantini <adjam7@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA  02110-1301  USA
 */

#include <qtest_kde.h>

#include <QtTest>
#include <QtCore>
#include <QtGui>

#include "../tabbar.h"


class TabBarTest : public QObject
{
    Q_OBJECT

public slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

private slots:
    void tabbar_data();
    void tabbar();

    void tabSizeHint_data();
    void tabSizeHint();
};


// Subclass that exposes the protected functions.
class SubTabBar : public TabBar
{
public:
    void call_cloneTab(int index)
        { return SubTabBar::cloneTab(index); }

    void call_closeOtherTabs(int index)
        { return SubTabBar::closeOtherTabs(index); }

    void call_closeTab(int index)
        { return SubTabBar::closeTab(index); }

    void call_mouseMoveEvent(QMouseEvent* event)
        { return SubTabBar::mouseMoveEvent(event); }

    void call_mousePressEvent(QMouseEvent* event)
        { return SubTabBar::mousePressEvent(event); }

    void call_reloadAllTabs()
        { return SubTabBar::reloadAllTabs(); }

    void call_reloadTab(int index)
        { return SubTabBar::reloadTab(index); }

    QSize call_tabSizeHint(int index) const
        { return SubTabBar::tabSizeHint(index); }

    void call_showTabPreview(int tab)
        { return SubTabBar::showTabPreview(tab); }
};


// This will be called before the first test function is executed.
// It is only called once.
void TabBarTest::initTestCase()
{
}


// This will be called after the last test function is executed.
// It is only called once.
void TabBarTest::cleanupTestCase()
{
}


// This will be called before each test function is executed.
void TabBarTest::init()
{
}


// This will be called after every test function.
void TabBarTest::cleanup()
{
}

// -------------------------------------------

void TabBarTest::tabbar_data()
{
}


void TabBarTest::tabbar()
{
    SubTabBar widget;
}

// -------------------------------------------

void TabBarTest::tabSizeHint_data()
{
//     QTest::addColumn<int>("index");
//     QTest::newRow("0") << 0;
}


// protected QSize tabSizeHint(int index) const
void TabBarTest::tabSizeHint()
{
    // Need fixes as our function uses MainView methods to determine size
//     QFETCH(int, index);
//     SubTabBar bar;
//     QVERIFY(bar.call_tabSizeHint(index).width() <= 250);
}
    
// -------------------------------------------

QTEST_KDEMAIN(TabBarTest, GUI)
#include "tabbar_test.moc"
