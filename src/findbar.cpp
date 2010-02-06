/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2008-2009 by Andrea Diamantini <adjam7 at gmail dot com>
* Copyright (C) 2009 by Lionel Chauvin <megabigbug@yahoo.fr>
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
#include "findbar.h"
#include "findbar.moc"

// KDE Includes
#include <KLineEdit>
#include <KIcon>
#include <KPushButton>
#include <klocalizedstring.h>
#include <KMainWindow>
#include <KApplication>

// Qt Includes
#include <QtGui/QWidget>
#include <QtGui/QHBoxLayout>
#include <QtGui/QToolButton>
#include <QtGui/QLabel>
#include <QtGui/QColor>
#include <QtGui/QKeyEvent>
#include <QtCore/QString>
#include <QtCore/QTimer>


FindBar::FindBar(KMainWindow *mainwindow)
        : QWidget(mainwindow)
        , m_lineEdit(new KLineEdit(this))
        , m_matchCase(new QCheckBox(i18n("&Match case"), this))
        , m_hideTimer(new QTimer(this))
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

    // hide timer 
    connect(m_hideTimer, SIGNAL(timeout()), this, SLOT(hide()));

    // label
    QLabel *label = new QLabel(i18n("Find:"));
    layout->addWidget(label);

    // lineEdit, focusProxy
    setFocusProxy(m_lineEdit);
    m_lineEdit->setMaximumWidth(250);
    connect(m_lineEdit, SIGNAL(textChanged(const QString &)), mainwindow, SLOT(find(const QString &)));
    layout->addWidget(m_lineEdit);

    // buttons
    KPushButton *findNext = new KPushButton(KIcon("go-down"), i18n("&Next"), this);
    KPushButton *findPrev = new KPushButton(KIcon("go-up"), i18n("&Previous"), this);
    connect(findNext, SIGNAL(clicked()), mainwindow, SLOT(findNext()));
    connect(findPrev, SIGNAL(clicked()), mainwindow, SLOT(findPrevious()));
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


void FindBar::show()
{
    // set focus to findbar if user select showFindBar shortcut
    m_lineEdit->setFocus();
    m_lineEdit->selectAll();

    // show findbar if not visible
    if (isVisible())
        return;

    QWidget::show();
    m_hideTimer->start(60000);
}


void FindBar::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Return && !m_lineEdit->text().isEmpty())
    {
        emit searchString(m_lineEdit->text());
        return;
    }

    QWidget::keyPressEvent(event);
}


void FindBar::notifyMatch(bool match)
{
    QPalette p = m_lineEdit->palette();

    if (m_lineEdit->text().isEmpty())
    {
        p.setColor(QPalette::Base, QColor(KApplication::palette().color(QPalette::Active, QPalette::Base)));
    }
    else 
    {
        if (match)
        {
            p.setColor(QPalette::Base, QColor(186, 249, 206));
        }
        else
        {
            p.setColor(QPalette::Base, QColor(247, 130, 130)); // previous were 247, 230, 230
        }
    }
    m_lineEdit->setPalette(p);
    m_hideTimer->start(60000);
}



void FindBar::hide()
{
    m_hideTimer->stop();
    QWidget::hide();
}
