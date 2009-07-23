/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2009 by Andrea Diamantini <adjam7 at gmail dot com>
*
*
* This program is free software; you can redistribute it
* and/or modify it under the terms of the GNU General
* Public License as published by the Free Software Foundation;
* either version 3, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* ============================================================ */


// Self Includes
#include "rekonqrun.h"
#include "rekonqrun.moc"

// Auto Includes
#include "rekonq.h"

// Local Includes
#include "mainview.h"
#include "urlbar.h"

// KDE Includes
#include <KToolInvocation>

// Qt Includes
#include <QRegExp>
#include <QFile>
#include <QFileInfo>
            

RekonqRun::RekonqRun(QWidget *parent = 0)
    : QObject(parent)
    , m_window(parent)
{
}


RekonqRun::~RekonqRun()
{
}


void RekonqRun::loadUrl(const KUrl& url, const Rekonq::OpenType& type)
{
    if (url.isEmpty())
        return;

    QString scheme = url.scheme();

    if (scheme == QLatin1String("mailto"))
    {
        KToolInvocation::invokeMailer(url);
        return;
    }

    KUrl loadingUrl(url);

    // create convenience fake api:// protocol for KDE apidox search and Qt docs
    if (scheme == QLatin1String("api"))
    {
        QString path;
        QString className = url.host().toLower();
        if (className[0] == 'k')
        {
            path = QString("http://api.kde.org/new.classmapper.php?class=%1").arg(className);
        }
        else if (className[0] == 'q')
        {
            path = QString("http://doc.trolltech.com/4.5/%1.html").arg(className);
        }
        loadingUrl.setUrl(path);
    }

    if (loadingUrl.isRelative())
    {
        if(loadingUrl.path().contains('.'))
        {
            QString fn = loadingUrl.url(KUrl::RemoveTrailingSlash);
            loadingUrl.setUrl("//" + fn);
            loadingUrl.setScheme("http");
        }
        else
        {
            scheme = QLatin1String("gg");
        }
    }

    // create convenience fake gg:// protocol, waiting for KServices learning
    if(scheme == QLatin1String("gg"))
    {
        QString str = loadingUrl.path();
        loadingUrl.setUrl( QString("http://google.com/search?&q=%1").arg(str) );
    }

    // create convenience fake wk:// protocol, waiting for KServices learning
    if(scheme == QLatin1String("wk"))
    {
        QString str = loadingUrl.path();
        loadingUrl.setUrl( QString("http://en.wikipedia.org/wiki/%1").arg(str) );
    }


    WebView *webView = m_window->newTab();
    m_window->currentUrlBar()->setUrl(loadingUrl.prettyUrl());

    switch(type)
    {
    case Rekonq::Default:
        if (!ReKonfig::openTabsBack())
        {
            setCurrentWidget(webView);  // this method does NOT take ownership of webView
        }
        break;
    case Rekonq::New:
        m_window->setCurrentWidget(webView);  // this method does NOT take ownership of webView
        break;
    case Rekonq::Background:
        break;
    };

    if (webView)
    {
        webView->setFocus();
        webView->load(loadingUrl);
    }
}


void RekonqRun::loadUrl(const QString& urlString,  const Rekonq::OpenType& type)
{    
    return loadUrl( guessUrlFromString(urlString), type );
}


KUrl RekonqRun::guessUrlFromString(const QString &string)
{
    QString urlStr = string.trimmed();
    QRegExp test(QLatin1String("^[a-zA-Z]+\\:.*"));

    // Check if it looks like a qualified URL. Try parsing it and see.
    bool hasSchema = test.exactMatch(urlStr);

    if (hasSchema)
    {
        QUrl qurl(urlStr, QUrl::TolerantMode);
        KUrl url(qurl);

        if (url.isValid())
        {
            return url;
        }
    }

    // Might be a file.
    if (QFile::exists(urlStr))
    {
        QFileInfo info(urlStr);
        return KUrl::fromPath(info.absoluteFilePath());
    }

    // Might be a shorturl - try to detect the schema.
    if (!hasSchema)
    {
        int dotIndex = urlStr.indexOf(QLatin1Char('.'));

        if (dotIndex != -1)
        {
            QString prefix = urlStr.left(dotIndex).toLower();
            QString schema = (prefix == QLatin1String("ftp")) ? prefix : QLatin1String("http");
            QUrl qurl(schema + QLatin1String("://") + urlStr, QUrl::TolerantMode);
            KUrl url(qurl);

            if (url.isValid())
            {
                return url;
            }
        }
    }

    // Fall back to QUrl's own tolerant parser.
    QUrl qurl = QUrl(string, QUrl::TolerantMode);
    KUrl url(qurl);

    // finally for cases where the user just types in a hostname add http
    if (qurl.scheme().isEmpty())
    {
        qurl = QUrl(QLatin1String("http://") + string, QUrl::TolerantMode);
        url = KUrl(qurl);
    }
    return url;
}
