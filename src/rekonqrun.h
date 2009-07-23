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
     * @short opening tab behaviour
     * Different open tab behaviour
     */
    
    enum BrowserArguments
    {
        Default,    ///< follow rekonq settings
        New,        ///< open tab with focus
        Background  ///< open tab in the background
    };
}


class RekonqRun : public QObject
{
    Q_OBJECT

public:
    RekonqRun(QWidget *parent = 0);
    ~RekonqRun();
    
    
public slots:

    void loadUrl( const KUrl& url,
                  const Rekonq::BrowserArguments& browserArgs = Rekonq::Default
                );
           
    void loadUrl( const QString& urlString,
                  const Rekonq::BrowserArguments& browserArgs = Rekonq::Default
                );    
             
private:
    
    KUrl guessUrlFromString(const QString &url);

    QWidget *m_window;
};

#endif
