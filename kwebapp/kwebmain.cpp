/*
 * This file is part of the KDE project.
 * Copyright (C) 2011 by Andrea Diamantini <adjam7@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) version 3, or any
 * later version accepted by the membership of KDE e.V. (or its
 * successor approved by the membership of KDE e.V.), which shall
 * act as a proxy defined in Section 6 of version 3 of the license.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */


#include "webview.h"

#include <KDE/KApplication>

#include <KDE/KAboutData>
#include <KDE/KCmdLineArgs>

#include <QDebug>
#include <QUrl>

static const char description[] =
    I18N_NOOP("Web Application Viewer");

static const char version[] = "0.1";

int main(int argc, char **argv)
{
    KAboutData about("kwebapp", 0, ki18n("kwebapp"), version, ki18n(description),
                     KAboutData::License_GPL, ki18n("(C) 2011 Andrea Diamantini"), KLocalizedString(), 0, "adjam7@gmail.com");
    about.addAuthor(ki18n("Andrea Diamantini"), KLocalizedString(), "adjam7@gmail.com");
    KCmdLineArgs::init(argc, argv, &about);

    KCmdLineOptions options;
    options.add("+[URL]", ki18n("Document to open"));
    KCmdLineArgs::addCmdLineOptions(options);

    KApplication app;

    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    if (args->count() != 1)
    {
        qDebug() << "ERROR: Impossible to launch kwebapp WITHOUT just ONE url to load!!!";
        return 0;
    }

    WebView *widg = new WebView(QUrl::fromUserInput(args->arg(0)));
    widg->show();
    args->clear();

    return app.exec();
}

