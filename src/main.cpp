/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2008-2009 by Andrea Diamantini <adjam7 at gmail dot com>
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


// Local Includes
#include "application.h"

// KDE Includes
#include <KAboutData>
#include <KCmdLineArgs>
#include <KDebug>


static const char description[] =
    I18N_NOOP("WebKit-based Web Browser for KDE");


static const char version[] = "0.1.1";


int main(int argc, char **argv)
{
    KAboutData about("rekonq",
                     0,
                     ki18n("rekonq"),
                     version,
                     ki18n(description),
                     KAboutData::License_GPL_V3,
                     ki18n("(C) 2008-2009 Andrea Diamantini"),
                     KLocalizedString(),
                     "http://rekonq.sourceforge.net",
                     "rekonq@kde.org"
                    );

    // about authors
    about.addAuthor(ki18n("Andrea Diamantini"),
                    ki18n("Project Lead, Developer"),
                    "adjam7@gmail.com",
                    "http://www.adjam.org");

    about.addAuthor(ki18n("Domrachev Alexandr"),
                    ki18n("Developer"),
                    "alexandr.domrachev@gmail.com",
                    "");

    about.addAuthor(ki18n("Pawel Prazak"),
                    ki18n("Developer"),
                    "kojots350@gmail.com",
                    "");

    // Initialize command line args
    KCmdLineArgs::init(argc, argv, &about);

    // Define the command line options using KCmdLineOptions
    KCmdLineOptions options;

    // adding URL option
    options.add("+[URL]" , ki18n("Location to open"));

    // Register the supported options
    KCmdLineArgs::addCmdLineOptions(options);

    // Add options from Application class
    Application::addCmdLineOptions();

    if (!Application::start())
    {
        kWarning() << "rekonq is already running!";
        return 0;
    }

    Application app;
    return app.exec();
}
