/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2011-2014 by Andrea Diamantini <adjam7 at gmail dot com>
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

// Local Includes
#include "useragentinfo.h"
#include "useragentwidget.h"

#include "webwindow.h"
#include "webpage.h"

// KDE Includes
#include <KAction>
#include <KDialog>
#include <KMenu>


QPointer<UserAgentManager> UserAgentManager::s_userAgentManager;


UserAgentManager *UserAgentManager::self()
{
    if (s_userAgentManager.isNull())
    {
        s_userAgentManager = new UserAgentManager(qApp);
    }
    return s_userAgentManager.data();
}


// ----------------------------------------------------------------------------------------------


UserAgentManager::UserAgentManager(QObject *parent)
    : QObject(parent)
    , m_uaSettingsAction(0)
{
    m_uaSettingsAction = new KAction(KIcon("preferences-web-browser-identification"), i18n("Browser Identification"), this);
    connect(m_uaSettingsAction, SIGNAL(triggered(bool)), this, SLOT(showSettings()));
}


void UserAgentManager::showSettings()
{
    QPointer<KDialog> dialog = new KDialog(m_uaTab.data());
    dialog->setCaption(i18nc("@title:window", "User Agent Settings"));
    dialog->setButtons(KDialog::Ok);

    UserAgentWidget widget;
    dialog->setMainWidget(&widget);
    dialog->exec();

    dialog->deleteLater();
}


void UserAgentManager::populateUAMenuForTabUrl(KMenu *uaMenu, WebWindow *uaTab)
{
    if (!m_uaTab.isNull())
    {
        m_uaTab.clear();
    }

    m_uaTab = uaTab;

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

    // Main Browsers Menus
    KMenu *ffMenu = new KMenu(i18n("Firefox"), uaMenu);
    uaMenu->addMenu(ffMenu);

    KMenu *ieMenu = new KMenu(i18n("Internet Explorer"), uaMenu);
    uaMenu->addMenu(ieMenu);

    KMenu *nsMenu = new KMenu(i18n("Netscape"), uaMenu);
    uaMenu->addMenu(nsMenu);

    KMenu *opMenu = new KMenu(i18n("Opera"), uaMenu);
    uaMenu->addMenu(opMenu);

    KMenu *sfMenu = new KMenu(i18n("Safari"), uaMenu);
    uaMenu->addMenu(sfMenu);

    KMenu *otMenu = new KMenu(i18n("Other"), uaMenu);
    uaMenu->addMenu(otMenu);

    UserAgentInfo uaInfo;
    QStringList UAlist = uaInfo.availableUserAgents();
    const KService::List providers = uaInfo.availableProviders();
    int uaIndex = uaInfo.uaIndexForHost(m_uaTab.data()->url().host());

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

        QString tag = providers.at(i)->property("X-KDE-UA-TAG").toString();
        if (tag == QL1S("FF"))
        {
            ffMenu->addAction(a);
        }
        else if (tag == QL1S("IE"))
        {
            ieMenu->addAction(a);
        }
        else if (tag == QL1S("NN"))
        {
            nsMenu->addAction(a);
        }
        else if (tag == QL1S("OPR"))
        {
            opMenu->addAction(a);
        }
        else if (tag == QL1S("SAF"))
        {
            sfMenu->addAction(a);
        }
        else    // OTHERs
        {
            otMenu->addAction(a);
        }
    }
    defaultAction->setChecked(defaultUA);

    uaMenu->addSeparator();
    uaMenu->addAction(m_uaSettingsAction);
}


void UserAgentManager::setUserAgent()
{
    QAction *sender = static_cast<QAction *>(QObject::sender());

    int uaIndex = sender->data().toInt();

    UserAgentInfo uaInfo;
    uaInfo.setUserAgentForHost(uaIndex, m_uaTab.data()->url().host());

    // reload tab
    m_uaTab.data()->page()->triggerAction(QWebPage::Reload);
}
