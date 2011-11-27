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

#include "protocolhandler.h"

#include <QWebView>
#include <QNetworkRequest>


class ProtocolhandlerTest : public QObject
{
    Q_OBJECT

public Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

private Q_SLOTS:
    void preHandling_data();
    void preHandling();

    void postHandling_data();
    void postHandling();

private:
    ProtocolHandler *handler;
};


// -------------------------------------------

void ProtocolhandlerTest::initTestCase()
{
    handler = new ProtocolHandler;
}


void ProtocolhandlerTest::cleanupTestCase()
{
    delete handler;
}


// -------------------------------------------


void ProtocolhandlerTest::preHandling_data()
{
    QTest::addColumn<QString>("urlString");
    QTest::addColumn<bool>("result");

    QTest::newRow("mailto")     << "mailto:me@here.com"             << true  ;
    QTest::newRow("relative")   << "google.it"                      << false ;
    QTest::newRow("javascript") << "javascript:alertbox('hello')"   << true  ;
    QTest::newRow("aboutblank") << "about:blank"                    << false ;
    QTest::newRow("abouthome")  << "about:home"                     << true  ;
    QTest::newRow("ftp")        << "ftp://ftp.kde.org"              << false ;
    QTest::newRow("file")       << "file:///home"                   << false ;
}


void ProtocolhandlerTest::preHandling()
{
    QFETCH(QString, urlString);
    QFETCH(bool   , result);

    QWebView *view = new QWebView;
    QWebFrame *frame = view->page()->mainFrame();

    QNetworkRequest request = QNetworkRequest(QUrl(urlString));

    QCOMPARE(handler->preHandling(request, frame) , result);
}


void ProtocolhandlerTest::postHandling_data()
{
    QTest::addColumn<QString>("urlString");
    QTest::addColumn<bool>("result");

    QTest::newRow("mailto")     << "mailto:me@here.com"             << true  ;
    QTest::newRow("relative")   << "google.it"                      << false ;
    QTest::newRow("javascript") << "javascript:alertbox('hello')"   << false ;
    QTest::newRow("aboutblank") << "about:blank"                    << false ;
    QTest::newRow("abouthome")  << "about:home"                     << false ;
    QTest::newRow("ftp")        << "ftp://ftp.kde.org"              << true  ;
    QTest::newRow("file")       << "file:///home"                   << true  ;
}


void ProtocolhandlerTest::postHandling()
{
    QFETCH(QString, urlString);
    QFETCH(bool   , result);

    QWebView *view = new QWebView;
    QWebFrame *frame = view->page()->mainFrame();

    QNetworkRequest request = QNetworkRequest(QUrl(urlString));

    QCOMPARE(handler->postHandling(request, frame) , result);
}

// -------------------------------------------

QTEST_KDEMAIN(ProtocolhandlerTest, GUI)
#include "protocolhandler_test.moc"
