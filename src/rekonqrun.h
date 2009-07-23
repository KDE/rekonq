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

#ifndef REKONQRUN_H
#define REKONQRUN_H

// KDE Includes
#include <KUrl>

// Qt Includes
#include <QtCore/QString>
#include <QtGui/QWidget>

namespace Rekonq
{
    /**
     * @short Open link options
     * Different modes of opening new tab
     */
    enum OpenType
    {
        Default,    ///< open url according to users settings
        New,        ///< open url in new tab and make it current
        Background  ///< open url in new tab in background
    };
}


/**
 * RekonqRun is not inherited from KRun or KParts::BrowserRun, as probably expected.
 * We (actually) use this class just to find the REAL url to load (and load it!), so it does
 * these operations:
 *
 * - stop loading malformed urls
 * - find url scheme on relative urls
 * - find right url to load on SearchProviders (Google, Wikipedia, etc.)
 * - handle local urls (launches dolphin)
 * - guess urls from strings
 * 
*/

class RekonqRun : public QObject
{
    Q_OBJECT

public:
    RekonqRun(QWidget *parent = 0);
    ~RekonqRun();
    
    
public slots:

    void loadUrl( const KUrl& url,
                  const Rekonq::OpenType& type = Rekonq::Default
                );
           
    void loadUrl( const QString& urlString,
                  const Rekonq::OpenType& type = Rekonq::Default
                );    
             
private:
    
    KUrl guessUrlFromString(const QString &url);

    QWidget *m_window;
};

#endif
