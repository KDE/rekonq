/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2008-2013 by Andrea Diamantini <adjam7 at gmail dot com>
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
#include <config-version.h>

// Local Includes
#include "application.h"
#include "sessionmanager.h"
#include "rekonqwindow.h"
#include "urlresolver.h"

// KDE Includes
#include <KDE/KAboutData>
#include <KDE/KUniqueApplication>
#include <KDE/KCmdLineArgs>
#include <KDebug>

// Qt Includes
#include <QDir>
#include <QUrl>


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
                     ki18n("(C) 2008-2013 Andrea Diamantini"),
                     KLocalizedString(),
                     "http://rekonq.kde.org"
                    );


    // --------------- about authors -----------------------------
    about.addAuthor(ki18n("Andrea Diamantini"),
                    ki18n("Project Lead, Developer, Maintainer"),
                    "adjam7@gmail.com",
                    "http://www.adjam.org");

    about.addAuthor(ki18n("Johannes Tröscher"),
                    ki18n("QGraphicsEffect expert. Tabbar highlight animation"),
                    "fritz_van_tom@hotmail.com",
                    "");

    about.addAuthor(ki18n("Furkan Uzumcu"),
                    ki18n("A lot of improvements, especially on usability"),
                    "furkanuzumcu@gmail.com",
                    "");

    about.addAuthor(ki18n("Yoann Laissus"),
                    ki18n("Developer, History & Bookmarks Improvements"),
                    "yoann.laissus@gmail.com",
                    "");

    about.addAuthor(ki18n("Cédric Bellegarde"),
                    ki18n("Patched code quite everywhere :)"),
                    "gnumdk@adishatz.1s.fr",
                    "");

    about.addAuthor(ki18n("Jon Ander Peñalba"),
                    ki18n("Bookmarks code peer reviewer. A fantastic help"),
                    "jonan88@gmail.com",
                    "http://identi.ca/jonan");

    about.addAuthor(ki18n("Pierre Rossi"),
                    ki18n("Urlbar, tests, new tab page, bars... and more"),
                    "pierre.rossi@gmail.com",
                    "");

    about.addAuthor(ki18n("Lionel Chauvin"),
                    ki18n("Development, Ideas, Mockups, rekonq Icon"),
                    "megabigbug@yahoo.fr",
                    "");

    about.addAuthor(ki18n("Siteshwar Vashisht"),
                    ki18n("Code, Ideas, sync... and IRC chats!"),
                    "siteshwar@gmail.com",
                    "");

    about.addAuthor(ki18n("Tirtha Chatterjee"),
                    ki18n("A lot of nice work, here and there in the code :)"),
                    "tirtha.p.chatterjee@gmail.com",
                    "");

    about.addAuthor(ki18n("Lindsay Mathieson"),
                    ki18n("Implemented inline spellcheck, provided hints, discovered bugs"),
                    "lindsay.mathieson@gmail.com",
                    "");


    // --------------- about credits -----------------------------
    about.addCredit(ki18n("Dawit Alemayehu"),
                    ki18n("KDEWebKit (main) developer. And KIO. And KUriFilter. And more.."),
                    "adawit@kde.org",
                    "");

    about.addCredit(ki18n("Jekyll Wu"),
                    ki18n("Bug triaging. Impressive job about..."),
                    "adaptee@gmail.com",
                    "");

    about.addCredit(ki18n("Dimitrios Christidis"),
                    ki18n("Provides patches, fixes and good testing"),
                    "dchristidis@ceid.upatras.gr",
                    "");

    about.addCredit(ki18n("Panagiotis Papadopoulos"),
                    ki18n("Quite everything but code"),
                    "pano_90@gmx.net",
                    "");

    about.addCredit(ki18n("Phaneendra Hedge"),
                    ki18n("Nepomuk fancy bookmarking"),
                    "pnh.pes@gmail.com",
                    "");

    about.addCredit(ki18n("Jonathan Raphael Joachim Kolberg"),
                    ki18n("Handbook, Maintains a Kubuntu PPA with rekonq git packages"),
                    "bulldog98@freenet.de",
                    "");

    about.addCredit(ki18n("Benjamin Poulain"),
                    ki18n("The \"QtWebKit guy\". Adblock (new) implementation. Code quality improvements"),
                    "ikipou@gmail.com",
                    "http://www.openyourcode.org/");

    about.addCredit(ki18n("Rohan Garg"),
                    ki18n("Handbook, Maintains a Kubuntu PPA with rekonq git packages."),
                    "rohan16garg@gmail.com",
                    "");

    about.addCredit(ki18n("Anton Kreuzkamp"),
                    ki18n("Session Management, patches"),
                    "akreuzkamp@web.de",
                    "");

    about.addCredit(ki18n("David E. Narváez"),
                    ki18n("Implemented User Session Management and cleaned up SessionManager code"),
                    "david.narvaez@computer.org",
                    "");

    about.addCredit(ki18n("Marc Deop"),
                    ki18n("Access Keys Navigation"),
                    "damnshock@gmail.com",
                    "");

    about.addCredit(ki18n("Yuri Chornoivan"),
                    ki18n("Checking rekonq strings, helping with docs"),
                    "yurchor@ukr.net",
                    "");

    about.addCredit(ki18n("Burkhard Lück"),
                    ki18n("Checking rekonq strings, helping with docs"),
                    "lueck@hube-lueck.de",
                    "");

    about.addCredit(ki18n("Andrius da Costa Ribas"),
                    ki18n("Helped letting rekonq compile on Windows/MSVC and Mac OS X"),
                    "andriusmao@gmail.com",
                    "");

    about.addCredit(ki18n("Pino Toscano"),
                    ki18n("fixuifiles ;)"),
                    "pino@kde.org",
                    "");


    // Initialize command line args
    KCmdLineArgs::init(argc, argv, &about);

    // Define the command line options using KCmdLineOptions
    KCmdLineOptions options;

    // adding options
    options.add("incognito" , ki18n("Open in incognito mode"));
    options.add("webapp" , ki18n("Open URL as web app (in a simple window)"));
    options.add("+[URL]" , ki18n("Location to open"));

    // Register the supported options
    KCmdLineArgs::addCmdLineOptions(options);

    if (!Application::start())
    {
        kWarning() << "rekonq is already running!";
        return 0;
    }

#if defined(Q_WS_X11)
    // On X11, the raster engine gives better performance than native.
    QApplication::setGraphicsSystem(QLatin1String("raster"));
#endif

    KCmdLineArgs::setCwd(QDir::currentPath().toUtf8());

    Application app;

    // set application data
    QCoreApplication::setApplicationName(QLatin1String("rekonq"));
    QCoreApplication::setApplicationVersion(REKONQ_VERSION);

    if (app.isSessionRestored())
    {
        for (int i = 1; RekonqWindow::canBeRestored(i); i++)
        {
            RekonqWindow * newRekonqWindow = app.newWindow(false);
            if (newRekonqWindow->restore(i))
                SessionManager::self()->restoreWindow(newRekonqWindow);
        }
    }
    
    return app.exec();
}
