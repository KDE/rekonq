/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2008 by Andrea Diamantini <adjam7 at gmail dot com>
* Copyright (C) 2009 by Paweł Prażak <pawelprazak at gmail dot com>
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



#ifndef STACKEDURLBAR_H
#define STACKEDURLBAR_H

// Qt Includes
#include <QStackedWidget>

// Forward Declarations
class KCompletion;
class HistoryCompletionModel;
class UrlBar;


class StackedUrlBar : public QStackedWidget
{
    Q_OBJECT

public:
    StackedUrlBar(QWidget *parent = 0);
    ~StackedUrlBar();

public:
    UrlBar *currentUrlBar();
    UrlBar *urlBar(int index);
    void addUrlBar(UrlBar *urlBar);
    void setCurrentUrlBar(UrlBar *urlBar);
    void removeUrlBar(UrlBar *urlBar);

    QList<const UrlBar *> urlBars();

    KCompletion *completion();
    HistoryCompletionModel *completionModel();

public slots:
    void clear();

private:
    Q_DISABLE_COPY(StackedUrlBar)

    KCompletion *m_completion;
    HistoryCompletionModel *m_completionModel;
};

#endif // STACKEDURLBAR_H
