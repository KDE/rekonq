/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2008-2009 by Andrea Diamantini <adjam7 at gmail dot com>
*
*
* This program is free software; you can redistribute it
* and/or modify it under the terms of the GNU General
* Public License as published by the Free Software Foundation;
* either version 2, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* ============================================================ */



#ifndef SEARCHBAR_H
#define SEARCHBAR_H

// Local Includes
#include "lineedit.h"

// Forward Declarations
class KUrl;
class QFocusEvent;
class QTimer;
class QNetworkAccessManager;
class QNetworkReply;

/**
 * This class defines an internet search bar.
 *
 */
class SearchBar : public LineEdit
{
    Q_OBJECT

public:
    SearchBar(QWidget *parent = 0);
    ~SearchBar();

public slots:
    void autoSuggest();
    void handleNetworkData(QNetworkReply *networkReply);

    /**
    *  Use this slot to perform one search in one search engine
    *
    */
    void searchNow();

protected:
    void focusInEvent(QFocusEvent *);

signals:
    void search(const KUrl &);

private:
    QNetworkAccessManager *m_networkAccessManager;
    QTimer *m_timer;
};

#endif
