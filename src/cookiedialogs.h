/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2009 by Andrea Diamantini <adjam7 at gmail dot com>
*
*
* This program is free software; you can redistribute it
* and/or modify it under the terms of the GNU General
* Public License as published by the Free Software Foundation;
* either version 2, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* ============================================================ */



#ifndef COOKIEDIALOGS_H
#define COOKIEDIALOGS_H


// KDE Includes
#include <KDialog>

// Qt Includes
#include <QtCore/QStringList>
#include <QtCore/QAbstractItemModel>

#include <QtGui/QTableView>
#include <QtGui/QSortFilterProxyModel>

#include <QtNetwork/QNetworkCookieJar>

// Forward Declarations
class CookieJar;
class CookieExceptionsModel;


class CookiesDialog : public KDialog
{
    Q_OBJECT

public:
    explicit CookiesDialog(CookieJar *cookieJar, QWidget *parent = 0);

private:
    QSortFilterProxyModel *m_proxyModel;
};


// -----------------------------------------------------------------------------------------------------------------


// Ui Includes
#include "ui_cookiesexceptions.h"


class CookiesExceptionsDialog : public KDialog
{
    Q_OBJECT

public:
    explicit CookiesExceptionsDialog(CookieJar *cookieJar, QWidget *parent = 0);

private slots:
    void block();
    void allow();
    void allowForSession();
    void textChanged(const QString &text);

private:
    CookieExceptionsModel *m_exceptionsModel;
    QSortFilterProxyModel *m_proxyModel;
    CookieJar *m_cookieJar;

    Ui::CookiesExceptionsWidget *exceptionsWidget;
};


#endif
