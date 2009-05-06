/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2008-2009 by Andrea Diamantini <adjam7 at gmail dot com>
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
#include "findbar.h"
#include "findbar.moc"

// KDE Includes
#include <KLineEdit>
#include <KAction>
#include <KIcon>
#include <KPushButton>
#include <klocalizedstring.h>
#include <KXmlGuiWindow>

// Qt Includes
#include <QtGui/QWidget>
#include <QtGui/QHBoxLayout>
#include <QtGui/QToolButton>
#include <QtGui/QLabel>
#include <QtGui/QKeyEvent>
#include <QtCore/QString>

FindBar::FindBar(KXmlGuiWindow *mainwindow)
        : QWidget(mainwindow)
        , m_lineEdit(new KLineEdit(this))
        , m_matchCase(new QCheckBox(i18n("&Match case"), this))
{
    QHBoxLayout *layout = new QHBoxLayout;

    // cosmetic
    layout->setContentsMargins(2, 0, 2, 0);

    // hide button
    QToolButton *hideButton = new QToolButton(this);
    hideButton->setAutoRaise(true);
    hideButton->setIcon(KIcon("dialog-close"));
    connect(hideButton, SIGNAL(clicked()), this, SLOT(hide()));
    layout->addWidget(hideButton);
    layout->setAlignment(hideButton, Qt::AlignLeft | Qt::AlignTop);

    // label
    QLabel *label = new QLabel(i18n("Find: "));
    layout->addWidget(label);

    // lineEdit, focusProxy
    setFocusProxy(m_lineEdit);
    m_lineEdit->setMaximumWidth(250);
    connect(m_lineEdit, SIGNAL(textChanged(const QString &)), mainwindow, SLOT(slotFind(const QString &)));
    layout->addWidget(m_lineEdit);

    // buttons
    KPushButton *findNext = new KPushButton(KIcon("go-down"), i18n("&Next"), this);
    KPushButton *findPrev = new KPushButton(KIcon("go-up"), i18n("&Previous"), this);
    connect(findNext, SIGNAL(clicked()), mainwindow, SLOT(slotFindNext()));
    connect(findPrev, SIGNAL(clicked()), mainwindow, SLOT(slotFindPrevious()));
    layout->addWidget(findNext);
    layout->addWidget(findPrev);

    // Case sensitivity. Deliberately set so this is off by default.
    m_matchCase->setCheckState(Qt::Unchecked);
    m_matchCase->setTristate(false);
    layout->addWidget(m_matchCase);

    // stretching widget on the left
    layout->addStretch();

    setLayout(layout);

    // we start off hidden
    hide();
}


FindBar::~FindBar()
{
}


KLineEdit *FindBar::lineEdit() const
{
    return m_lineEdit;
}


bool FindBar::matchCase() const
{
    return m_matchCase->isChecked();
}


void FindBar::clear()
{
    m_lineEdit->setText(QString());
}


void FindBar::showFindBar()
{
    // show findbar if not visible
    if (!isVisible())
    {
        show();
    }
    // set focus to findbar if user select showFindBar shortcut
    m_lineEdit->setFocus();
    m_lineEdit->selectAll();
}


void FindBar::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Escape)
    {
        hide();
        return;
    }
    if (event->key() == Qt::Key_Return && !m_lineEdit->text().isEmpty())
    {
        emit searchString(m_lineEdit->text());
        return;
    }
    QWidget::keyPressEvent(event);
}

