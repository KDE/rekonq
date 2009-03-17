/* ============================================================
 *
 * This file is a part of the rekonq project
 *
 * Copyright (C) 2007-2008 Trolltech ASA. All rights reserved
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


#include "application.h"

#include <KAboutData>
#include <KCmdLineArgs>
#include <KDebug>

static const char description[] =
    I18N_NOOP("KDE Browser Webkit Based");

static const char version[] = "0.0.4";

int main(int argc, char **argv)
{
    KAboutData about(   "rekonq", 
                        0, 
                        ki18n("rekonq"), 
                        version, 
                        ki18n(description),
                        KAboutData::License_GPL, 
                        ki18n("(C) 2008 Andrea Diamantini"), 
                        KLocalizedString(), 
                        "http://rekonq.sourceforge.net",
                        "adjam7@gmail.com"
                    );

    about.addAuthor(    ki18n("Andrea Diamantini"), 
                        KLocalizedString(), 
                        "adjam7@gmail.com" 
                   );

    KCmdLineArgs::init(argc, argv, &about);

    KCmdLineOptions options;
    options.add( "+URL" , ki18n("Location to open") );
    KCmdLineArgs::addCmdLineOptions( options );
    Application::addCmdLineOptions();

    if (!Application::start()) 
    {
       kWarning() << "rekonq is already running!";
       return 0;
    }
    Application app;
    return app.exec();
}

