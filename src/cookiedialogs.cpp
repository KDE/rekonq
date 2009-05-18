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
#include "cookiedialogs.h"
#include "cookiedialogs.moc"

// Ui Includes
#include "ui_cookies.h"

// Local Includes
#include "cookiejar.h"

// Qt Includes
#include <QCompleter>


CookiesDialog::CookiesDialog(CookieJar *cookieJar, QWidget *parent)
        : KDialog(parent)
{
    Ui::CookiesWidget cookieWidget;
    QWidget widget;
    cookieWidget.setupUi(&widget);
    setMainWidget(&widget);

    setWindowFlags(Qt::Sheet);

    CookieModel *model = new CookieModel(cookieJar, this);
    m_proxyModel = new QSortFilterProxyModel(this);

    // connecting signals and slots
    connect(cookieWidget.search, SIGNAL(textChanged(QString)), m_proxyModel, SLOT(setFilterFixedString(QString)));
    connect(cookieWidget.removeButton, SIGNAL(clicked()), cookieWidget.cookiesTable, SLOT(removeOne()));
    connect(cookieWidget.removeAllButton, SIGNAL(clicked()), cookieWidget.cookiesTable, SLOT(removeAll()));

    m_proxyModel->setSourceModel(model);

    cookieWidget.cookiesTable->verticalHeader()->hide();
    cookieWidget.cookiesTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    cookieWidget.cookiesTable->setModel(m_proxyModel);
    cookieWidget.cookiesTable->setAlternatingRowColors(true);
    cookieWidget.cookiesTable->setTextElideMode(Qt::ElideMiddle);
    cookieWidget.cookiesTable->setShowGrid(false);
    cookieWidget.cookiesTable->setSortingEnabled(true);

    QFont f = font();
    f.setPointSize(10);
    QFontMetrics fm(f);
    int height = fm.height() + fm.height() / 3;
    cookieWidget.cookiesTable->verticalHeader()->setDefaultSectionSize(height);
    cookieWidget.cookiesTable->verticalHeader()->setMinimumSectionSize(-1);

    for (int i = 0; i < model->columnCount(); ++i)
    {
        int header = cookieWidget.cookiesTable->horizontalHeader()->sectionSizeHint(i);
        switch (i)
        {
        case 0:
            header = fm.width(QLatin1String("averagehost.domain.com"));
            break;
        case 1:
            header = fm.width(QLatin1String("_session_id"));
            break;
        case 4:
            header = fm.width(QDateTime::currentDateTime().toString(Qt::LocalDate));
            break;
        }
        int buffer = fm.width(QLatin1String("xx"));
        header += buffer;
        cookieWidget.cookiesTable->horizontalHeader()->resizeSection(i, header);
    }
    cookieWidget.cookiesTable->horizontalHeader()->setStretchLastSection(true);
}


// ----------------------------------------------------------------------------------------------------------------


CookiesExceptionsDialog::CookiesExceptionsDialog(CookieJar *cookieJar, QWidget *parent)
        : KDialog(parent)
        , m_cookieJar(cookieJar)
        , exceptionsWidget(new Ui::CookiesExceptionsWidget)
{
    QWidget widget;
    exceptionsWidget->setupUi(&widget);
    setMainWidget(&widget);

    setWindowFlags(Qt::Sheet);

    connect(exceptionsWidget->removeButton, SIGNAL(clicked()), exceptionsWidget->exceptionTable, SLOT(removeOne()));
    connect(exceptionsWidget->removeAllButton, SIGNAL(clicked()), exceptionsWidget->exceptionTable, SLOT(removeAll()));

    exceptionsWidget->exceptionTable->verticalHeader()->hide();
    exceptionsWidget->exceptionTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    exceptionsWidget->exceptionTable->setAlternatingRowColors(true);
    exceptionsWidget->exceptionTable->setTextElideMode(Qt::ElideMiddle);
    exceptionsWidget->exceptionTable->setShowGrid(false);
    exceptionsWidget->exceptionTable->setSortingEnabled(true);
    m_exceptionsModel = new CookieExceptionsModel(cookieJar, this);
    m_proxyModel = new QSortFilterProxyModel(this);
    m_proxyModel->setSourceModel(m_exceptionsModel);

    connect(exceptionsWidget->search, SIGNAL(textChanged(QString)), m_proxyModel, SLOT(setFilterFixedString(QString)));

    exceptionsWidget->exceptionTable->setModel(m_proxyModel);

    CookieModel *cookieModel = new CookieModel(cookieJar, this);
    exceptionsWidget->domainLineEdit->setCompleter(new QCompleter(cookieModel, exceptionsWidget->domainLineEdit));

    connect(exceptionsWidget->domainLineEdit, SIGNAL(textChanged(const QString &)), this, SLOT(textChanged(const QString &)));
    connect(exceptionsWidget->blockButton, SIGNAL(clicked()), this, SLOT(block()));
    connect(exceptionsWidget->allowButton, SIGNAL(clicked()), this, SLOT(allow()));
    connect(exceptionsWidget->allowForSessionButton, SIGNAL(clicked()), this, SLOT(allowForSession()));

    QFont f = font();
    f.setPointSize(10);
    QFontMetrics fm(f);
    int height = fm.height() + fm.height() / 3;
    exceptionsWidget->exceptionTable->verticalHeader()->setDefaultSectionSize(height);
    exceptionsWidget->exceptionTable->verticalHeader()->setMinimumSectionSize(-1);
    for (int i = 0; i < m_exceptionsModel->columnCount(); ++i)
    {
        int header = exceptionsWidget->exceptionTable->horizontalHeader()->sectionSizeHint(i);
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
        exceptionsWidget->exceptionTable->horizontalHeader()->resizeSection(i, header);
    }
}


void CookiesExceptionsDialog::textChanged(const QString &text)
{
    bool enabled = !text.isEmpty();
    exceptionsWidget->blockButton->setEnabled(enabled);
    exceptionsWidget->allowButton->setEnabled(enabled);
    exceptionsWidget->allowForSessionButton->setEnabled(enabled);
}


void CookiesExceptionsDialog::block()
{
    if (exceptionsWidget->domainLineEdit->text().isEmpty())
        return;
    m_exceptionsModel->m_blockedCookies.append(exceptionsWidget->domainLineEdit->text());
    m_cookieJar->setBlockedCookies(m_exceptionsModel->m_blockedCookies);
    m_exceptionsModel->reset();
}


void CookiesExceptionsDialog::allow()
{
    if (exceptionsWidget->domainLineEdit->text().isEmpty())
        return;
    m_exceptionsModel->m_allowedCookies.append(exceptionsWidget->domainLineEdit->text());
    m_cookieJar->setAllowedCookies(m_exceptionsModel->m_allowedCookies);
    m_exceptionsModel->reset();
}


void CookiesExceptionsDialog::allowForSession()
{
    if (exceptionsWidget->domainLineEdit->text().isEmpty())
        return;
    m_exceptionsModel->m_sessionCookies.append(exceptionsWidget->domainLineEdit->text());
    m_cookieJar->setAllowForSessionCookies(m_exceptionsModel->m_sessionCookies);
    m_exceptionsModel->reset();
}
