/* ============================================================
 *
 * This file is a part of the rekonq project
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


#ifndef SEARCHBAR_H
#define SEARCHBAR_H

// KDE Includes
#include <KLineEdit>
#include <KUrl>

// Qt Includes
#include <QtGui>

class SearchBar : public QWidget
{
Q_OBJECT

public:
    SearchBar(QWidget *parent = 0);
    ~SearchBar();

    KLineEdit *lineEdit();

    friend class KLineEdit;

public slots:
    void searchNow();

protected:
//     void resizeEvent(QResizeEvent *);
//     void focusInEvent(QFocusEvent *);

private:
    KLineEdit *m_lineEdit;

signals:
    void search(const KUrl &url);

};

#endif
