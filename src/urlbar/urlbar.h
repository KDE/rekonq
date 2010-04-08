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
#include "completionwidget.h"
#include "webtab.h"

// KDE Includes
#include <KUrl>
#include <KComboBox>

// Qt Includes
#include <QUrl>
#include <QPointer>

// Forward Declarations
class QLinearGradient;
class QWidget;


class UrlBar : public LineEdit
{
    Q_OBJECT

public:
    UrlBar(QWidget *parent = 0);
    ~UrlBar();

    void setPrivateMode(bool on);

private slots:
    void activated(const QString& url, Rekonq::OpenType = Rekonq::CurrentTab);
    void suggestUrls(const QString &editedText);
    void setQUrl(const QUrl &url);

    void loadFinished();
    
protected:
    virtual void paintEvent(QPaintEvent *event);
    virtual void keyPressEvent(QKeyEvent *event);
    virtual void focusInEvent(QFocusEvent *event);
    virtual void dropEvent(QDropEvent *event);

private:
    CompletionWidget *_box;
    WebTab *_tab;
    bool _privateMode;
};

#endif
