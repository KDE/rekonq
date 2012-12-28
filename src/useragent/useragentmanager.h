/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2011-2012 by Andrea Diamantini <adjam7 at gmail dot com>
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


#ifndef USER_AGENT_MANAGER_H
#define USER_AGENT_MANAGER_H


// Qt Includes
#include <QObject>
#include <QWeakPointer>

// Forward Declarations
class WebWindow;

class KAction;
class KMenu;


class UserAgentManager : public QObject
{
    Q_OBJECT

public:
    /**
     * Entry point.
     * Access to UserAgentManager class by using
     * UserAgentManager::self()->thePublicMethodYouNeed()
     */
    static UserAgentManager *self();

    void populateUAMenuForTabUrl(KMenu *, WebWindow *);

private:
    explicit UserAgentManager(QObject *parent = 0);

private Q_SLOTS:
    void showSettings();
    void setUserAgent();

private:
    KAction *m_uaSettingsAction;
    QWeakPointer<WebWindow> m_uaTab;

    static QWeakPointer<UserAgentManager> s_userAgentManager;
};

#endif  // USER_AGENT_MANAGER_H
