/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2008-2014 by Andrea Diamantini <adjam7 at gmail dot com>
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

// Rekonq Includes
#include "rekonq_defines.h"

// Local Includes
#include "application.h"
#include "sessionmanager.h"
#include "rekonqwindow.h"
#include "urlresolver.h"

// KDE Includes
#include <KAboutData>
#include <KLocalizedString>

// Qt Includes
#include <QCommandLineParser>
#include <QUrl>


static const char description[] =
    I18N_NOOP("A lightweight Web Browser for KDE based on WebKit");


extern "C" Q_DECL_EXPORT int kdemain(int argc, char **argv)
{
    Application app(argc, argv);

    // set application data
    QCoreApplication::setApplicationName(QL1S("rekonq"));
    QCoreApplication::setApplicationVersion( QL1S(REKONQ_VERSION) );
    QCoreApplication::setOrganizationDomain(QL1S("kde.org"));

    KAboutData about(QL1S("rekonq"),
                     QL1S("rekonq"),
                     QL1S(REKONQ_VERSION),
                     i18n(description),
                     KAboutData::License_GPL_V3,
                     i18n("(C) 2008-2014 Andrea Diamantini"),
                     QString(),
                     QL1S("http://rekonq.kde.org")
                    );


    // --------------- about authors -----------------------------
    about.addAuthor(i18n("Andrea Diamantini"),
                    i18n("Project Lead, Developer, Maintainer"),
                    QL1S("adjam7@gmail.com"),
                    QL1S("http://www.adjam.org"));

    about.addAuthor(i18n("Johannes Tröscher"),
                    i18n("QGraphicsEffect expert. Tabbar highlight animation"),
                    QL1S("fritz_van_tom@hotmail.com"));

    about.addAuthor(i18n("Furkan Uzumcu"),
                    i18n("A lot of improvements, especially on usability"),
                    QL1S("furkanuzumcu@gmail.com"));

    about.addAuthor(i18n("Yoann Laissus"),
                    i18n("Developer, History & Bookmarks Improvements"),
                    QL1S("yoann.laissus@gmail.com"));

    about.addAuthor(i18n("Cédric Bellegarde"),
                    i18n("Patched code quite everywhere :)"),
                    QL1S("gnumdk@adishatz.1s.fr"));

    about.addAuthor(i18n("Jon Ander Peñalba"),
                    i18n("Bookmarks code peer reviewer. A fantastic help"),
                    QL1S("jonan88@gmail.com"),
                    QL1S("http://identi.ca/jonan"));

    about.addAuthor(i18n("Pierre Rossi"),
                    i18n("Urlbar, tests, new tab page, bars... and more"),
                    QL1S("pierre.rossi@gmail.com"));

    about.addAuthor(i18n("Lionel Chauvin"),
                    i18n("Development, Ideas, Mockups, rekonq Icon"),
                    QL1S("megabigbug@yahoo.fr"));

    about.addAuthor(i18n("Siteshwar Vashisht"),
                    i18n("Code, Ideas, sync... and IRC chats!"),
                    QL1S("siteshwar@gmail.com"));

    about.addAuthor(i18n("Tirtha Chatterjee"),
                    i18n("A lot of nice work, here and there in the code :)"),
                    QL1S("tirtha.p.chatterjee@gmail.com"));

    about.addAuthor(i18n("Lindsay Mathieson"),
                    i18n("Implemented inline spellcheck, provided hints, discovered bugs"),
                    QL1S("lindsay.mathieson@gmail.com"));


    // --------------- about credits -----------------------------
    about.addCredit(i18n("Dawit Alemayehu"),
                    i18n("KDEWebKit (main) developer. And KIO. And KUriFilter. And more.."),
                    QL1S("adawit@kde.org"));

    about.addCredit(i18n("Jekyll Wu"),
                    i18n("Bug triaging. Impressive job about..."),
                    QL1S("adaptee@gmail.com"));

    about.addCredit(i18n("Dimitrios Christidis"),
                    i18n("Provides patches, fixes and good testing"),
                    QL1S("dchristidis@ceid.upatras.gr"));

    about.addCredit(i18n("Panagiotis Papadopoulos"),
                    i18n("Quite everything but code"),
                    QL1S("pano_90@gmx.net"));

    about.addCredit(i18n("Phaneendra Hedge"),
                    i18n("Nepomuk fancy bookmarking"),
                    QL1S("pnh.pes@gmail.com"));

    about.addCredit(i18n("Jonathan Raphael Joachim Kolberg"),
                    i18n("Handbook, Maintains a Kubuntu PPA with rekonq git packages"),
                    QL1S("bulldog98@freenet.de"));

    about.addCredit(i18n("Benjamin Poulain"),
                    i18n("The \"QtWebKit guy\". Adblock (new) implementation. Code quality improvements"),
                    QL1S("ikipou@gmail.com"),
                    QL1S("http://www.openyourcode.org/"));

    about.addCredit(i18n("Rohan Garg"),
                    i18n("Handbook, Maintains a Kubuntu PPA with rekonq git packages."),
                    QL1S("rohan16garg@gmail.com"));

    about.addCredit(i18n("Anton Kreuzkamp"),
                    i18n("Session Management, patches"),
                    QL1S("akreuzkamp@web.de"));

    about.addCredit(i18n("David E. Narváez"),
                    i18n("Implemented User Session Management and cleaned up SessionManager code"),
                    QL1S("david.narvaez@computer.org"));

    about.addCredit(i18n("Marc Deop"),
                    i18n("Access Keys Navigation"),
                    QL1S("damnshock@gmail.com"));

    about.addCredit(i18n("Yuri Chornoivan"),
                    i18n("Checking rekonq strings, helping with docs"),
                    QL1S("yurchor@ukr.net"));

    about.addCredit(i18n("Burkhard Lück"),
                    i18n("Checking rekonq strings, helping with docs"),
                    QL1S("lueck@hube-lueck.de"));

    about.addCredit(i18n("Andrius da Costa Ribas"),
                    i18n("Helped letting rekonq compile on Windows/MSVC and Mac OS X"),
                    QL1S("andriusmao@gmail.com"));

    about.addCredit(i18n("Pino Toscano"),
                    i18n("fixuifiles ;)"),
                    QL1S("pino@kde.org"));

    KAboutData::setApplicationData(about);
        
// -----------------------------------------------------------------------------------------------------------------
    

    QCommandLineParser parser;
    about.setupCommandLine(&parser);
    parser.setApplicationDescription(about.shortDescription());
    parser.addHelpOption();
    parser.addVersionOption();
    
//     // Initialize command line args
//     KCmdLineArgs::init(argc, argv, &about);
// 
//     // Define the command line options using KCmdLineOptions
//     KCmdLineOptions options;
// 
//     // adding options
//     options.add("incognito" , i18n("Open in incognito mode"));
//     options.add("webapp" , i18n("Open URL as web app (in a simple window)"));
//     options.add("+[URL]" , i18n("Location to open"));
// 
//     // Register the supported options
//     KCmdLineArgs::addCmdLineOptions(options);


// #if defined(Q_WS_X11)
//     // On X11, the raster engine gives better performance than native.
//     QApplication::setGraphicsSystem(QL1S("raster"));
// #endif
// 
//     KCmdLineArgs::setCwd(QDir::currentPath().toUtf8());


    // FIXME
//     if (app.isSessionRestored())
//     {
//         for (int i = 1; RekonqWindow::canBeRestored(i); i++)
//         {
//             RekonqWindow * newRekonqWindow = app.newWindow(false);
//             if (newRekonqWindow->restore(i))
//                 SessionManager::self()->restoreWindow(newRekonqWindow);
//         }
//     }
    
    return app.exec();
}
