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


#ifndef BOOKMARKWIDGET_H
#define BOOKMARKWIDGET_H

// Qt Includes
#include <QMenu>
#include <QGridLayout>
#include <QPlainTextEdit>

// Nepomuk Includes
#ifdef HAVE_NEPOMUK
#include <Nepomuk2/Resource>
#include <Nepomuk2/Tag>
#include <Nepomuk2/Vocabulary/NFO>
#endif

// Forward Declarations
class KBookmark;
class KLineEdit;
class KComboBox;


class BookmarkWidget : public QMenu
{
    Q_OBJECT

public:
    explicit BookmarkWidget(const KBookmark &bookmark, QWidget *parent = 0);
    virtual ~BookmarkWidget();

    void showAt(const QPoint &pos);

#ifdef HAVE_NEPOMUK
    void addTags(QList<Nepomuk2::Tag>);
    void parseTags();
    void loadTags();
#endif

Q_SIGNALS:
    void updateIcon();

private:
    void setupFolderComboBox();

private Q_SLOTS:
    void accept();
    void removeBookmark();

#ifdef HAVE_NEPOMUK
    void setRatingSlot(int rate);
    void addCommentSlot();
    void linkToResourceSlot();
#endif

private:
    KBookmark *m_bookmark;
    KLineEdit *m_name;
    KComboBox *m_folder;
    KLineEdit *m_tagLine;
    QPlainTextEdit *m_commentEdit;
    QStringList m_tList;

#ifdef HAVE_NEPOMUK
    Nepomuk2::Resource m_nfoResource;
    bool m_isNepomukEnabled;
#endif
};

#endif
