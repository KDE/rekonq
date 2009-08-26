/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2008-2009 by Andrea Diamantini <adjam7 at gmail dot com>
*
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License as
* published by the Free Software Foundation; either version 2 of
* the License or (at your option) version 3 or any later version
* accepted by the membership of KDE e.V. (or its successor approved
* by the membership of KDE e.V.), which shall act as a proxy 
* defined in Section 14 of version 3 of the license.
* 
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
* ============================================================ */


// Local Includes
#include "application.h"

// KDE Includes
#include <KAboutData>
#include <KCmdLineArgs>
#include <KDebug>


static const char description[] =
    I18N_NOOP("A lightweight Web Browser for KDE based on Webkit");


static const char version[] = "0.2.52";


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
                     "http://rekonq.sourceforge.net"
                    );

    // --------------- about authors -----------------------------
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

    about.addAuthor(ki18n("Panagiotis Papadopoulos"),
                    ki18n("Quite everything but code"),
                    "pano_90@gmx.net",
                    "");

    about.addAuthor(ki18n("Lionel Chauvin"),
                    ki18n("Developer, Ideas, Mockups"),
                    "megabigbug@yahoo.fr",
                    "");

    // --------------- about credits -----------------------------                    
    about.addCredit(ki18n("Henry de Valence"),
                    ki18n("Promised help on multitask rekonq"),
                    "hdevalence@gmail.com",
                    "");

    about.addCredit(ki18n("Abuus"),
                    ki18n("Webview mouse event support"),
                    "buusmail@gmail.com",
                    "");

    about.addCredit(ki18n("Johannes Zellner"),
                    ki18n("Patches, suggestions, testing, bugfixing"),
                    "webmaster@nebulon.de",
                    "");

    about.addCredit(ki18n("Ivan Čukić"),
                    ki18n("Patches, bugfixing"),
                    "ivan@fomentgroup.org",
                    "");

    about.addCredit(ki18n("Adrià Arrufat"),
                    ki18n("New tab loading animation"),
                    "swiftscythe@gmail.com",
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
