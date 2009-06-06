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
* either version 3, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* ============================================================ */


#ifndef COOKIEEXCEPTIONDIALOG_H
#define COOKIEEXCEPTIONDIALOG_H


// Local Includes
#include "cookiejar.h"

// Qt Includes
#include <QtCore/QAbstractTableModel>

// Forward Declarations
class QString;
class QStringList;
class QModelIndex;
class QVariant;

class CookieExceptionsModel : public QAbstractTableModel
{
    Q_OBJECT
    friend class CookiesExceptionsDialog;

public:
    explicit CookieExceptionsModel(CookieJar *cookieJar, QObject *parent = 0);

    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());

private:
    CookieJar *m_cookieJar;

    // Domains we allow, Domains we block, Domains we allow for this session
    QStringList m_allowedCookies;
    QStringList m_blockedCookies;
    QStringList m_sessionCookies;
};


// -----------------------------------------------------------------------------------------------


// Ui Includes
#include "ui_cookiesexceptions.h"

//Forward Declarations
class QSortFilterProxyModel;


class CookiesExceptionsDialog : public KDialog
{
    Q_OBJECT

public:
    explicit CookiesExceptionsDialog(CookieJar *cookieJar, QWidget *parent = 0);

    QSize sizeHint() const;

private slots:
    void block();
    void allow();
    void allowForSession();
    void textChanged(const QString &text);

private:
    CookieExceptionsModel *m_exceptionsModel;
    QSortFilterProxyModel *m_proxyModel;
    CookieJar *m_cookieJar;

    Ui::CookiesExceptionsWidget *m_exceptionsWidget;
};

#endif
