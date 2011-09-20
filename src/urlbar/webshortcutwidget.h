/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2009 by Fredy Yanardi <fyanardi@gmail.com>
* Copyright (C) 2010-2011 by  Lionel Chauvin <megabigbug@yahoo.fr>
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


#ifndef WEBSHORTCUTWIDGET_H
#define WEBSHORTCUTWIDGET_H

#include <QMenu>
#include <KUrl>
#include <KService>

class QLabel;
class QLineEdit;
class QPushButton;

class WebShortcutWidget : public QMenu
{
    Q_OBJECT
public:
    WebShortcutWidget(QWidget *parent = 0);

    void show(const KUrl &url, const QString &openSearchName, const QPoint &pos);

private slots:
    void accept();
    void shortcutsChanged(const QString& newShorthands);

signals:
    void webShortcutSet(const KUrl &url, const QString &openSearchName, const QString &webShortcut);

private:
    QLineEdit *m_wsLineEdit;
    QLineEdit *m_nameLineEdit;
    QLabel *m_noteLabel;

    KService::List m_providers;
    KUrl m_url;

    void showAt(const QPoint &pos);
};

#endif // WEBSHORTCUTWIDGET_H

