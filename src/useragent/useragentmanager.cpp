/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2011 by Andrea Diamantini <adjam7 at gmail dot com>
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
#include "useragentmanager.h"
#include "useragentmanager.moc"

// Local Includes
#include "useragentinfo.h"
#include "useragentwidget.h"
#include "webtab.h"
#include "webview.h"

// KDE Includes
#include <KAction>
#include <KDialog>
#include <KMenu>


UserAgentManager::UserAgentManager(QObject *parent)
    : QObject(parent)
    , _uaSettingsAction(0)
    , _uaTab(0)
{
    _uaSettingsAction = new KAction(KIcon("preferences-web-browser-identification"), i18n("Browser Identification"), this);
    connect(_uaSettingsAction, SIGNAL(triggered(bool)), this, SLOT(showSettings()));
}


void UserAgentManager::showSettings()
{
    QPointer<KDialog> dialog = new KDialog(_uaTab);
    dialog->setCaption(i18nc("@title:window", "User Agent Settings"));
    dialog->setButtons(KDialog::Ok);

    UserAgentWidget widget;
    dialog->setMainWidget(&widget);
    dialog->exec();

    dialog->deleteLater();
}


void UserAgentManager::populateUAMenuForTabUrl(KMenu *uaMenu, WebTab *uaTab)
{
    if (_uaTab)
        disconnect(this, SIGNAL(reloadTab()), _uaTab->view(), SLOT(reload()));

    _uaTab = uaTab;
    connect(this, SIGNAL(reloadTab()), _uaTab->view(), SLOT(reload()));

    bool defaultUA = true;

    QAction *a, *defaultAction;

    // just to be sure...
    uaMenu->clear();

    defaultAction = new QAction(i18nc("Default rekonq user agent", "Default"), uaMenu);
    defaultAction->setData(-1);
    defaultAction->setCheckable(true);
    connect(defaultAction, SIGNAL(triggered(bool)), this, SLOT(setUserAgent()));

    uaMenu->addAction(defaultAction);
    uaMenu->addSeparator();

    UserAgentInfo uaInfo;
    QStringList UAlist = uaInfo.availableUserAgents();
    int uaIndex = uaInfo.uaIndexForHost(_uaTab->url().host());

    for (int i = 0; i < UAlist.count(); ++i)
    {
        QString uaDesc = UAlist.at(i);

        a = new QAction(uaDesc, uaMenu);
        a->setData(i);
        a->setCheckable(true);
        connect(a, SIGNAL(triggered(bool)), this, SLOT(setUserAgent()));

        if (i == uaIndex)
        {
            a->setChecked(true);
            defaultUA = false;
        }
        uaMenu->addAction(a);
    }
    defaultAction->setChecked(defaultUA);

    uaMenu->addSeparator();
    uaMenu->addAction(_uaSettingsAction);
}


void UserAgentManager::setUserAgent()
{
    QAction *sender = static_cast<QAction *>(QObject::sender());

    QString desc = sender->text();
    int uaIndex = sender->data().toInt();

    UserAgentInfo uaInfo;
    uaInfo.setUserAgentForHost(uaIndex, _uaTab->url().host());
    emit reloadTab();
}
