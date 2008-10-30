/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the demonstration applications of the Qt Toolkit.
**
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License versions 2.0 or 3.0 as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file.  Please review the following information
** to ensure GNU General Public Licensing requirements will be met:
** http://www.fsf.org/licensing/licenses/info/GPLv2.html and
** http://www.gnu.org/copyleft/gpl.html.  In addition, as a special
** exception, Nokia gives you certain additional rights. These rights
** are described in the Nokia Qt GPL Exception version 1.3, included in
** the file GPL_EXCEPTION.txt in this package.
**
** Qt for Windows(R) Licensees
** As a special exception, Nokia, as the sole copyright holder for Qt
** Designer, grants users of the Qt/Eclipse Integration plug-in the
** right for the Qt/Eclipse Integration to link to functionality
** provided by Qt Designer and its related libraries.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
**
****************************************************************************/

#ifndef MODELMENU_H
#define MODELMENU_H

#include <QtGui/QMenu>
#include <QtCore/QAbstractItemModel>

// A QMenu that is dynamically populated from a QAbstractItemModel
class ModelMenu : public QMenu
{
    Q_OBJECT

signals:
    void activated(const QModelIndex &index);
    void hovered(const QString &text);

public:
    ModelMenu(QWidget *parent = 0);

    void setModel(QAbstractItemModel *model);
    QAbstractItemModel *model() const;

    void setMaxRows(int max);
    int maxRows() const;

    void setFirstSeparator(int offset);
    int firstSeparator() const;

    void setRootIndex(const QModelIndex &index);
    QModelIndex rootIndex() const;

    void setHoverRole(int role);
    int hoverRole() const;

    void setSeparatorRole(int role);
    int separatorRole() const;

    QAction *makeAction(const QIcon &icon, const QString &text, QObject *parent);

protected:
    // add any actions before the tree, return true if any actions are added.
    virtual bool prePopulated();
    // add any actions after the tree
    virtual void postPopulated();
    // put all of the children of parent into menu up to max
    void createMenu(const QModelIndex &parent, int max, QMenu *parentMenu = 0, QMenu *menu = 0);

private slots:
    void aboutToShow();
    void triggered(QAction *action);
    void hovered(QAction *action);

private:
    QAction *makeAction(const QModelIndex &index);
    int m_maxRows;
    int m_firstSeparator;
    int m_maxWidth;
    int m_hoverRole;
    int m_separatorRole;
    QAbstractItemModel *m_model;
    QPersistentModelIndex m_root;
};

#endif // MODELMENU_H

