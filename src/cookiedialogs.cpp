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
    setCaption("Cookies");
    setButtons( KDialog::Ok );

    Ui::CookiesWidget *cookieWidget = new Ui::CookiesWidget;
    QWidget *widget = new QWidget(this);
    cookieWidget->setupUi(widget);
    setMainWidget(widget);

    setWindowFlags(Qt::Sheet);

    CookieModel *model = new CookieModel(cookieJar, this);
    m_proxyModel = new QSortFilterProxyModel(this);

    // connecting signals and slots
    connect(cookieWidget->search, SIGNAL(textChanged(QString)), m_proxyModel, SLOT(setFilterFixedString(QString)));
    connect(cookieWidget->removeButton, SIGNAL(clicked()), cookieWidget->cookiesTable, SLOT(removeOne()));
    connect(cookieWidget->removeAllButton, SIGNAL(clicked()), cookieWidget->cookiesTable, SLOT(removeAll()));

    m_proxyModel->setSourceModel(model);

    cookieWidget->cookiesTable->verticalHeader()->hide();
    cookieWidget->cookiesTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    cookieWidget->cookiesTable->setModel(m_proxyModel);
    cookieWidget->cookiesTable->setAlternatingRowColors(true);
    cookieWidget->cookiesTable->setTextElideMode(Qt::ElideMiddle);
    cookieWidget->cookiesTable->setShowGrid(false);
    cookieWidget->cookiesTable->setSortingEnabled(true);

    QFont f = font();
    f.setPointSize(10);
    QFontMetrics fm(f);
    int height = fm.height() + fm.height() / 3;
    cookieWidget->cookiesTable->verticalHeader()->setDefaultSectionSize(height);
    cookieWidget->cookiesTable->verticalHeader()->setMinimumSectionSize(-1);

    for (int i = 0; i < model->columnCount(); ++i)
    {
        int header = cookieWidget->cookiesTable->horizontalHeader()->sectionSizeHint(i);
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
        cookieWidget->cookiesTable->horizontalHeader()->resizeSection(i, header);
    }
    cookieWidget->cookiesTable->horizontalHeader()->setStretchLastSection(true);
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

//     CookieModel *cookieModel = new CookieModel(cookieJar, this);
//     m_exceptionsWidget->domainLineEdit->setCompleter(new QCompleter(cookieModel, m_exceptionsWidget->domainLineEdit));

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
