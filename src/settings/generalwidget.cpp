/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2010-2012 by Andrea Diamantini <adjam7 at gmail dot com>
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
#include "rekonqwindow.h"
#include "webwindow.h"

//KDE Includes
#include <kstandarddirs.h>
#include <KUrlRequester>


GeneralWidget::GeneralWidget(QWidget *parent)
    : QWidget(parent)
    , _changed(false)
{
    setupUi(this);

    connect(setHomeToCurrentPageButton, SIGNAL(clicked()), this, SLOT(setHomeToCurrentPage()));

    disableHomeSettings(ReKonfig::useNewTabPage());

    connect(kcfg_useNewTabPage, SIGNAL(toggled(bool)), this, SLOT(disableHomeSettings(bool)));

    checkKGetPresence();

    connect(kcfg_homePage, SIGNAL(editingFinished()), this, SLOT(fixHomePageURL()));

    kcfg_downloadPath->setMode(KFile::Directory);

    askDownloadYes->setChecked(ReKonfig::askDownloadPath());
    askDownloadNo->setChecked(!ReKonfig::askDownloadPath());

    kcfg_downloadPath->setEnabled(!ReKonfig::askDownloadPath());
    connect(askDownloadNo, SIGNAL(toggled(bool)), kcfg_downloadPath, SLOT(setEnabled(bool)));
    connect(askDownloadNo, SIGNAL(toggled(bool)), this, SLOT(hasChanged()));
}


void GeneralWidget::save()
{
    ReKonfig::setAskDownloadPath(askDownloadYes->isChecked());

    _changed = false;
}


bool GeneralWidget::changed()
{
    return _changed;
}


void GeneralWidget::hasChanged()
{
    _changed = true;
    emit changed(true);
}


void GeneralWidget::setHomeToCurrentPage()
{
    WebWindow *tab = rApp->rekonqWindow()->currentWebWindow();
    if (tab)
    {
        kcfg_homePage->setText(tab->url().url());
    }
}


void GeneralWidget::disableHomeSettings(bool b)
{
    kcfg_homePage->setEnabled(!b);
    setHomeToCurrentPageButton->setEnabled(!b);
}


void GeneralWidget::checkKGetPresence()
{
    if (KStandardDirs::findExe("kget").isNull())
    {
        kcfg_kgetDownload->setDisabled(true);
        kcfg_kgetList->setDisabled(true);
        kcfg_kgetDownload->setToolTip(i18n("Install KGet to enable rekonq to use it as download manager"));
    }
    else
    {
        kcfg_kgetDownload->setDisabled(false);
        kcfg_kgetList->setDisabled(false);
    }
}


void GeneralWidget::fixHomePageURL()
{
    QString fixedURL = QUrl::fromUserInput(kcfg_homePage->text()).toString();
    kcfg_homePage->setText(fixedURL);
}
