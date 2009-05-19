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



#ifndef COOKIEDIALOG_H
#define COOKIEDIALOG_H


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


class CookieModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit CookieModel(CookieJar *jar, QObject *parent = 0);

    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());

private slots:
    void cookiesChanged();

private:
    CookieJar *m_cookieJar;
};



// -----------------------------------------------------------------------------------------------------------------


class CookiesDialog : public KDialog
{
    Q_OBJECT

public:
    explicit CookiesDialog(CookieJar *cookieJar, QWidget *parent = 0);

private:
    QSortFilterProxyModel *m_proxyModel;
};

#endif
