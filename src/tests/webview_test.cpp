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

#include <QtCore>
#include <QtGui>
#include <QtTest>
#include <QtWebKit>

#include "webview.h"
#include "webtab.h"

class WebViewTest : public QObject
{
    Q_OBJECT
    
public slots:
    void initTestCase();
    void cleanupTestCase();
    
private slots:

private:
    WebView *view;
    WebTab *tab;
};


// -------------------------------------------


void WebViewTest::initTestCase()
{
    tab = new WebTab;
    view = tab->view();
}


void WebViewTest::cleanupTestCase()
{
}
    
    
// -------------------------------------------


// -------------------------------------------

QTEST_KDEMAIN(WebViewTest,GUI)
#include "webview_test.moc"
