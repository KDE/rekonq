/*
 * Copyright 2010 Andrea Diamantini <adjam7@gmail.com>
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

#include <QtGui>
#include <QtTest/QtTest>

#include "findbar.h"
#include "mainwindow.h"


class FindBarTest : public QObject
{
    Q_OBJECT

public slots:
    void initTestCase();
    void cleanupTestCase();

private slots:
    void matchCase();
    void notifyMatch();

private:
    FindBar *bar;
    MainWindow *w;
};


// -------------------------------------------


void FindBarTest::initTestCase()
{
    w = new MainWindow;
    bar = new FindBar(w);
}


void FindBarTest::cleanupTestCase()
{
    delete bar;
}

void FindBarTest::matchCase()
{

}

void FindBarTest::notifyMatch()
{
}

// -------------------------------------------

QTEST_KDEMAIN(FindBarTest, GUI)
#include "findbar_test.moc"
