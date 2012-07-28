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

#include "application.h"
#include "tabwindow.h"
#include "urlresolver.h"

#include <KDE/KAboutData>
#include <KDE/KUniqueApplication>
#include <KDE/KCmdLineArgs>
#include <KDebug>

#include <QDir>
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

    if (!Application::start())
    {
        kWarning() << "rekonq is already running!";
        return 0;
    }

#if defined(Q_WS_X11)
    // On X11, the raster engine gives better performance than native.
    QApplication::setGraphicsSystem(QLatin1String("raster"));
#endif

    Application app;

    QWebSettings::setIconDatabasePath("/tmp/iconcache");

    // set application data
    QCoreApplication::setApplicationName(QLatin1String("rekonq"));
    QCoreApplication::setApplicationVersion(REKONQ_VERSION);

    KCmdLineArgs::setCwd(QDir::currentPath().toUtf8());

//     if (app.isSessionRestored())
//         for (int i = 1; MainWindow::canBeRestored(i); i++)
//             app.newMainWindow(false)->restore(i);

    return app.exec();
}
