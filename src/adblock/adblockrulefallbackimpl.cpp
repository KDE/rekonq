/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2010 by Benjamin Poulain <ikipou at gmail dot com>
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
#include "adblockrulefallbackimpl.h"

// Rekonq Includes
#include "rekonq_defines.h"

// Qt Includes
#include <QStringList>

static inline bool isRegExpFilter(const QString &filter)
{
    return filter.startsWith(QL1C('/')) && filter.endsWith(QL1C('/'));
}

AdBlockRuleFallbackImpl::AdBlockRuleFallbackImpl(const QString &filter)
    : AdBlockRuleImpl(filter)
{
    m_regExp.setCaseSensitivity(Qt::CaseInsensitive);
    m_regExp.setPatternSyntax(QRegExp::RegExp2);

    QString parsedLine = filter;

    const int optionsNumber = parsedLine.lastIndexOf(QL1C('$'));
    if (optionsNumber >= 0 && !isRegExpFilter(parsedLine)) {
        const QStringList options(parsedLine.mid(optionsNumber + 1).split(QL1C(',')));
        if (options.contains(QL1S("match-case")))
            m_regExp.setCaseSensitivity(Qt::CaseSensitive);
        parsedLine = parsedLine.left(optionsNumber);
    }

    if (isRegExpFilter(parsedLine))
        parsedLine = parsedLine.mid(1, parsedLine.length() - 2);
    else
        parsedLine = convertPatternToRegExp(parsedLine);

    m_regExp.setPattern(parsedLine);
}

bool AdBlockRuleFallbackImpl::match(const QString &encodedUrl) const
{
    return m_regExp.indexIn(encodedUrl) != -1;
}

QString AdBlockRuleFallbackImpl::convertPatternToRegExp(const QString &wildcardPattern)
{
    QString pattern = wildcardPattern;

    // remove multiple wildcards
    pattern.replace(QRegExp(QL1S("\\*+")), QL1S("*"));

    // remove anchors following separator placeholder
    pattern.replace(QRegExp(QL1S("\\^\\|$")), QL1S("^"));

    // remove leading wildcards
    pattern.replace(QRegExp(QL1S("^(\\*)")), QL1S(""));

    // remove trailing wildcards
    pattern.replace(QRegExp(QL1S("(\\*)$")), QL1S(""));

    // escape special symbols
    pattern.replace(QRegExp(QL1S("(\\W)")), QL1S("\\\\1"));

    // process extended anchor at expression start
    pattern.replace(QRegExp(QL1S("^\\\\\\|\\\\\\|")), QL1S("^[\\w\\-]+:\\/+(?!\\/)(?:[^\\/]+\\.)?"));

    // process separator placeholders
    pattern.replace(QRegExp(QL1S("\\\\\\^")), QL1S("(?:[^\\w\\d\\-.%]|$)"));

    // process anchor at expression start
    pattern.replace(QRegExp(QL1S("^\\\\\\|")), QL1S("^"));

    // process anchor at expression end
    pattern.replace(QRegExp(QL1S("\\\\\\|$")), QL1S("$"));

    // replace wildcards by .*
    pattern.replace(QRegExp(QL1S("\\\\\\*")), QL1S(".*"));

    // Finally, return...
    return pattern;
}
