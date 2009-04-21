/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2008 by Andrea Diamantini <adjam7 at gmail dot com>
*
*
* This program is free software; you can redistribute it
* and/or modify it under the terms of the GNU General
* Public License as published by the Free Software Foundation;
* either version 2, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* ============================================================ */


// Self Includes
#include "searchbar.h"
#include "searchbar.moc"

// Local Includes
#include "application.h"
#include "mainwindow.h"

// KDE Includes
#include <KUrl>
#include <KDebug>

// Qt Includes
#include <QtCore>
#include <QtGui>
#include <QtNetwork>


SearchBar::SearchBar(QWidget *parent) :
    KLineEdit(parent)
    , m_networkAccessManager(new QNetworkAccessManager(this))
    , m_timer(new QTimer(this))
{
    setMinimumWidth(180);
    kWarning() << "setting fixed minimum width.." ;

    setFocusPolicy(Qt::WheelFocus);
    setMouseTracking(true);
    setAcceptDrops(true);

    QSizePolicy policy = sizePolicy();
    setSizePolicy(QSizePolicy::Preferred, policy.verticalPolicy());

    setClearButtonShown(true);

    // better solution than using QPalette(s)..
    setClickMessage(i18n("Search.."));

    // setting QNetworkAccessManager..
    connect(m_networkAccessManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(handleNetworkData(QNetworkReply*)));

    // setting QTimer..
    m_timer->setSingleShot(true);
    m_timer->setInterval(300);
    connect(m_timer, SIGNAL(timeout()), SLOT(autoSuggest()));
    connect(this, SIGNAL(textEdited(QString)), m_timer, SLOT(start()));

    // connect searchNow slot..
    connect(this, SIGNAL(returnPressed()) , this , SLOT(searchNow()));
}


SearchBar::~SearchBar()
{
}


void SearchBar::searchNow()
{
    m_timer->stop();
    QString searchText = text();

    KUrl url(QLatin1String("http://www.google.com/search"));
    url.addQueryItem(QLatin1String("q"), searchText);
    url.addQueryItem(QLatin1String("ie"), QLatin1String("UTF-8"));
    url.addQueryItem(QLatin1String("oe"), QLatin1String("UTF-8"));
    url.addQueryItem(QLatin1String("client"), QLatin1String("rekonq"));
    emit search(url);
}


void SearchBar::focusInEvent(QFocusEvent *event)
{
    KLineEdit::focusInEvent(event);
    clear();
}


void SearchBar::autoSuggest()
{
    QString str = text();
    QString url = QString("http://google.com/complete/search?output=toolbar&q=%1").arg(str);
    m_networkAccessManager->get(QNetworkRequest(QString(url)));
}


void SearchBar::handleNetworkData(QNetworkReply *networkReply)
{
    QUrl url = networkReply->url();
    if (!networkReply->error())
    {
        QStringList choices;

        QString response(networkReply->readAll());
        QXmlStreamReader xml(response);
        while (!xml.atEnd())
        {
            xml.readNext();
            if (xml.tokenType() == QXmlStreamReader::StartElement)
                if (xml.name() == "suggestion")
                {
                    QStringRef str = xml.attributes().value("data");
                    choices << str.toString();
                }
        }

        setCompletedItems(choices, true);
    }

    networkReply->deleteLater();
}
