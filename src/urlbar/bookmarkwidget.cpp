/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2010-2011 by Yoann Laissus <yoann dot laissus at gmail dot com>
* Copyright (C) 2012 by Andrea Diamantini <adjam7 at gmail dot com>
* Copyright (c) 2011-2012 by Phaneendra Hegde <pnh.pes@gmail.com>
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
#include <KRatingWidget>
#include <KBookmarkDialog>

// Qt Includes
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QPushButton>
#include <QCompleter>
#include <QTextCursor>
#include <QDBusConnection>
#include <QDBusConnectionInterface>

// Nepomuk config include
#include "config-nepomuk.h"

#ifdef HAVE_NEPOMUK
// Local Nepomuk Includes
#include "resourcelinkdialog.h"

//Nepomuk Includes
#include <Soprano/Vocabulary/NAO>
#endif



BookmarkWidget::BookmarkWidget(const KBookmark &bookmark, QWidget *parent)
    : QMenu(parent)
    , m_bookmark(new KBookmark(bookmark))
{
    setAttribute(Qt::WA_DeleteOnClose);
    setFixedWidth(320);

#ifdef HAVE_NEPOMUK
    m_nfoResource = (QUrl)m_bookmark->url();
    m_isNepomukEnabled = QDBusConnection::sessionBus().interface()->isServiceRegistered("org.kde.NepomukServer");
    kDebug() << "IS NEPOMUK ACTUALLY RUNNING? " << m_isNepomukEnabled;
#endif

    QFormLayout *layout = new QFormLayout(this);
    layout->setHorizontalSpacing(20);

    // Title
    QHBoxLayout *hLayout = new QHBoxLayout;
    QLabel *bookmarkInfo = new QLabel(this);
    bookmarkInfo->setText(i18n(" Bookmark"));
    QFont f = bookmarkInfo->font();
    f.setBold(true);
    bookmarkInfo->setFont(f);

    // Remove button
    QLabel *removeLabel = new QLabel(this);
    removeLabel->setText(i18n("<a href='Remove'>Remove</a>"));
    removeLabel->setAlignment(Qt::AlignRight);
    hLayout->addWidget(bookmarkInfo);
    hLayout->addWidget(removeLabel);
    layout->addRow(hLayout);

    connect(removeLabel, SIGNAL(linkActivated(QString)), this, SLOT(removeBookmark()));

    //Bookmark Folder
    QLabel *folderLabel = new QLabel(this);
    folderLabel->setText(i18n("Folder:"));

    m_folder = new KComboBox(this);
    layout->addRow(folderLabel, m_folder);
    setupFolderComboBox();
    connect(m_folder, SIGNAL(currentIndexChanged(int)), this, SLOT(onFolderIndexChanged(int)));

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

#ifdef HAVE_NEPOMUK

    if (m_isNepomukEnabled)
    {
        QLabel* rateLabel = new QLabel(this);
        rateLabel->setText(i18n("Rate:"));
        KRatingWidget *ratingWidget = new KRatingWidget(this);
        if (m_nfoResource.rating() != 0)
        {
            ratingWidget->setRating(m_nfoResource.rating());
        }
        connect(ratingWidget, SIGNAL(ratingChanged(int)), this, SLOT(setRatingSlot(int)));
        ratingWidget->setToolTip(i18n("Rate this page"));
        layout->addRow(rateLabel, ratingWidget);

        //Add comments
        QLabel *commentLabel = new QLabel(this);
        commentLabel->setText(i18n("Describe:"));
        commentLabel->setAlignment(Qt::AlignCenter);
        m_commentEdit = new QPlainTextEdit(this);
        if (!m_nfoResource.description().isEmpty())
        {
            m_commentEdit->setPlainText(m_nfoResource.description());
        }
        connect(m_commentEdit, SIGNAL(textChanged()), this, SLOT(addCommentSlot()));
        layout->addRow(commentLabel, m_commentEdit);

        // Create tags
        QLabel *tagLabel = new QLabel(this);
        tagLabel->setText(i18n("Tags:"));
        tagLabel->setAlignment(Qt::AlignLeft);
        m_tagLine = new KLineEdit(this);
        m_tagLine->setPlaceholderText(i18n("add tags(comma separated)"));


        QList<Nepomuk::Tag> tagList = Nepomuk::Tag::allTags();
        Q_FOREACH(Nepomuk::Tag t, tagList)
        {
            m_tList.append(t.label());
        }
        QCompleter *completeTag = new QCompleter(m_tList);
        completeTag->setCompletionMode(QCompleter::PopupCompletion);
        m_tagLine->setCompleter(completeTag);
        loadTags();

        layout->addRow(tagLabel, m_tagLine);

        QPushButton *linkToResource = new QPushButton(this);
        linkToResource->setText(i18n("Link Resources"));
        connect(linkToResource, SIGNAL(clicked()), this, SLOT(linkToResourceSlot()));
        layout->addWidget(linkToResource);
    }
    else
    {
        QLabel *nepomukLabel = new QLabel(this);
        QPalette p = nepomukLabel->palette();
        p.setColor(QPalette::WindowText, Qt::red);
        nepomukLabel->setPalette(p);
        nepomukLabel->setText(i18n("Nepomuk is actually disabled."));
        layout->addWidget(nepomukLabel);
    }
#endif

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

#ifdef HAVE_NEPOMUK
    if (m_isNepomukEnabled)
    {
        parseTags();
    }
#endif

    close();
}


void BookmarkWidget::setupFolderComboBox()
{
    KBookmarkGroup toolBarRoot = rApp->bookmarkManager()->manager()->toolbar();
    KBookmarkGroup root = rApp->bookmarkManager()->rootGroup();

    if (toolBarRoot.address() == root.address())
    {
        m_folder->addItem(KIcon("bookmark-toolbar"),
                          i18n("Bookmark Toolbar"),
                          toolBarRoot.address());
    }
    else
    {
        m_folder->addItem(KIcon("bookmark-toolbar"),
                          toolBarRoot.text(),
                          toolBarRoot.address());
    }
    m_folder->insertSeparator(1);

    if (m_bookmark->parentGroup().address() != toolBarRoot.address())
    {
        QString parentText = m_bookmark->parentGroup().text();

        if (m_bookmark->parentGroup().address() == root.address())
        {
            parentText = i18n("Root folder");
        }

        m_folder->addItem(parentText,
                          m_bookmark->parentGroup().address());
        m_folder->insertSeparator(3);
    }

    for (KBookmark bookmark = toolBarRoot.first(); !bookmark.isNull(); bookmark = toolBarRoot.next(bookmark))
    {
        if (bookmark.isGroup() && bookmark.address() != m_bookmark->parentGroup().address())
        {
            m_folder->addItem(bookmark.text(), bookmark.address());
        }
    }

    m_folder->insertSeparator(m_folder->count());
    m_folder->addItem(KIcon("folder"), i18n("Choose..."));

    int index =  m_folder->findData(m_bookmark->parentGroup().address());
    m_folder->setCurrentIndex(index);
}


void BookmarkWidget::onFolderIndexChanged(int index)
{
    if (index == m_folder->count() - 1)
    {
        KBookmarkDialog dialog(rApp->bookmarkManager()->manager());
        KBookmarkGroup selectedGroup = dialog.selectFolder(m_bookmark->parentGroup());

        if (selectedGroup.address() != m_bookmark->parentGroup().address() && !selectedGroup.isNull() )
        {
            m_bookmark->parentGroup().deleteBookmark(*m_bookmark);
            selectedGroup.addBookmark(*m_bookmark);
            rApp->bookmarkManager()->manager()->emitChanged();
        }
    }
}


void BookmarkWidget::removeBookmark()
{
    rApp->bookmarkManager()->owner()->deleteBookmark(*m_bookmark);
    close();

    emit updateIcon();
}


#ifdef HAVE_NEPOMUK
void BookmarkWidget::addTags(QList<Nepomuk::Tag> tagList)
{
    Q_FOREACH(const Nepomuk::Tag & tag, tagList)
    {
        if (!m_nfoResource.tags().contains(tag))
        {
            m_nfoResource.addTag(tag);
        }
    }
    Q_FOREACH(Nepomuk::Tag tag, m_nfoResource.tags())
    {
        if (!tagList.contains(tag))
        {
            tag.remove();
        }
    }
}

void BookmarkWidget::parseTags()
{
    QList<Nepomuk::Tag> tagList;
    if (m_tagLine->text().contains(','))
    {
        QString text = m_tagLine->text();
        QStringList tagStringList = text.split(QChar::fromAscii(','));

        Q_FOREACH(const QString & tag, tagStringList)
        {
            QString trimmedTag = tag.trimmed();
            if (!trimmedTag.isEmpty())
                tagList << trimmedTag;
        }
    }
    else
    {
        tagList << m_tagLine->text().trimmed();
    }
    addTags(tagList);
}


void BookmarkWidget::loadTags()
{
    QString list;
    if (!m_nfoResource.tags().isEmpty())
    {
        Q_FOREACH(const Nepomuk::Tag & tag, m_nfoResource.tags())
        {
            list.append(tag.genericLabel());
            list.append(",");
        }
        m_tagLine->setText(list);
    }
}


void BookmarkWidget::setRatingSlot(int rate)
{
    m_nfoResource.setRating(rate);
}


void BookmarkWidget::addCommentSlot()
{
    m_nfoResource.setDescription(m_commentEdit->toPlainText());
}


void BookmarkWidget::linkToResourceSlot()
{
    Nepomuk::ResourceLinkDialog r(m_nfoResource);
    r.exec();
}
#endif
