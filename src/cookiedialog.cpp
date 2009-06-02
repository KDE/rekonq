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


// Self Includes
#include "cookiedialog.h"
#include "cookiedialog.moc"

// Local Includes
#include "cookiejar.h"

// KDE Includes
#include <KLocalizedString>


CookieModel::CookieModel(CookieJar *cookieJar, QObject *parent)
        : QAbstractTableModel(parent)
        , m_cookieJar(cookieJar)
{
    connect(m_cookieJar, SIGNAL(cookiesChanged()), this, SLOT(cookiesChanged()));
}


QVariant CookieModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::SizeHintRole)
    {
        QFont font;
        font.setPointSize(10);
        QFontMetrics fm(font);
        int height = fm.height() + fm.height() / 3;
        int width = fm.width(headerData(section, orientation, Qt::DisplayRole).toString());
        return QSize(width, height);
    }

    if (orientation == Qt::Horizontal)
    {
        if (role != Qt::DisplayRole)
            return QVariant();

        switch (section)
        {
        case 0:
            return i18n("Website");
        case 1:
            return i18n("Name");
        case 2:
            return i18n("Path");
        case 3:
            return i18n("Secure");
        case 4:
            return i18n("Expires");
        case 5:
            return i18n("Contents");
        default:
            return QVariant();
        }
    }
    return QAbstractTableModel::headerData(section, orientation, role);
}


QVariant CookieModel::data(const QModelIndex &index, int role) const
{
    QList<QNetworkCookie> lst;
    if (m_cookieJar)
        lst = m_cookieJar->allCookies();
    if (index.row() < 0 || index.row() >= lst.size())
        return QVariant();

    switch (role)
    {
    case Qt::DisplayRole:
    case Qt::EditRole:
    {
        QNetworkCookie cookie = lst.at(index.row());
        switch (index.column())
        {
        case 0:
            return cookie.domain();
        case 1:
            return cookie.name();
        case 2:
            return cookie.path();
        case 3:
            return cookie.isSecure();
        case 4:
            return cookie.expirationDate();
        case 5:
            return cookie.value();
        }
    }
    case Qt::FontRole:
    {
        QFont font;
        font.setPointSize(10);
        return font;
    }
    }

    return QVariant();
}


int CookieModel::columnCount(const QModelIndex &parent) const
{
    return (parent.isValid()) ? 0 : 6;
}


int CookieModel::rowCount(const QModelIndex &parent) const
{
    return (parent.isValid() || !m_cookieJar) ? 0 : m_cookieJar->allCookies().count();
}


bool CookieModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if (parent.isValid() || !m_cookieJar)
        return false;
    int lastRow = row + count - 1;
    beginRemoveRows(parent, row, lastRow);
    QList<QNetworkCookie> lst = m_cookieJar->allCookies();
    for (int i = lastRow; i >= row; --i)
    {
        lst.removeAt(i);
    }
    m_cookieJar->setAllCookies(lst);
    endRemoveRows();
    return true;
}


void CookieModel::cookiesChanged()
{
    reset();
}


// ---------------------------------------------------------------------------------------


// Ui Includes
#include "ui_cookies.h"

// Qt Includes
#include <QtCore/QRect>
#include <QtCore/QSize>

#include <QtGui/QDesktopWidget>


CookiesDialog::CookiesDialog(CookieJar *cookieJar, QWidget *parent)
        : KDialog(parent)
{
    setWindowFlags(Qt::Sheet);
    setCaption("Cookies");
    setButtons( KDialog::Close );

    Ui::CookiesWidget *cookieWidget = new Ui::CookiesWidget;
    QWidget *widget = new QWidget(this);
    cookieWidget->setupUi(widget);
    setMainWidget(widget);

    CookieModel *model = new CookieModel(cookieJar, this);
    m_proxyModel = new QSortFilterProxyModel(this);

    // connecting signals and slots
    connect(cookieWidget->search, SIGNAL(textChanged(QString)), m_proxyModel, SLOT(setFilterFixedString(QString)));
    connect(cookieWidget->removeButton, SIGNAL(clicked()), cookieWidget->cookiesTable, SLOT(removeOne()));
    connect(cookieWidget->removeAllButton, SIGNAL(clicked()), cookieWidget->cookiesTable, SLOT(removeAll()));

    m_proxyModel->setSourceModel(model);

    cookieWidget->cookiesTable->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    cookieWidget->cookiesTable->verticalHeader()->hide();
    cookieWidget->cookiesTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    cookieWidget->cookiesTable->setModel(m_proxyModel);
    cookieWidget->cookiesTable->setAlternatingRowColors(true);
    cookieWidget->cookiesTable->setTextElideMode(Qt::ElideMiddle);
    cookieWidget->cookiesTable->setShowGrid(false);
    cookieWidget->cookiesTable->setSortingEnabled(true);

    // Fixing header dimension
    QHeaderView *headerView = cookieWidget->cookiesTable->horizontalHeader();
    headerView->setResizeMode(QHeaderView::Stretch);
}


QSize CookiesDialog::sizeHint() const
{
    QRect desktopRect = QApplication::desktop()->screenGeometry();
    QSize size = desktopRect.size() * 0.8;
    return size;
}
