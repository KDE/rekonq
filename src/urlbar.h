/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2008-2009 by Andrea Diamantini <adjam7 at gmail dot com>
* Copyright (C) 2009 by Domrachev Alexandr <alexandr.domrachev@gmail.com>
* Copyright (C) 2009 by Paweł Prażak <pawelprazak at gmail dot com>
* Copyright (C) 2009 by Lionel Chauvin <megabigbug@yahoo.fr>
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


#ifndef URLBAR_H
#define URLBAR_H


// Local Includes
#include "lineedit.h"

// KDE Includes
#include <KUrl>
#include <KHistoryComboBox>

// Qt Includes
#include <QUrl>

// Forward Declarations
class QLinearGradient;
class QWidget;
class KCompletion;
class HistoryCompletionModel;

class UrlBar : public KHistoryComboBox
{
    Q_OBJECT

public:
    UrlBar(QWidget *parent = 0);
    ~UrlBar();

    void selectAll() const;
    KUrl url() const;
    QSize sizeHint() const;
    void setBackgroundColor(QColor);
    bool isLoading();
    KCompletion *completion();
    HistoryCompletionModel *completionModel();
    void setProgress(int progress);

signals:
    void activated(const KUrl&);

public slots:
    void setUrl(const QUrl &url);
    void slotUpdateProgress(int progress);

private slots:
    void slotActivated(const QString&);
    void slotLoadFinished(bool);
    void slotCleared();
    void slotUpdateUrl();

protected:
    virtual void paintEvent(QPaintEvent *event);
    virtual void focusOutEvent(QFocusEvent *event);

private:
    void setupLineEdit();

    KLineEdit *lineEdit() const;

    static QLinearGradient generateGradient(const QColor &color, int height);

    static QColor s_defaultBaseColor;

    LineEdit *m_lineEdit;

    KUrl m_currentUrl;
    int m_progress;
    
    KCompletion *m_completion;
    HistoryCompletionModel *m_completionModel;
};

#endif
