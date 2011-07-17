/*
 * Copyright 2010 Pierre Rossi <pierre.rossi@gmail.com>
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

#include "listitem.h"


class ListItemTest : public QObject
{
    Q_OBJECT

public slots:
    void initTestCase();
    void cleanupTestCase();

private slots:

    void wordHighLighting_data();
    void wordHighLighting();

};


// -------------------------------------------

void ListItemTest::initTestCase()
{
}


void ListItemTest::cleanupTestCase()
{
}


// -------------------------------------------

void ListItemTest::wordHighLighting_data()
{
    QTest::addColumn<QString>("string");
    QTest::addColumn<QString>("wordsToHighlight");
    QTest::addColumn<QString>("expected");

    QTest::newRow("plan b") << "<i>http://www.google.com/search?q=plan b&ie=UTF-8&oe=UTF-8</i>"
                            << "plan b" << "<i>http://www.google.com/search?q=<b>plan</b> <b>b</b>&amp;ie=UTF-8&amp;oe=UTF-8</i>";
    QTest::newRow("plan b #2") << "<i>http://en.wikipedia.org/wiki/Plan_B_(British_musician)</i>"
                               << "plan b" << "<i>http://en.wikipedia.org/wiki/<b>Plan</b>_<b>B</b>_(<b>B</b>ritish_musician)</i>";
    QTest::newRow("i") << "<i>http://i.imgur.com/jacoj.jpg</i>" << "i"
                       << "<i>http://<b>i</b>.<b>i</b>mgur.com/jacoj.jpg</i>";
    QTest::newRow("i#2") << "KDE - Experience Freedom!" << "i" << "KDE - Exper<b>i</b>ence Freedom!";
    QTest::newRow("i#3") << "The WebKit Open Source Project" << "i" << "The WebK<b>i</b>t Open Source Project";
    QTest::newRow("i#4") << "<i>http://webkit.org/</i>" << "i" << "<i>http://webk<b>i</b>t.org/</i>";
    QTest::newRow("b") << "<i>http://mail.google.com/mail/#inbox</i>" << "b" << "<i>http://mail.google.com/mail/#in<b>b</b>ox</i>";
    QTest::newRow("b#2") << "rekonq, WebKit KDE browser" << "b" << "rekonq, We<b>b</b>Kit KDE <b>b</b>rowser";
    QTest::newRow("<") << "Subject < Section < Wiki" << "<" << "Subject <b>&lt;</b> Section <b>&lt;</b> Wiki";
    QTest::newRow("&") << "<i>http://www.google.com/search?q=qt test&ie=UTF-8&oe=UTF-8</i>" << "&"
                       << "<i>http://www.google.com/search?q=qt test<b>&amp;</b>ie=UTF-8<b>&amp;</b>oe=UTF-8</i>";
    QTest::newRow("ciao") << "ciao" << "ciao" << "<b>ciao</b>";
    QTest::newRow("http://ciao") << "http://ciao" << "ciao" << "http://<b>ciao</b>";
}

void ListItemTest::wordHighLighting()
{
    QFETCH(QString, string);
    QFETCH(QString, wordsToHighlight);
    QFETCH(QString, expected);

    TextLabel test(string, wordsToHighlight);
    QCOMPARE(test.text(), expected);
}


// -------------------------------------------

QTEST_KDEMAIN(ListItemTest, GUI)
#include "listitem_test.moc"
