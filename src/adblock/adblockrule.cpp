/*
 * Copyright (c) 2009, Zsombor Gegesy <gzsombor@gmail.com>
 * Copyright (c) 2009, Benjamin C. Meyer <ben@meyerhome.net>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Benjamin Meyer nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * ============================================================
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
#include "adblockrule.h"

// Qt Includes
#include <QStringList>
#include <QUrl>


AdBlockRule::AdBlockRule(const QString &filter)
{
    bool isRegExpRule = false;

    QString parsedLine = filter;

    if (parsedLine.startsWith(QL1C('/')) && parsedLine.endsWith(QL1C('/')))
    {
        parsedLine = parsedLine.mid(1);
        parsedLine = parsedLine.left(parsedLine.size() - 1);
        isRegExpRule = true;
    }

    int optionsNumber = parsedLine.indexOf(QL1C('$'), 0);
    QStringList options;

    if (optionsNumber >= 0)
    {
        options = parsedLine.mid(optionsNumber + 1).split(QL1C(','));
        parsedLine = parsedLine.left(optionsNumber);
    }

    if (!isRegExpRule)
        parsedLine = convertPatternToRegExp(parsedLine);

    m_regExp = QRegExp(parsedLine, Qt::CaseInsensitive, QRegExp::RegExp2);

    if (options.contains(QL1S("match-case")))
    {
        m_regExp.setCaseSensitivity(Qt::CaseSensitive);
    }
}


// here return false means that rule doesn't match,
// so that url is allowed
// return true means "matched rule", so stop url!
bool AdBlockRule::match(const QString &encodedUrl) const
{
    return m_regExp.indexIn(encodedUrl) != -1;
}


QString AdBlockRule::convertPatternToRegExp(const QString &wildcardPattern)
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


QString AdBlockRule::pattern() const
{
    return m_regExp.pattern();
}
