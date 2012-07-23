/***************************************************************************
 *   Copyright (C) 2012 by Andrea Diamantini <adjam7@gmail.com>                            *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/


// version include
#include "config-version.h"

#include "tabwindow.h"

#include <KDE/KAboutData>
#include <KDE/KApplication>
#include <KDE/KCmdLineArgs>

#include <QUrl>
#include <QWebSettings>


static const char description[] =
    I18N_NOOP("A lightweight Web Browser for KDE based on WebKit");

    
extern "C" KDE_EXPORT int kdemain(int argc, char **argv)
{
    KAboutData about("rekonq",
                    0,
                    ki18n("rekonq"),
                    REKONQ_VERSION,
                    ki18n(description),
                    KAboutData::License_GPL_V3,
                    ki18n("(C) 2008-2012 Andrea Diamantini"),
                    KLocalizedString(),
                    "http://rekonq.kde.org"
                );

    // Initialize command line args
    KCmdLineArgs::init(argc, argv, &about);

    // Define the command line options using KCmdLineOptions
    KCmdLineOptions options;

    // adding URL option
    options.add("+[URL]" , ki18n("Location to open"));

    // Register the supported options
    KCmdLineArgs::addCmdLineOptions(options);

    KApplication app;

    QWebSettings::setIconDatabasePath("/tmp/iconcache");
    
    TabWindow *w = new TabWindow;
    
    // no session.. just start up normally
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    if (args->count() == 0)
    {
        w->newCleanTab();
        w->show();
    }
    else
    {
        int i = 0;
        for (; i < args->count(); i++)
        {
            w->loadUrlInNewTab(QUrl::fromUserInput(args->arg(i)));
            w->show();
        }
    }
    args->clear();

    return app.exec();
}
