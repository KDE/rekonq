/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2010-2011 by Yoann Laissus <yoann dot laissus at gmail dot com>
* Copyright (C) 2012 by Andrea Diamantini <adjam7 at gmail dot com>
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
#include "bookmarkmanager.h"
#include "bookmarkowner.h"

// KDE Includes
#include <KComboBox>
#include <KLocalizedString>
#include <KIcon>
#include <KLineEdit>

// Qt Includes
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QPushButton>


BookmarkWidget::BookmarkWidget(const KBookmark &bookmark, QWidget *parent)
    : QMenu(parent)
    , m_bookmark(new KBookmark(bookmark))
{
    setAttribute(Qt::WA_DeleteOnClose);
    setFixedWidth(350);

    QFormLayout *layout = new QFormLayout(this);

    // Bookmark icon
    QLabel *bookmarkIcon = new QLabel(this);
    bookmarkIcon->setPixmap(KIcon("bookmarks").pixmap(32, 32));

    // Title
    QVBoxLayout *vLayout = new QVBoxLayout;
    QLabel *bookmarkInfo = new QLabel(this);
    bookmarkInfo->setText(i18n("Edit this Bookmark"));
    QFont f = bookmarkInfo->font();
    f.setBold(true);
    bookmarkInfo->setFont(f);
    vLayout->addWidget(bookmarkInfo);

    // Remove button
    QPushButton *removeButton = new QPushButton(this);
    removeButton->setText(i18n("Remove this Bookmark"));
    connect(removeButton, SIGNAL(clicked()), this, SLOT(removeBookmark()));
    vLayout->addWidget(removeButton);

    layout->addRow(bookmarkIcon, vLayout);

    //Bookmark Folder
    QLabel *folderLabel = new QLabel(this);
    folderLabel->setText(i18n("Folder:"));

    m_folder = new KComboBox(this);
    layout->addRow(folderLabel, m_folder);
    setupFolderComboBox();

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
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(close()));
    layout->addWidget(buttonBox);
}


BookmarkWidget::~BookmarkWidget()
{
    delete m_bookmark;
}


void BookmarkWidget::showAt(const QPoint &pos)
{
    adjustSize();

    QPoint p(pos.x() - width(), pos.y() + 10);
    move(p);
    show();
}


void BookmarkWidget::accept()
{
    if (!m_bookmark->isNull() && m_name->text() != m_bookmark->fullText())
    {
        m_bookmark->setFullText(m_name->text());
        rApp->bookmarkManager()->emitChanged();
    }

    QString folderAddress = m_folder->itemData(m_folder->currentIndex()).toString();
    KBookmarkGroup a = rApp->bookmarkManager()->manager()->findByAddress(folderAddress).toGroup();

    KBookmarkGroup parent = m_bookmark->parentGroup();
    parent.deleteBookmark(*m_bookmark);
    a.addBookmark(*m_bookmark);
    rApp->bookmarkManager()->manager()->emitChanged(a);

    close();
}


void BookmarkWidget::setupFolderComboBox()
{
    KBookmarkGroup root = rApp->bookmarkManager()->manager()->toolbar();

    if (rApp->bookmarkManager()->manager()->toolbar().address() == rApp->bookmarkManager()->manager()->root().address())
    {
        m_folder->addItem(i18n("Bookmark Toolbar"),
                          rApp->bookmarkManager()->manager()->toolbar().address());
    }
    else
    {
        m_folder->addItem(rApp->bookmarkManager()->manager()->toolbar().text(),
                          rApp->bookmarkManager()->manager()->toolbar().address());
    }
    m_folder->insertSeparator(1);

    if (m_bookmark->parentGroup().address() != rApp->bookmarkManager()->manager()->toolbar().address())
    {
        m_folder->addItem(m_bookmark->parentGroup().text(),
                          m_bookmark->parentGroup().address());
        m_folder->insertSeparator(3);
    }

    for (KBookmark bookmark = root.first(); !bookmark.isNull(); bookmark = root.next(bookmark))
    {
        if (bookmark.isGroup())
        {
            m_folder->addItem(bookmark.text(), bookmark.address());
        }
    }

    if (m_bookmark->parentGroup().address() == root.address())
    {
        m_folder->setCurrentIndex(0);
    }
    else
    {
        int index = m_folder->findText(m_bookmark->parentGroup().text());
        m_folder->setCurrentIndex(index);
    }
}


void BookmarkWidget::removeBookmark()
{
    rApp->bookmarkManager()->owner()->deleteBookmark(*m_bookmark);
    close();

    emit updateIcon();
}
