/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2010 by Yoann Laissus <yoann dot laissus at gmail dot com>
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


// Auto Includes
#include "bookmarkwidget.h"
#include "bookmarkwidget.moc"

// Local includes
#include "application.h"
#include "bookmarkprovider.h"
#include "bookmarkowner.h"

// KDE Includes
#include <KLocalizedString>
#include <KIcon>
#include <KLineEdit>

// Qt Includes
#include <QtGui/QDialogButtonBox>
#include <QtGui/QFormLayout>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>


BookmarkWidget::BookmarkWidget(const KBookmark &bookmark, QWidget *parent)
        : QMenu(parent)
        , m_bookmark(new KBookmark(bookmark))
{
    setAttribute(Qt::WA_DeleteOnClose);
    setFixedWidth(350);

    QFormLayout *layout = new QFormLayout(this);

    // Bookmark icon
    QHBoxLayout *hLayout = new QHBoxLayout(this);
    QLabel *bookmarkIcon = new QLabel(this);
    bookmarkIcon->setPixmap(KIcon("bookmarks").pixmap(32, 32));
    hLayout->setSpacing(10);
    hLayout->addWidget(bookmarkIcon);

    // Title
    QVBoxLayout *vLayout = new QVBoxLayout(this);
    QLabel *bookmarkInfo = new QLabel(this);
    bookmarkInfo->setText(i18n("<h4>Edit this Bookmark</h4>"));
    bookmarkInfo->setAlignment(Qt::AlignCenter);
    vLayout->addWidget(bookmarkInfo);

    // Remove button
    QPushButton *removeButton = new QPushButton(this);
    removeButton->setText(i18n("Remove this Bookmark"));
    connect(removeButton, SIGNAL(clicked()), this, SLOT(removeBookmark()));
    vLayout->addWidget(removeButton);

    hLayout->addLayout(vLayout);
    layout->addItem(hLayout);

    // Bookmark name
    QLabel *nameLabel = new QLabel(this);
    nameLabel->setText(i18n("Name:"));
    m_name = new KLineEdit(this);
    if (m_bookmark->isNull())
    {
        m_name->setEnabled(false);
    }
    else
    {
        m_name->setText(m_bookmark->text());
        m_name->setFocus();
    }
    layout->addRow(nameLabel, m_name);

    // Ok & Cancel buttons
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    layout->addWidget(buttonBox);

    setLayout(layout);
}


BookmarkWidget::~BookmarkWidget()
{
    delete m_bookmark;
}


void BookmarkWidget::showAt(const QPoint &pos)
{
    adjustSize();

    QPoint p(pos.x()-width(), pos.y()+10);
    move(p);
    show();
}


void BookmarkWidget::accept()
{
    if (!m_bookmark->isNull() && m_name->text() != m_bookmark->fullText())
    {
        m_bookmark->setFullText(m_name->text());
        Application::bookmarkProvider()->bookmarkManager()->emitChanged();
    }
    close();
}


void BookmarkWidget::removeBookmark()
{
    Application::bookmarkProvider()->bookmarkOwner()->deleteBookmark(*m_bookmark);
    close();
}
