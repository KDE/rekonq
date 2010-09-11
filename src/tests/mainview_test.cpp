/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright 2008 Benjamin C. Meyer <ben@meyerhome.net>
* Copyright 2009 by Andrea Diamantini <adjam7 at gmail dot com>
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
#include "webview.h"


class MainViewTest : public QObject
{
    Q_OBJECT

public slots:
    void initTestCase();
    void cleanupTestCase();

private slots:
    void tabwidget_data();
    void tabwidget();

    void closeTab_data();
    void closeTab();

    void currentWebView_data();
    void currentWebView();

    void newTab_data();
    void newTab();

    void nextTab_data();
    void nextTab();

    void previousTab_data();
    void previousTab();

    void recentlyClosedTabs_data();
    void recentlyClosedTabs();

    void setCurrentTitle_data();
    void setCurrentTitle(const QString &);

    void showStatusBarMessage_data();
    void showStatusBarMessage(const QString &);

    void currentChanged_data();
    void currentChanged();

private:
    MainWindow *window;
    MainView *view;
};


// -------------------------------------------------------------------------------


// This will be called before the first test function is executed.
// It is only called once.
void MainViewTest::initTestCase()
{
    window = new MainWindow;
    view = window->mainView();
}

// This will be called after the last test function is executed.
// It is only called once.
void MainViewTest::cleanupTestCase()
{
//     delete window;   // FIXME: this let the test fail. Why??
}

// -------------------------------------------

void MainViewTest::tabwidget_data()
{
}

void MainViewTest::tabwidget()
{
    QCOMPARE(view->currentIndex(), -1);

//     view->newTab();
//     QCOMPARE(view->currentIndex(), 1);
//     view->newTab();
//     view->nextTab();
//     QCOMPARE(view->currentIndex(), 0);
//
//     view->previousTab();
//     QCOMPARE(view->currentIndex(), 0);
}

// -------------------------------------------

void MainViewTest::closeTab_data()
{
    QTest::addColumn<int>("index");
    QTest::newRow("null") << 0;
}


void MainViewTest::closeTab()
{
//     QFETCH(int, index);
//
//     QSignalSpy spy1(view, SIGNAL(linkHovered(const QString &)));
//     QSignalSpy spy2(view, SIGNAL(setCurrentTitle(const QString &)));
//     QSignalSpy spy3(view, SIGNAL(tabsChanged()));
//     QSignalSpy spy4(view, SIGNAL(lastTabClosed()));
//
//     view->newTab();
//     view->closeTab(index);
//     view->newTab();
//     view->closeTab(index);
//     view->newTab();
//
//     QCOMPARE(spy1.count(), 0);
//     QCOMPARE(spy2.count(), 2);
//     QCOMPARE(spy3.count(), 0);
//     QCOMPARE(spy4.count(), 0);
}

// -------------------------------------------

void MainViewTest::currentWebView_data()
{
    /*
    QTest::addColumn<WebView*>("currentWebView");
    QTest::newRow("null") << WebView*();
    */
}

void MainViewTest::currentWebView()
{
    /*
    QFETCH(WebView*, currentWebView);

    SubMainView widget;

    QSignalSpy spy0(view, SIGNAL(linkHovered(const QString &)));
    QSignalSpy spy2(view, SIGNAL(loadProgress(int)));
    QSignalSpy spy3(view, SIGNAL(setCurrentTitle(const QString &)));
    QSignalSpy spy4(view, SIGNAL(showStatusBarMessage(const QString &)));
    QSignalSpy spy5(view, SIGNAL(tabsChanged()));
    QSignalSpy spy6(view, SIGNAL(lastTabClosed()));

    QCOMPARE(widget.currentWebView(), currentWebView);

    QCOMPARE(spy0.count(), 0);
    QCOMPARE(spy2.count(), 0);
    QCOMPARE(spy3.count(), 0);
    QCOMPARE(spy4.count(), 0);
    QCOMPARE(spy5.count(), 0);
    QCOMPARE(spy6.count(), 0);
    */
    QSKIP("Test is not implemented.", SkipAll);
}

// -------------------------------------------

void MainViewTest::newTab_data()
{
    QTest::addColumn<int>("foo");
    QTest::newRow("null") << 0;
}


void MainViewTest::newTab()
{
//     QFETCH(int, foo);
//
//     QSignalSpy spy0(view, SIGNAL(linkHovered(const QString &)));
//     QSignalSpy spy1(view, SIGNAL(setCurrentTitle(const QString &)));
//     QSignalSpy spy2(view, SIGNAL(showStatusBarMessage(const QString &)));
//     QSignalSpy spy3(view, SIGNAL(tabsChanged()));
//     QSignalSpy spy4(view, SIGNAL(lastTabClosed()));
//
//     view->newTab();
//
//     QCOMPARE(spy0.count(), 0);
//     QCOMPARE(spy1.count(), 0);
//     QCOMPARE(spy2.count(), 0);
//     QCOMPARE(spy3.count(), 0);
//     QCOMPARE(spy4.count(), 0);
}

// -------------------------------------------

void MainViewTest::nextTab_data()
{
    QTest::addColumn<int>("foo");
    QTest::newRow("null") << 0;
}

// public void nextTab()
void MainViewTest::nextTab()
{
    /*
    QFETCH(int, foo);

    SubMainView widget;

    QSignalSpy spy0(view, SIGNAL(linkHovered(const QString &)));
    QSignalSpy spy2(view, SIGNAL(loadProgress(int)));
    QSignalSpy spy3(view, SIGNAL(setCurrentTitle(const QString &)));
    QSignalSpy spy4(view, SIGNAL(showStatusBarMessage(const QString &)));
    QSignalSpy spy5(view, SIGNAL(tabsChanged()));
    QSignalSpy spy6(view, SIGNAL(lastTabClosed()));

    widget.nextTab();

    QCOMPARE(spy0.count(), 0);
    QCOMPARE(spy2.count(), 0);
    QCOMPARE(spy3.count(), 0);
    QCOMPARE(spy4.count(), 0);
    QCOMPARE(spy5.count(), 0);
    QCOMPARE(spy6.count(), 0);
    */
    QSKIP("Test is not implemented.", SkipAll);
}

// -------------------------------------------

void MainViewTest::previousTab_data()
{
    QTest::addColumn<int>("foo");
    QTest::newRow("null") << 0;
}


// public void previousTab()
void MainViewTest::previousTab()
{
    /*
    QFETCH(int, foo);

    SubMainView widget;

    QSignalSpy spy0(view, SIGNAL(linkHovered(const QString &)));
    QSignalSpy spy2(view, SIGNAL(loadProgress(int)));
    QSignalSpy spy3(view, SIGNAL(setCurrentTitle(const QString &)));
    QSignalSpy spy4(view, SIGNAL(showStatusBarMessage(const QString &)));
    QSignalSpy spy5(view, SIGNAL(tabsChanged()));
    QSignalSpy spy6(view, SIGNAL(lastTabClosed()));

    widget.previousTab();

    QCOMPARE(spy0.count(), 0);
    QCOMPARE(spy2.count(), 0);
    QCOMPARE(spy3.count(), 0);
    QCOMPARE(spy4.count(), 0);
    QCOMPARE(spy5.count(), 0);
    QCOMPARE(spy6.count(), 0);
    */
    QSKIP("Test is not implemented.", SkipAll);
}

// -------------------------------------------

void MainViewTest::recentlyClosedTabs_data()
{
}

void MainViewTest::recentlyClosedTabs()
{
    /*
    SubMainView widget;

    QSignalSpy spy0(view, SIGNAL(linkHovered(const QString &)));
    QSignalSpy spy2(view, SIGNAL(loadProgress(int)));
    QSignalSpy spy3(view, SIGNAL(setCurrentTitle(const QString &)));
    QSignalSpy spy4(view, SIGNAL(showStatusBarMessage(const QString &)));
    QSignalSpy spy5(view, SIGNAL(tabsChanged()));
    QSignalSpy spy6(view, SIGNAL(lastTabClosed()));

    QCOMPARE(spy0.count(), 0);
    QCOMPARE(spy2.count(), 0);
    QCOMPARE(spy3.count(), 0);
    QCOMPARE(spy4.count(), 0);
    QCOMPARE(spy5.count(), 0);
    QCOMPARE(spy6.count(), 0);
    */
    QSKIP("Test is not implemented.", SkipAll);
}

// -------------------------------------------

void MainViewTest::setCurrentTitle_data()
{
    QTest::addColumn<QString>("url");
    QTest::newRow("null") << QString("foo");
}

// protected void setCurrentTitle(QString const &url)
void MainViewTest::setCurrentTitle(const QString &)
{
    /*
    QFETCH(QString, url);

    SubMainView widget;

    QSignalSpy spy0(view, SIGNAL(linkHovered(const QString &)));
    QSignalSpy spy2(view, SIGNAL(loadProgress(int)));
    QSignalSpy spy3(view, SIGNAL(setCurrentTitle(const QString &)));
    QSignalSpy spy4(view, SIGNAL(showStatusBarMessage(const QString &)));
    QSignalSpy spy5(view, SIGNAL(tabsChanged()));
    QSignalSpy spy6(view, SIGNAL(lastTabClosed()));

    widget.call_setCurrentTitle(url);

    QCOMPARE(spy0.count(), 0);
    QCOMPARE(spy2.count(), 0);
    QCOMPARE(spy3.count(), 0);
    QCOMPARE(spy4.count(), 0);
    QCOMPARE(spy5.count(), 0);
    QCOMPARE(spy6.count(), 0);
    */
    QSKIP("Test is not implemented.", SkipAll);
}

// -------------------------------------------

void MainViewTest::showStatusBarMessage_data()
{
    QTest::addColumn<QString>("message");
    QTest::newRow("null") << QString("foo");
}

// protected void showStatusBarMessage(QString const &message)
void MainViewTest::showStatusBarMessage(const QString &)
{
    /*
    QFETCH(QString, message);

    SubMainView widget;

    QSignalSpy spy0(view, SIGNAL(linkHovered(const QString &)));
    QSignalSpy spy2(view, SIGNAL(loadProgress(int)));
    QSignalSpy spy3(view, SIGNAL(setCurrentTitle(const QString &)));
    QSignalSpy spy4(view, SIGNAL(showStatusBarMessage(const QString &)));
    QSignalSpy spy5(view, SIGNAL(tabsChanged()));
    QSignalSpy spy6(view, SIGNAL(lastTabClosed()));

    widget.call_showStatusBarMessage(message);

    QCOMPARE(spy0.count(), 0);
    QCOMPARE(spy2.count(), 0);
    QCOMPARE(spy3.count(), 0);
    QCOMPARE(spy4.count(), 0);
    QCOMPARE(spy5.count(), 0);
    QCOMPARE(spy6.count(), 0);
    */
    QSKIP("Test is not implemented.", SkipAll);
}

// -------------------------------------------

void MainViewTest::currentChanged_data()
{
    QTest::addColumn<int>("foo");
    QTest::newRow("null") << 0;
}

void MainViewTest::currentChanged()
{
    /*
    QFETCH(int, foo);

    SubMainView widget;

    QSignalSpy spy0(view, SIGNAL(linkHovered(const QString &)));
    QSignalSpy spy2(view, SIGNAL(loadProgress(int)));
    QSignalSpy spy3(view, SIGNAL(setCurrentTitle(const QString &)));
    QSignalSpy spy4(view, SIGNAL(showStatusBarMessage(const QString &)));
    QSignalSpy spy5(view, SIGNAL(tabsChanged()));
    QSignalSpy spy6(view, SIGNAL(lastTabClosed()));

    widget.call_tabsChanged();

    QCOMPARE(spy0.count(), 0);
    QCOMPARE(spy2.count(), 0);
    QCOMPARE(spy3.count(), 0);
    QCOMPARE(spy4.count(), 0);
    QCOMPARE(spy5.count(), 0);
    QCOMPARE(spy6.count(), 0);
    */
    QSKIP("Test is not implemented.", SkipAll);
}

// -------------------------------------------

QTEST_KDEMAIN(MainViewTest, GUI)
#include "mainview_test.moc"
