/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (c) 2001 by Dawit Alemayehu <adawit@kde.org>
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



#ifndef USER_AGENT_INFO_H
#define USER_AGENT_INFO_H


// Rekonq Includes
#include "rekonq_defines.h"

// KDE Includes
#include <KService>

// Qt Includes
#include <QString>


class UserAgentInfo
{
public:
    UserAgentInfo();

    /**
     * Lists all available providers
     *
     */
    KService::List availableProviders() const;

    /**
     * Lists all available User Agents
     *
     * @returns the list of the UA descriptions
     */
    QStringList availableUserAgents();

    /**
     * Set User Agent for host
     *
     * @param uaIndex   the index of the UA description. @see availableUserAgents()
     * @param host      the host to se the UA
     */
    bool setUserAgentForHost(int uaIndex, const QString &host);

    /**
     * @returns the index of the UA set for the @p host
     */
    int uaIndexForHost(const QString &);

private:
    QString userAgentString(int);
    QString userAgentName(int);
    QString userAgentVersion(int);
    QString userAgentDescription(int);

    bool providerExists(int);

private:
    KService::List m_providers;
};

#endif // USER_AGENT_INFO_H
