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

#ifndef SEARCHBAR_H
#define SEARCHBAR_H

#include <KHistoryComboBox>
#include <KProcess>
#include <KAction>
#include <KDialog>

#include <QPixmap>
#include <QString>
#include <QMenu>

class KHTMLPart;


class SearchBarCombo : public KHistoryComboBox
{
    Q_OBJECT

public:
    SearchBarCombo(QWidget *parent);
    ~SearchBarCombo();
    const QPixmap &icon() const;
    void setIcon(const QPixmap &icon);
    int findHistoryItem(const QString &text);

public slots:
    virtual void show();

signals:
    void iconClicked();

protected:
    virtual void mousePressEvent(QMouseEvent *e);

private slots:
    void historyCleared();

private:
    QPixmap m_icon;
};


class SearchBar : public KDialog
{
    Q_OBJECT

public:
    enum SearchModes { FindInThisPage = 0, UseSearchProvider };

    SearchBar();
    virtual ~SearchBar();

private slots:
    void startSearch(const QString &search);
    void setIcon();
    void showSelectionMenu();

    void useFindInThisPage();
    void useSearchProvider(QAction *);
    void selectSearchEngines();
    void searchEnginesSelected(int, QProcess::ExitStatus);
    void configurationChanged();

    void updateComboVisibility();

    void focusSearchbar();
private:
    void nextSearchEntry();
    void previousSearchEntry();


    SearchBarCombo        *m_searchCombo;
    KAction               *m_searchComboAction;
    QMenu                 *m_popupMenu;
    QPixmap                m_searchIcon;
    SearchModes            m_searchMode;
    QString                m_providerName;
    bool                   m_urlEnterLock;
    QString                m_currentEngine;
    QStringList            m_searchEngines;
    KProcess              *m_process;
};

#endif
