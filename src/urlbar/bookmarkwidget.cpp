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
#include "bookmarksmanager.h"
#include "bookmarkowner.h"

// KDE Includes
#include <KLocalizedString>
#include <KLineEdit>
#include <KMessageBox>

// Qt Includes
#include <QtGui/QFormLayout>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>



BookmarkWidget::BookmarkWidget(const KBookmark &bookmark, QWidget *parent)
        : QFrame(parent, Qt::Popup)
        , m_bookmark(bookmark)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setFixedWidth(350);
    setFrameStyle(QFrame::Panel);

    QFormLayout *layout = new QFormLayout(this);
    setLayout(layout);

    QHBoxLayout *hLayout = new QHBoxLayout();

    QLabel *bookmarkIcon = new QLabel(this);
    bookmarkIcon->setPixmap(KIcon("bookmarks").pixmap(32, 32));
    hLayout->addWidget(bookmarkIcon);
    hLayout->setSpacing(10);

    QVBoxLayout *vLayout = new QVBoxLayout();

    QLabel *bookmarkInfo = new QLabel(this);
    bookmarkInfo->setText(i18n("Edit this Bookmark"));
    QFont font;
    font.setPointSize(font.pointSize() + 2);
    bookmarkInfo->setFont(font);

    vLayout->addWidget(bookmarkInfo);

    QPushButton *removeButton = new QPushButton(this);
    removeButton->setText(i18n("Remove this Bookmark"));
    connect(removeButton, SIGNAL(clicked()), this, SLOT(removeBookmark()));

    vLayout->addWidget(removeButton);
    hLayout->addLayout(vLayout);
    layout->addItem(hLayout);


    QLabel *nameLabel = new QLabel(this);
    nameLabel->setText(i18n("Name:"));

    m_name = new KLineEdit(this);
    if (m_bookmark.isNull())
    {
        m_name->setEnabled(false);
    }
    else
    {
        m_name->setText(m_bookmark.text());
        m_name->setFocus();
    }

    layout->addRow(nameLabel, m_name);

    QLabel *urlLabel = new QLabel(this);
    urlLabel->setText("URL:");

    KLineEdit *url = new KLineEdit(this);
    url->setText(m_bookmark.url().url());
    url->setEnabled(false);

    layout->addRow(urlLabel, url);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
    buttonBox->button(QDialogButtonBox::Ok)->setText(i18n("Done"));
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    layout->addWidget(buttonBox);
}


BookmarkWidget::~BookmarkWidget()
{
    delete m_name;
}


void BookmarkWidget::accept()
{
    if (!m_bookmark.isNull() && m_name->text() != m_bookmark.fullText())
    {
        m_bookmark.setFullText(m_name->text());
        Application::bookmarkProvider()->bookmarkManager()->emitChanged();
    }
    reject();
}


void BookmarkWidget::reject()
{
    close();
    deleteLater();
}


void BookmarkWidget::showAt(const QPoint &pos)
{
    QPoint p;
    p.setX(pos.x());
    p.setY(pos.y() + 12);
    move(p);
    show();
}


void BookmarkWidget::removeBookmark()
{
    Application::bookmarkProvider()->bookmarkOwner()->deleteBookmark(m_bookmark);
    reject();
}
