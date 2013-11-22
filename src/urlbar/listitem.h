/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2009-2013 by Andrea Diamantini <adjam7 at gmail dot com>
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


#ifndef LISTITEM_H
#define LISTITEM_H


// Rekonq Includes
#include "rekonq_defines.h"

// KDE Includes
#include <KToolBar>
#include <KService>
#include <KUrl>

// Qt Includes
#include <QWidget>
#include <QLabel>
#include <QStyleOptionViewItemV4>
#include <QString>
#include <QByteArray>

// Forward Declarations
class UrlSuggestionItem;

class KAction;
class KIcon;
class KJob;

class QActionGroup;


class REKONQ_TESTS_EXPORT ListItem : public QWidget
{
    Q_OBJECT

public:
    explicit ListItem(const UrlSuggestionItem &item, QWidget *parent = 0);
    virtual ~ListItem();

    void activate();
    void deactivate();

    KUrl url();
    virtual QString text();

public Q_SLOTS:
    virtual void nextItemSubChoice();

Q_SIGNALS:
    void itemClicked(ListItem *item, Qt::MouseButton, Qt::KeyboardModifiers);

protected:
    virtual void paintEvent(QPaintEvent *event);
    virtual void enterEvent(QEvent *);
    virtual void leaveEvent(QEvent *);
    virtual void mousePressEvent(QMouseEvent *e);

private:
    QStyleOptionViewItemV4 m_option;

protected:
    KUrl m_url;
};


// -------------------------------------------------------------------------


class REKONQ_TESTS_EXPORT TypeIconLabel : public QLabel
{
    Q_OBJECT

public:
    explicit TypeIconLabel(int type, QWidget *parent = 0);
    
private:
    QLabel *getIcon(QString icon);
};


// -------------------------------------------------------------------------


class REKONQ_TESTS_EXPORT TextLabel : public QLabel
{
    Q_OBJECT

public:
    explicit TextLabel(const QString &text, const QString &textToPointOut = QString(), QWidget *parent = 0);
    explicit TextLabel(QWidget *parent = 0);
    
    void setEngineText(const QString &engine, const QString &text);
};


// -------------------------------------------------------------------------


class REKONQ_TESTS_EXPORT EngineBar : public KToolBar
{
    Q_OBJECT

public:
    explicit EngineBar(KService::Ptr selectedEngine, QWidget *parent = 0);
    
    void selectNextEngine();

Q_SIGNALS:
    void searchEngineChanged(KService::Ptr engine);

private Q_SLOTS:
    void changeSearchEngine();

private:
    KAction *newEngineAction(KService::Ptr engine, KService::Ptr selectedEngine);
    QActionGroup *m_engineGroup;
    
};


// -------------------------------------------------------------------------


class REKONQ_TESTS_EXPORT SearchListItem : public ListItem
{
    Q_OBJECT

public:
    explicit SearchListItem(const UrlSuggestionItem &item, const QString &text, QWidget *parent = 0);
    
    QString text();

public Q_SLOTS:
    virtual void nextItemSubChoice();

private Q_SLOTS:
    void changeSearchEngine(KService::Ptr engine);

private:
    TextLabel* m_titleLabel;
    EngineBar* m_engineBar;
    QString m_text;
    KService::Ptr m_currentEngine;
};


// -------------------------------------------------------------------------


class REKONQ_TESTS_EXPORT PreviewListItem : public ListItem
{
    Q_OBJECT

public:
    PreviewListItem(const UrlSuggestionItem &item, const QString &text, QWidget *parent = 0);
};


// -------------------------------------------------------------------------


class REKONQ_TESTS_EXPORT BrowseListItem : public ListItem
{
    Q_OBJECT

public:
    BrowseListItem(const UrlSuggestionItem &item, const QString &text, QWidget *parent = 0);
};


//-------------------------------------------------------------------------------------------------


class REKONQ_TESTS_EXPORT ListItemFactory
{
public:
    static ListItem *create(const UrlSuggestionItem &item, const QString &text, QWidget *parent);
};


#endif
