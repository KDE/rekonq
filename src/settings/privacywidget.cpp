/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2012 by Andrea Diamantini <adjam7 at gmail dot com>
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
#include "privacywidget.h"
#include "privacywidget.moc"

// Local Includes
#include "passexceptionswidget.h"

// Auto Includes
#include "rekonq.h"

// KDE Includes
#include <KDialog>
#include <KPushButton>

// Qt Includes
#include <QProcess>


PrivacyWidget::PrivacyWidget(QWidget *parent)
    : QWidget(parent)
    , _changed(false)
{
    setupUi(this);

    reload();
    
    // DO NOT TRACK
    KConfigGroup cg = KConfigGroup(KSharedConfig::openConfig("kioslaverc", KConfig::NoGlobals), QString());
    doNotTrackCheckBox->setChecked(cg.readEntry("DoNotTrack", false));
    connect(doNotTrackCheckBox, SIGNAL(clicked()), this, SLOT(hasChanged()));

    // CACHE & COOKIES
    connect(cacheButton, SIGNAL(clicked()), this, SLOT(launchCacheSettings()));
    connect(cookiesButton, SIGNAL(clicked()), this, SLOT(launchCookieSettings()));

    // PASSWORDS
    connect(managePassExceptionsButton, SIGNAL(clicked()), this, SLOT(showPassExceptions()));
}


void PrivacyWidget::save()
{
    KConfigGroup cg = KConfigGroup(KSharedConfig::openConfig("kioslaverc", KConfig::NoGlobals), QString());
    cg.writeEntry("DoNotTrack", doNotTrackCheckBox->isChecked());
    cg.sync();

    reload();
}


void PrivacyWidget::reload()
{
    bool b = ReKonfig::javascriptEnabled();
    
    kcfg_javascriptCanAccessClipboard->setEnabled(b);
    kcfg_javascriptCanOpenWindows->setEnabled(b);

    if (b)
    {
        kcfg_javascriptCanOpenWindows->setToolTip(i18n("If enabled, JavaScript programs are allowed to open new windows."));
        kcfg_javascriptCanAccessClipboard->setToolTip(i18n("If enabled, JavaScript programs are allowed to read from and to write to the clipboard."));
    }
    else
    {
        QString str = i18n("Javascript is NOT enabled, cannot change this settings");
        kcfg_javascriptCanOpenWindows->setToolTip(str);
        kcfg_javascriptCanAccessClipboard->setToolTip(str);
    }
}


bool PrivacyWidget::changed()
{
    return _changed;
}


void PrivacyWidget::hasChanged()
{
    _changed = true;
    emit changed(true);
}


void PrivacyWidget::launchCacheSettings()
{
    QString program = QL1S("kcmshell4");
    QStringList arguments;
    arguments << QL1S("cache");
    QProcess *proc = new QProcess(this);
    proc->start(program, arguments);
}


void PrivacyWidget::launchCookieSettings()
{
    QString program = QL1S("kcmshell4");
    QStringList arguments;
    arguments << QL1S("cookies");
    QProcess *proc = new QProcess(this);
    proc->start(program, arguments);
}


void PrivacyWidget::showPassExceptions()
{
    PassExWidget *widg = new PassExWidget;
    widg->show();
}
