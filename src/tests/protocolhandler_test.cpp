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
    
    QTest::newRow("mailto")     << "mailto:me@here.com";
    QTest::newRow("relative")   << "google.it";
    QTest::newRow("javascript") << "javascript:alertbox('hello')";
    QTest::newRow("aboutblank") << "about:blank";
    QTest::newRow("abouthome")  << "about:home";
    QTest::newRow("ftp")        << "ftp://ftp.kde.org";
    QTest::newRow("file")       << "file:///home";
}


void ProtocolhandlerTest::preHandling()
{
//     QFETCH( QString, urlString );
//     
//     QWebView *view = new QWebView;
//     QWebFrame *frame = view->page()->mainFrame();
//     
//     QNetworkRequest *request = new QNetworkRequest( QUrl(urlString) );
//     handler->preHandling( request, frame );
}


void ProtocolhandlerTest::postHandling_data()
{
    QTest::addColumn<QString>("urlString");
    
    QTest::newRow("mailto")     << "mailto:me@here.com";
    QTest::newRow("relative")   << "google.it";
    QTest::newRow("javascript") << "javascript:alertbox('hello')";
    QTest::newRow("aboutblank") << "about:blank";
    QTest::newRow("abouthome")  << "about:home";
    QTest::newRow("ftp")        << "ftp://ftp.kde.org";
    QTest::newRow("file")       << "file:///home";
}


void ProtocolhandlerTest::postHandling()
{
}
    
// -------------------------------------------

QTEST_KDEMAIN(ProtocolhandlerTest,GUI)
#include "protocolhandler_test.moc"
