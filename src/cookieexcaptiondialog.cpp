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
#include "cookieexceptiondialog.h"
#include "cookieexceptiondialog.moc"

// Local Includes



CookieExceptionsModel::CookieExceptionsModel(CookieJar *cookiejar, QObject *parent)
        : QAbstractTableModel(parent)
        , m_cookieJar(cookiejar)
{
    m_allowedCookies = m_cookieJar->allowedCookies();
    m_blockedCookies = m_cookieJar->blockedCookies();
    m_sessionCookies = m_cookieJar->allowForSessionCookies();
}


QVariant CookieExceptionsModel::headerData(int section, Qt::Orientation orientation, int role) const
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

    if (orientation == Qt::Horizontal
            && role == Qt::DisplayRole)
    {
        switch (section)
        {
        case 0:
            return i18n("Website");
        case 1:
            return i18n("Status");
        }
    }
    return QAbstractTableModel::headerData(section, orientation, role);
}


QVariant CookieExceptionsModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() >= rowCount())
        return QVariant();

    switch (role)
    {
    case Qt::DisplayRole:
    case Qt::EditRole:
    {
        int row = index.row();
        if (row < m_allowedCookies.count())
        {
            switch (index.column())
            {
            case 0:
                return m_allowedCookies.at(row);
            case 1:
                return i18n("Allow");
            }
        }
        row = row - m_allowedCookies.count();
        if (row < m_blockedCookies.count())
        {
            switch (index.column())
            {
            case 0:
                return m_blockedCookies.at(row);
            case 1:
                return i18n("Block");
            }
        }
        row = row - m_blockedCookies.count();
        if (row < m_sessionCookies.count())
        {
            switch (index.column())
            {
            case 0:
                return m_sessionCookies.at(row);
            case 1:
                return i18n("Allow For Session");
            }
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


int CookieExceptionsModel::columnCount(const QModelIndex &parent) const
{
    return (parent.isValid()) ? 0 : 2;
}


int CookieExceptionsModel::rowCount(const QModelIndex &parent) const
{
    return (parent.isValid() || !m_cookieJar) ? 0 : m_allowedCookies.count() + m_blockedCookies.count() + m_sessionCookies.count();
}


bool CookieExceptionsModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if (parent.isValid() || !m_cookieJar)
        return false;

    int lastRow = row + count - 1;
    beginRemoveRows(parent, row, lastRow);
    for (int i = lastRow; i >= row; --i)
    {
        if (i < m_allowedCookies.count())
        {
            m_allowedCookies.removeAt(row);
            continue;
        }
        i = i - m_allowedCookies.count();
        if (i < m_blockedCookies.count())
        {
            m_blockedCookies.removeAt(row);
            continue;
        }
        i = i - m_blockedCookies.count();
        if (i < m_sessionCookies.count())
        {
            m_sessionCookies.removeAt(row);
            continue;
        }
    }
    m_cookieJar->setAllowedCookies(m_allowedCookies);
    m_cookieJar->setBlockedCookies(m_blockedCookies);
    m_cookieJar->setAllowForSessionCookies(m_sessionCookies);
    endRemoveRows();
    return true;
}




// ----------------------------------------------------------------------------------------------------------------


CookiesExceptionsDialog::CookiesExceptionsDialog(CookieJar *cookieJar, QWidget *parent)
        : KDialog(parent)
        , m_cookieJar(cookieJar)
        , m_exceptionsWidget(new Ui::CookiesExceptionsWidget)
{
    setCaption("Cookies Exceptions");
    setButtons( KDialog::Ok );

    QWidget *widget = new QWidget(this);
    m_exceptionsWidget->setupUi(widget);
    setMainWidget(widget);

    setWindowFlags(Qt::Sheet);

    connect(m_exceptionsWidget->removeButton, SIGNAL(clicked()), m_exceptionsWidget->exceptionTable, SLOT(removeOne()));
    connect(m_exceptionsWidget->removeAllButton, SIGNAL(clicked()), m_exceptionsWidget->exceptionTable, SLOT(removeAll()));

    m_exceptionsWidget->exceptionTable->verticalHeader()->hide();
    m_exceptionsWidget->exceptionTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_exceptionsWidget->exceptionTable->setAlternatingRowColors(true);
    m_exceptionsWidget->exceptionTable->setTextElideMode(Qt::ElideMiddle);
    m_exceptionsWidget->exceptionTable->setShowGrid(false);
    m_exceptionsWidget->exceptionTable->setSortingEnabled(true);
    m_exceptionsModel = new CookieExceptionsModel(cookieJar, this);
    m_proxyModel = new QSortFilterProxyModel(this);
    m_proxyModel->setSourceModel(m_exceptionsModel);

    connect(m_exceptionsWidget->search, SIGNAL(textChanged(QString)), m_proxyModel, SLOT(setFilterFixedString(QString)));

    m_exceptionsWidget->exceptionTable->setModel(m_proxyModel);

    connect(m_exceptionsWidget->domainLineEdit, SIGNAL(textChanged(const QString &)), this, SLOT(textChanged(const QString &)));
    connect(m_exceptionsWidget->blockButton, SIGNAL(clicked()), this, SLOT(block()));
    connect(m_exceptionsWidget->allowButton, SIGNAL(clicked()), this, SLOT(allow()));
    connect(m_exceptionsWidget->allowForSessionButton, SIGNAL(clicked()), this, SLOT(allowForSession()));

    QFont f = font();
    f.setPointSize(10);
    QFontMetrics fm(f);
    int height = fm.height() + fm.height() / 3;
    m_exceptionsWidget->exceptionTable->verticalHeader()->setDefaultSectionSize(height);
    m_exceptionsWidget->exceptionTable->verticalHeader()->setMinimumSectionSize(-1);
    for (int i = 0; i < m_exceptionsModel->columnCount(); ++i)
    {
        int header = m_exceptionsWidget->exceptionTable->horizontalHeader()->sectionSizeHint(i);
        switch (i)
        {
        case 0:
            header = fm.width(QLatin1String("averagebiglonghost.domain.com"));
            break;
        case 1:
            header = fm.width(QLatin1String("Allow For Session"));
            break;
        }
        int buffer = fm.width(QLatin1String("xx"));
        header += buffer;
        m_exceptionsWidget->exceptionTable->horizontalHeader()->resizeSection(i, header);
    }
}


void CookiesExceptionsDialog::textChanged(const QString &text)
{
    bool enabled = !text.isEmpty();
    m_exceptionsWidget->blockButton->setEnabled(enabled);
    m_exceptionsWidget->allowButton->setEnabled(enabled);
    m_exceptionsWidget->allowForSessionButton->setEnabled(enabled);
}


void CookiesExceptionsDialog::block()
{
    if (m_exceptionsWidget->domainLineEdit->text().isEmpty())
        return;
    m_exceptionsModel->m_blockedCookies.append(m_exceptionsWidget->domainLineEdit->text());
    m_cookieJar->setBlockedCookies(m_exceptionsModel->m_blockedCookies);
    m_exceptionsModel->reset();
}


void CookiesExceptionsDialog::allow()
{
    if (m_exceptionsWidget->domainLineEdit->text().isEmpty())
        return;
    m_exceptionsModel->m_allowedCookies.append(m_exceptionsWidget->domainLineEdit->text());
    m_cookieJar->setAllowedCookies(m_exceptionsModel->m_allowedCookies);
    m_exceptionsModel->reset();
}


void CookiesExceptionsDialog::allowForSession()
{
    if (m_exceptionsWidget->domainLineEdit->text().isEmpty())
        return;
    m_exceptionsModel->m_sessionCookies.append(m_exceptionsWidget->domainLineEdit->text());
    m_cookieJar->setAllowForSessionCookies(m_exceptionsModel->m_sessionCookies);
    m_exceptionsModel->reset();
}

