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

#include "protocolhandler.h"


class ProtocolhandlerTest : public QObject
{
    Q_OBJECT

public slots:
    void initTestCase();
    void cleanupTestCase();

private slots:
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
