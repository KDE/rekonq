/* This file is part of the KDE project
 *
 * Copyright (C) 2009 by Fredy Yanardi <fyanardi@gmail.com>
 * Copyright (C) 2010-2011 by  Lionel Chauvin <megabigbug@yahoo.fr>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "webshortcutwidget.h"
#include "rekonq_defines.h"

#include <QtCore/QTimer>
#include <QtCore/QSet>
#include <QtGui/QBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>
#include <QtGui/QFormLayout>


#include <KGlobalSettings>
#include <KIcon>
#include <KLocale>
#include <KServiceTypeTrader>

WebShortcutWidget::WebShortcutWidget(QWidget *parent)
    : QDialog(parent)
{
    QVBoxLayout *mainLayout = new QVBoxLayout();
    QHBoxLayout *titleLayout = new QHBoxLayout();
    mainLayout->addLayout(titleLayout);
    QLabel *iconLabel = new QLabel(this);

    KIcon wsIcon("edit-web-search");
    if (wsIcon.isNull())
    {
        wsIcon = KIcon("preferences-web-browser-shortcuts");
    }

    iconLabel->setPixmap(wsIcon.pixmap(22, 22));
    titleLayout->addWidget(iconLabel);
    m_searchTitleLabel = new QLabel(i18n("Add Search Engine"), this);
    QFont boldFont = KGlobalSettings::generalFont();
    boldFont.setBold(true);
    m_searchTitleLabel->setFont(boldFont);
    titleLayout->addWidget(m_searchTitleLabel);
    titleLayout->addStretch();

    QFormLayout *formLayout = new QFormLayout();
    mainLayout->addLayout(formLayout);

    QFont smallFont = KGlobalSettings::smallestReadableFont();
    m_nameLineEdit = new QLineEdit(this);
    m_nameLineEdit->setEnabled(false);
    m_nameLineEdit->setFont(smallFont);
    QLabel *nameLabel = new QLabel(i18n("Name:"), this);
    nameLabel->setFont(smallFont);
    formLayout->addRow(nameLabel, m_nameLineEdit);

    QLabel *shortcutsLabel = new QLabel(i18n("Shortcuts:"), this);
    shortcutsLabel->setFont(smallFont);
    m_wsLineEdit = new QLineEdit(this);
    m_wsLineEdit->setMinimumWidth(100);
    m_wsLineEdit->setFont(smallFont);
    formLayout->addRow(shortcutsLabel, m_wsLineEdit);
    connect(m_wsLineEdit,  SIGNAL(textChanged(QString)), SLOT(shortcutsChanged(const QString&)));

    m_noteLabel = new QLabel(this);
    m_noteLabel->setFont(boldFont);
    m_noteLabel->setWordWrap(true);
    formLayout->addRow(m_noteLabel);
    m_noteLabel->setVisible(false);

    mainLayout->addStretch();

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    mainLayout->addLayout(buttonLayout);
    buttonLayout->addStretch();
    m_okButton = new QPushButton(i18n("Ok"), this);
    m_okButton->setDefault(true);
    buttonLayout->addWidget(m_okButton);
    connect(m_okButton, SIGNAL(clicked()), this, SLOT(okClicked()));

    QPushButton *cancelButton = new QPushButton(i18n("Cancel"), this);
    buttonLayout->addWidget(cancelButton);
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(cancelClicked()));

    setLayout(mainLayout);

    setMinimumWidth(250);

    m_providers = KServiceTypeTrader::self()->query("SearchProvider");

    QTimer::singleShot(0, m_wsLineEdit, SLOT(setFocus()));
}


void WebShortcutWidget::showAt(const QPoint &pos)
{
    adjustSize();

    QPoint p;
    p.setX(pos.x() - width());
    p.setY(pos.y() + 10);

    move(p);
    QDialog::show();
}


void WebShortcutWidget::show(const KUrl &url, const QString &openSearchName, const QPoint &pos)
{
    m_wsLineEdit->clear();
    m_nameLineEdit->setText(openSearchName);
    m_url = url;
    showAt(pos);
}


void WebShortcutWidget::okClicked()
{
    hide();
    emit webShortcutSet(m_url, m_nameLineEdit->text(), m_wsLineEdit->text());
}


void WebShortcutWidget::cancelClicked()
{
    hide();
}


void WebShortcutWidget::shortcutsChanged(const QString& newShorthands)
{
    int savedCursorPosition = m_wsLineEdit->cursorPosition();
    QString normalizedShorthands = QString(newShorthands).replace(QL1C(' '), QL1C(','));
    m_wsLineEdit->setText(normalizedShorthands);
    m_wsLineEdit->setCursorPosition(savedCursorPosition);

    QSet<QString> shorthands = normalizedShorthands.split(QL1C(',')).toSet();
    QString contenderName = "";
    QString contenderWS = "";

    Q_FOREACH(const QString & shorthand, shorthands)
    {
        Q_FOREACH(KService::Ptr provider, m_providers)
        {
            if (provider->property("Keys").toStringList().contains(shorthand))
            {
                contenderName = provider->property("Name").toString();
                contenderWS = shorthand;
                break;
            }
        }
    }

    if (!contenderName.isEmpty())
    {
        m_okButton->setEnabled(false);
        m_noteLabel->setText(i18n("The shortcut \"%1\" is already assigned to \"%2\".", contenderWS, contenderName));
        m_noteLabel->setVisible(true);
        resize(minimumSize().width(), minimumSizeHint().height() + 15);
    }
    else
    {
        m_okButton->setEnabled(true);
        m_noteLabel->clear();
        bool noteIsVisible = m_noteLabel->isVisible();
        m_noteLabel->setVisible(false);
        if (noteIsVisible)
        {
            resize(minimumSize());
        }
    }
}

#include "webshortcutwidget.moc"


