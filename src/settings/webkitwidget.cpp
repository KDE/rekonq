/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2010-2011 by Andrea Diamantini <adjam7 at gmail dot com>
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


// Local Includes
#include "webkitwidget.h"
#include "webkitwidget.moc"


WebKitWidget::WebKitWidget(QWidget *parent)
    : QWidget(parent)
    , _changed(false)
{
    setupUi(this);
}


void WebKitWidget::save()
{
}


bool WebKitWidget::changed()
{
    return _changed;
}


void WebKitWidget::hasChanged()
{
}


void WebKitWidget::setWebSettingsToolTips()
{
    kcfg_autoLoadImages->setToolTip(i18n("Specifies whether images are automatically loaded in web pages."));
    kcfg_dnsPrefetch->setToolTip(i18n("Specifies whether WebKit will try to prefetch DNS entries to speed up browsing."));
    kcfg_javascriptEnabled->setToolTip(i18n("Enables the execution of JavaScript programs."));
    kcfg_javaEnabled->setToolTip(i18n("Enables support for Java applets."));
    kcfg_pluginsEnabled->setToolTip(i18n("Enables support for plugins in web pages."));
    kcfg_javascriptCanOpenWindows->setToolTip(i18n("If enabled, JavaScript programs are allowed to open new windows."));
    kcfg_javascriptCanAccessClipboard->setToolTip(i18n("If enabled, JavaScript programs are allowed to read from and to write to the clipboard."));
    kcfg_linksIncludedInFocusChain->setToolTip(i18n("If enabled, hyperlinks are included in the keyboard focus chain."));
    kcfg_zoomTextOnly->setToolTip(i18n("If enabled, the zoom factor on a frame is only applied to the text."));
    kcfg_printElementBackgrounds->setToolTip(i18n("If enabled, background colors and images are also drawn when the page is printed."));
    kcfg_offlineStorageDatabaseEnabled->setToolTip(i18n("Enables support for the HTML 5 offline storage feature."));
    kcfg_offlineWebApplicationCacheEnabled->setToolTip(i18n("Enables support for the HTML 5 web application cache feature."));
    kcfg_localStorageEnabled->setToolTip(i18n("Enables support for the HTML 5 local storage feature."));
}
