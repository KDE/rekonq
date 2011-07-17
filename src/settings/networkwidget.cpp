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


// Self Includes
#include "networkwidget.h"
#include "networkwidget.moc"

// KDE Includes
#include <KTabWidget>
#include <KCModuleInfo>

// Qt Includes
#include <QVBoxLayout>


NetworkWidget::NetworkWidget(QWidget *parent)
    : QWidget(parent)
    , _cacheModule(0)
    , _cookiesModule(0)
    , _proxyModule(0)
    , _changed(false)
{
    QVBoxLayout *l = new QVBoxLayout(this);
    l->setMargin(0);
    l->setSpacing(0);

    KTabWidget *tabWidget = new KTabWidget(this);
    l->addWidget(tabWidget);

    KCModuleInfo cacheInfo("cache.desktop");
    _cacheModule = new KCModuleProxy(cacheInfo, parent);
    tabWidget->addTab(_cacheModule, i18n(cacheInfo.moduleName().toUtf8()));

    KCModuleInfo cookiesInfo("cookies.desktop");
    _cookiesModule = new KCModuleProxy(cookiesInfo, parent);
    tabWidget->addTab(_cookiesModule, i18n(cookiesInfo.moduleName().toUtf8()));

    KCModuleInfo proxyInfo("proxy.desktop");
    _proxyModule = new KCModuleProxy(proxyInfo, parent);
    tabWidget->addTab(_proxyModule, i18n(proxyInfo.moduleName().toUtf8()));

    connect(_cacheModule,   SIGNAL(changed(bool)), this, SLOT(hasChanged()));
    connect(_cookiesModule, SIGNAL(changed(bool)), this, SLOT(hasChanged()));
    connect(_proxyModule,   SIGNAL(changed(bool)), this, SLOT(hasChanged()));
}


NetworkWidget::~NetworkWidget()
{
    delete _cacheModule;
    delete _cookiesModule;
    delete _proxyModule;
}


void NetworkWidget::save()
{
    _cookiesModule->save();
    _proxyModule->save();
    _cacheModule->save();

    _changed = false;
    emit changed(false);
}


void NetworkWidget::hasChanged()
{
    _changed = true;
    emit changed(true);
}


bool NetworkWidget::changed()
{
    return _changed;
}
