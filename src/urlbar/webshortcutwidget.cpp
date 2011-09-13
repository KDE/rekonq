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

// Self Includes
#include "webshortcutwidget.h"
#include "rekonq_defines.h"

// KDE Includes
#include <KGlobalSettings>
#include <KIcon>
#include <KLocale>
#include <KServiceTypeTrader>

// Qt Includes
#include <QSet>
#include <QLabel>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QFormLayout>

WebShortcutWidget::WebShortcutWidget(QWidget *parent)
    : QMenu(parent)
    , m_wsLineEdit(new QLineEdit(this))
    , m_nameLineEdit(new QLineEdit(this))
    , m_noteLabel(new QLabel(this))
{
    setAttribute(Qt::WA_DeleteOnClose);
    setFixedWidth(350);

    QFormLayout *layout = new QFormLayout(this);
    QVBoxLayout *vLay = new QVBoxLayout(this);

    // Web Search Icon
    QLabel *webSearchIcon = new QLabel(this);
    webSearchIcon->setPixmap(KIcon("edit-web-search").pixmap(32, 32));

    // Title
    QLabel *titleLabel = new QLabel(this);
    titleLabel->setText("<h4>" + i18n("Add Search Engine") + "</h4>");
    vLay->addWidget(titleLabel);

    // Name
    vLay->addWidget(m_nameLineEdit);

    layout->addRow(webSearchIcon, vLay);

    // Shortcuts
    QLabel *shortcutsLabel = new QLabel(i18n("Shortcuts:"), this);
    layout->addRow(shortcutsLabel, m_wsLineEdit);
    connect(m_wsLineEdit,  SIGNAL(textChanged(QString)), SLOT(shortcutsChanged(const QString&)));

    // Note
    m_noteLabel->setWordWrap(true);
    layout->addRow(m_noteLabel);
    m_noteLabel->hide();

    // Ok & Cancel buttons
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(close()));
    layout->addWidget(buttonBox);

    setLayout(layout);

    m_providers = KServiceTypeTrader::self()->query("SearchProvider");

    m_wsLineEdit->setFocus();
}


void WebShortcutWidget::showAt(const QPoint &pos)
{
    adjustSize();

    QPoint p(pos.x() - width(), pos.y() + 10);
    move(p);
    exec();
}


void WebShortcutWidget::show(const KUrl &url, const QString &openSearchName, const QPoint &pos)
{
    m_wsLineEdit->clear();
    m_nameLineEdit->setText(openSearchName);
    m_url = url;
    showAt(pos);
}


void WebShortcutWidget::accept()
{
    emit webShortcutSet(m_url, m_nameLineEdit->text(), m_wsLineEdit->text());
    close();
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
        m_noteLabel->setText(i18n("The shortcut \"%1\" is already assigned to \"%2\".", contenderWS, contenderName));
        m_noteLabel->setVisible(true);
        resize(minimumSize().width(), minimumSizeHint().height() + 15);
    }
    else
    {
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


