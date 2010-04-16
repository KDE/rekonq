/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2009 by Andrea Diamantini <adjam7 at gmail dot com>
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


// Local Includes
#include "urlresolver.h"

// KDE Includes
#include <KToolBar>
#include <KAction>
#include <KService>

// Qt Includes
#include <QWidget>
#include <QLayout>
#include <QStyleOptionViewItemV4>
#include <QLabel>

// Forward Declarations
class UrlSearchItem;


class ListItem : public QWidget
{
    Q_OBJECT

public:
    ListItem(const UrlSearchItem &item, QWidget *parent = 0);
    virtual ~ListItem();

    void activate();
    void deactivate();

    KUrl url();
    
public slots:
    virtual void nextItemSubChoice();

signals:
    void itemClicked(ListItem *item, Qt::MouseButton);

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


class TypeIcon : public QLabel
{
    Q_OBJECT
    
public:
    TypeIcon(int type, QWidget *parent = 0);

private:
    QLabel *getIcon(QString icon);
};


// -------------------------------------------------------------------------


class ItemIcon : public QLabel
{
    Q_OBJECT

public:
    ItemIcon(const QString &icon, QWidget *parent = 0);
};


// -------------------------------------------------------------------------


class ItemText : public QLabel
{
    Q_OBJECT
    
public:
    ItemText(const QString &text, const QString &textToPointOut, QWidget *parent = 0);
};


// -------------------------------------------------------------------------


class EngineBar : public KToolBar
{
    Q_OBJECT
    
public:
    EngineBar(const QString &text, const QString &selectedEngine, QWidget *parent = 0);
    static QString defaultEngine();
    void selectNextEngine();
    KUrl url() { return m_url; };
    
signals:
    void searchEngineChanged(QString url, QString engine);

private slots:
    void changeSearchEngine();

private:
    KAction *newEngineAction(KService::Ptr service, QString selectedEngine);

    QActionGroup *m_engineGroup;
    KUrl m_url;
};


// -------------------------------------------------------------------------


class SearchListItem : public ListItem
{
    Q_OBJECT
    
public:
    SearchListItem(const UrlSearchItem &item, const QString &text, QWidget *parent = 0);
    
public slots:
    virtual void nextItemSubChoice();
    
private slots:
    void changeSearchEngine(QString url, QString engine);
    
private:
    QString searchItemTitle(QString engine, QString text);
    ItemText* m_titleLabel;
    ItemIcon* m_iconLabel;
    EngineBar* m_engineBar;
    QString m_text;
    
    static QString m_currentEngine;
};


// -------------------------------------------------------------------------


class PreviewListItem : public ListItem
{
    Q_OBJECT
    
public:
    PreviewListItem(const UrlSearchItem &item, const QString &text, QWidget *parent = 0);
};


// -------------------------------------------------------------------------


class ItemPreview : public QLabel
{
    Q_OBJECT
    
public:
    ItemPreview(const QString &url, int width, int height, QWidget *parent = 0);

private:
    static QString guessNameFromUrl(QUrl url);
};


// -------------------------------------------------------------------------


class BrowseListItem : public ListItem
{
    Q_OBJECT
    
public:
    BrowseListItem(const UrlSearchItem &item, const QString &text, QWidget *parent = 0);
};


//-------------------------------------------------------------------------------------------------


class ListItemFactory
{
public:
    static ListItem *create(const UrlSearchItem &item, const QString &text, QWidget *parent = 0);
};


#endif
