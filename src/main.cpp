/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2008-2010 by Andrea Diamantini <adjam7 at gmail dot com>
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


// version include
#include "../version.h"

// Local Includes
#include "application.h"
#include "sessionmanager.h"

// KDE Includes
#include <KAboutData>
#include <KCmdLineArgs>
#include <KDebug>


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
                     ki18n("(C) 2008-2010 Andrea Diamantini"),
                     KLocalizedString(),
                     "http://rekonq.sourceforge.net"
                    );

    // --------------- about authors -----------------------------
    about.addAuthor(ki18n("Andrea Diamantini"),
                    ki18n("Project Lead, Developer, Maintainer"),
                    "adjam7@gmail.com",
                    "http://www.adjam.org");

    about.addAuthor(ki18n("Panagiotis Papadopoulos"),
                    ki18n("Quite everything but code"),
                    "pano_90@gmx.net",
                    "");

    about.addAuthor(ki18n("Lionel Chauvin"),
                    ki18n("Developer, Ideas, Mockups, rekonq Icon"),
                    "megabigbug@yahoo.fr",
                    "");
    
    about.addAuthor(ki18n("Johannes Zellner"),
                    ki18n("Patches, Suggestions, Testing, Bugfixing"),
                    "webmaster@nebulon.de",
                    "");

    about.addAuthor(ki18n("Matthieu Gicquel"),
                    ki18n("Developer, Ideas, New Tab Page improvements"),
                    "matgic78@gmail.com",
                    "");

    about.addAuthor(ki18n("Ronny Scholz"),
                    ki18n("(Tons of) Patches, Testing, Bugfixing"),
                    "ronny_scholz@web.de",
                    "");

    about.addAuthor(ki18n("Yoann Laissus"),
                    ki18n("Developer, History & Bookmarks Improvements"),
                    "yoann.laissus@gmail.com",
                    "");

    about.addAuthor(ki18n("Cédric Bellegarde"),
                    ki18n("Patched code quite everywhere :)"),
                    "gnumdk@adishatz.1s.fr",
                    "");

    about.addAuthor(ki18n("Nikhil Marathe"),
                    ki18n("Bugfixing, Support for Chrome extensions (not yet available)"),
                    "nsm.nikhil@gmail.com",
                    "");

    about.addAuthor(ki18n("Rohan Garg"),
                    ki18n("Handbook, Maintains a Kubuntu PPA with rekonq git packages"),
                    "rohan16garg@gmail.com",
                    "");

    about.addAuthor(ki18n("Jonathan Raphael Joachim Kolberg"),
                    ki18n("Handbook, Maintains a Kubuntu PPA with rekonq git packages"),
                    "bulldog98@freenet.de",
                    "");
                    
    // --------------- about credits -----------------------------
    about.addCredit(ki18n("Lindsay Mathieson"),
                    ki18n("Provided Patches & Hints, Discovered Bugs"),
                    "lindsay.mathieson@gmail.com",
                    "");
                    
    about.addCredit(ki18n("Abdurrahman AVCI"),
                    ki18n("Provided Patches & Hints"),
                    "abdurrahmanavci@gmail.com",
                    "");

    about.addCredit(ki18n("Domrachev Alexandr"),
                    ki18n("Former Developer"),
                    "alexandr.domrachev@gmail.com",
                    "");

    about.addCredit(ki18n("Abuus"),
                    ki18n("Webview mouse event support"),
                    "buusmail@gmail.com",
                    "");

    about.addCredit(ki18n("Ivan Čukić"),
                    ki18n("Patches, Bugfixing and Ideas"),
                    "ivan@fomentgroup.org",
                    "");

    about.addCredit(ki18n("Adrià Arrufat"),
                    ki18n("New Tab Page loading animation"),
                    "swiftscythe@gmail.com",
                    "");

    about.addCredit(ki18n("Pawel Prazak"),
                    ki18n("Former Developer"),
                    "kojots350@gmail.com",
                    "");

    about.addCredit(ki18n("Dario Freddi"),
                    ki18n("Patches, Hints, First implementation of KWallet support"),
                    "drf@kde.org",
                    "");

    about.addCredit(ki18n("Jon de Andrés Frías"),
                    ki18n("First awesome bar implementation"),
                    "jondeandres@gmail.com",
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
