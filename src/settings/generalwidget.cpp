/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2010 by Andrea Diamantini <adjam7 at gmail dot com>
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


// Self Includes
#include "generalwidget.h"
#include "generalwidget.moc"

// Auto Includes
#include "rekonq.h"

// Local Includes
#include "application.h"
#include "mainwindow.h"
#include "webtab.h"


GeneralWidget::GeneralWidget(QWidget *parent)
        : QWidget(parent)
        , _changed(false)
{
    setupUi(this);

    connect(setHomeToCurrentPageButton, SIGNAL(clicked()), this, SLOT(setHomeToCurrentPage()));

    disableHomeSettings(ReKonfig::useNewTabPage());
    
    connect(kcfg_useNewTabPage, SIGNAL(toggled(bool)), this, SLOT(disableHomeSettings(bool)));
}


void GeneralWidget::save()
{
}


bool GeneralWidget::changed()
{
    return _changed;
}


void GeneralWidget::hasChanged()
{
}


void GeneralWidget::setHomeToCurrentPage()
{
    MainWindow *mw = qobject_cast<MainWindow*>(parent());
    WebTab *webTab = mw->currentTab();
    if (webTab)
    {
        kcfg_homePage->setText(webTab->url().prettyUrl());
    }
}


void GeneralWidget::disableHomeSettings(bool b)
{
    kcfg_homePage->setEnabled(!b);
    setHomeToCurrentPageButton->setEnabled(!b);
}
