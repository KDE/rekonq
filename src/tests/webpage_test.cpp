/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2010-2011 by Andrea Diamantini <adjam7 at gmail dot com>
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

#include "webpage.h"
#include "webview.h"
#include "webtab.h"


class WebPageTest : public QObject
{
    Q_OBJECT

public slots:
    void initTestCase();
    void cleanupTestCase();

private slots:
    void manageNetworkErrors();
    void downloadRequest();
    void downloadAllContentsWithKGet();

    void createWindow();
    void acceptNavigationRequest();

    void handleUnsupportedContent();

    void loadFinished();

private:
    WebTab *tab;
    WebPage *page;
    WebView *view;
};


// -------------------------------------------


void WebPageTest::initTestCase()
{
    tab = new WebTab;
    view = tab->view();
    page = tab->page();
}


void WebPageTest::cleanupTestCase()
{
    delete tab;
}


// -------------------------------------------


void WebPageTest::manageNetworkErrors()
{
}

void WebPageTest::downloadRequest()
{
}

void WebPageTest::downloadAllContentsWithKGet()
{
}

void WebPageTest::createWindow()
{
}

void WebPageTest::acceptNavigationRequest()
{
}

void WebPageTest::handleUnsupportedContent()
{
}

void WebPageTest::loadFinished()
{
}

// -------------------------------------------

QTEST_KDEMAIN(WebPageTest, GUI)
#include "webpage_test.moc"
