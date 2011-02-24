/* This file is part of the KDE project
 *
 * Copyright (C) 2009 by Fredy Yanardi <fyanardi@gmail.com>
 * Copyright (C) 2010-2011 by Lionel Chauvin <megabigbug@yahoo.fr>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef WEBSHORTCUTWIDGET_H
#define WEBSHORTCUTWIDGET_H

#include <QtGui/QDialog>
#include <KUrl>
#include <KService>

class QLabel;
class QLineEdit;

class WebShortcutWidget : public QDialog
{
    Q_OBJECT
public:
    explicit WebShortcutWidget(QWidget *parent = 0);

    void show(const KUrl &url, const QString &openSearchName, const QPoint &pos);

private slots:
    void okClicked();
    void cancelClicked();
    void shortcutsChanged(const QString& newShorthands);

signals:
    void webShortcutSet(const KUrl &url, const QString &openSearchName, const QString &webShortcut);

private:
    QLabel *m_searchTitleLabel;
    QLineEdit *m_wsLineEdit;
    QLineEdit *m_nameLineEdit;
    QLabel *m_noteLabel;
    QPushButton *m_okButton;

    KService::List m_providers;
    KUrl m_url;

    void showAt(const QPoint &pos);
};

#endif // WEBSHORTCUTWIDGET_H

