/* ============================================================
 *
 * This file is a part of the reKonq project
 *
 * Copyright (C) 2008 by Andrea Diamantini <adjam7 at gmail dot com>
 *
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef XBEL_H
#define XBEL_H

#include <QtCore/QXmlStreamReader>
#include <QtCore/QDateTime>

class BookmarkNode
{
public:
    enum Type {
        Root,
        Folder,
        Bookmark,
        Separator
    };

    BookmarkNode(Type type = Root, BookmarkNode *parent = 0);
    ~BookmarkNode();
    bool operator==(const BookmarkNode &other);

    Type type() const;
    void setType(Type type);
    QList<BookmarkNode *> children() const;
    BookmarkNode *parent() const;

    void add(BookmarkNode *child, int offset = -1);
    void remove(BookmarkNode *child);

    QString url;
    QString title;
    QString desc;
    bool expanded;

private:
    BookmarkNode *m_parent;
    Type m_type;
    QList<BookmarkNode *> m_children;

};

class XbelReader : public QXmlStreamReader
{
public:
    XbelReader();
    BookmarkNode *read(const QString &fileName);
    BookmarkNode *read(QIODevice *device);

private:
    void skipUnknownElement();
    void readXBEL(BookmarkNode *parent);
    void readTitle(BookmarkNode *parent);
    void readDescription(BookmarkNode *parent);
    void readSeparator(BookmarkNode *parent);
    void readFolder(BookmarkNode *parent);
    void readBookmarkNode(BookmarkNode *parent);
};

#include <QtCore/QXmlStreamWriter>

class XbelWriter : public QXmlStreamWriter
{
public:
    XbelWriter();
    bool write(const QString &fileName, const BookmarkNode *root);
    bool write(QIODevice *device, const BookmarkNode *root);

private:
    void writeItem(const BookmarkNode *parent);
};

#endif // XBEL_H

