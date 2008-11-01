/* ============================================================
 *
 * This file is a part of the reKonq project
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */


#include "browserapplication.h"

#include <KAboutData>
#include <KCmdLineArgs>


// really simple writing main files today..
int main(int argc, char **argv)
{
    KAboutData aboutData(
        "reKonq",
        0, 
        ki18n("reKonq"), 
        "0.0.1", 
        ki18n("A KDE browser webkit based"),
        KAboutData::License_GPL,
        ki18n("Copyright (c) 2008 Andrea Diamantini"),
        KLocalizedString(),
        "http://www.adjam.org",
        "adjam7@gmail.com"  // bug report mail
    );

    aboutData.addAuthor(ki18n("Andrea Diamantini"), ki18n("reKonq author"), "adjam7@gmail.com");
    aboutData.setProgramIconName("applications-internet");

    KCmdLineArgs::init( argc, argv, &aboutData );
    
    KCmdLineOptions options;
    options.add("+URL", ki18n("Location to open"));
    KCmdLineArgs::addCmdLineOptions( options );

    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

    BrowserApplication app(args, "reKonq");
    if (!app.isTheOnlyBrowser())
        return 0;
    app.newMainWindow();
    return app.exec();
}

