/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2010-2011 by Pierre Rossi <pierre dot rossi at gmail dot com>
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
#include "messagebar.h"
#include "messagebar.moc"

// KDE Includes
#include <KIcon>
#include <KIconLoader>
#include <KLocalizedString>

// Qt Includes
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QToolButton>


MessageBar::MessageBar(const QString &message, QWidget *parent, QMessageBox::Icon icon, StandardButtons buttons)
        : NotificationBar(parent)
        , m_icon(0)
        , m_text(0)
{
    QToolButton *closeButton = new QToolButton(this);
    closeButton->setAutoRaise(true);
    closeButton->setIcon(KIcon("dialog-close"));
    connect(closeButton, SIGNAL(clicked()), this, SLOT(destroy()));

    m_text = new QLabel(message, this);
    m_text->setWordWrap(true);

    m_icon = new QLabel;
    QString icon_name;
    switch (icon)
    {
    case QMessageBox::NoIcon:
        break;
    case QMessageBox::Information:
        icon_name = "dialog-information";
        break;
    case QMessageBox::Warning:
        icon_name = "dialog-warning";
        break;
    case QMessageBox::Critical:
        icon_name = "dialog-error";
        break;
    default:
        break;
    }
    if (!icon_name.isEmpty())
        m_icon->setPixmap(KIcon(icon_name).pixmap(int(KIconLoader::SizeSmallMedium)));

    QPushButton *button;
    if (buttons & Ok)
    {
        button = new QPushButton(KIcon("dialog-ok"), i18n("Ok"));
        connect(button, SIGNAL(clicked()), this, SIGNAL(accepted()));
        connect(button, SIGNAL(clicked()), this, SLOT(destroy()));
        m_buttons.append(button);
    }
    if (buttons & Cancel)
    {
        button = new QPushButton(KIcon("dialog-cancel"), i18n("Cancel"));
        connect(button, SIGNAL(clicked()), this, SIGNAL(rejected()));
        connect(button, SIGNAL(clicked()), this, SLOT(destroy()));
        m_buttons.append(button);
    }
    if (buttons & Yes)
    {
        button = new QPushButton(i18n("Yes"));
        connect(button, SIGNAL(clicked()), this, SIGNAL(accepted()));
        connect(button, SIGNAL(clicked()), this, SLOT(destroy()));
        m_buttons.append(button);
    }
    if (buttons & No)
    {
        button = new QPushButton(i18n("No"));
        connect(button, SIGNAL(clicked()), this, SIGNAL(rejected()));
        connect(button, SIGNAL(clicked()), this, SLOT(destroy()));
        m_buttons.append(button);
    }
    if (buttons & Continue)
    {
        button = new QPushButton(i18n("Continue"));
        connect(button, SIGNAL(clicked()), this, SIGNAL(accepted()));
        connect(button, SIGNAL(clicked()), this, SLOT(destroy()));
        m_buttons.append(button);
    }

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(2, 0, 2, 0);
    layout->addWidget(closeButton);
    layout->addWidget(m_icon);
    layout->addWidget(m_text);
    foreach(QPushButton *button, m_buttons)
    layout->addWidget(button, 2);
    layout->setStretch(2, 20);

    setLayout(layout);

}

MessageBar::~MessageBar()
{
    qDeleteAll(m_buttons);
}
