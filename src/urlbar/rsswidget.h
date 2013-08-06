/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2010-2011 by Matthieu Gicquel <matgic78 at gmail dot com>
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


#ifndef RSSWIDGET_H
#define RSSWIDGET_H

// Qt Includes
#include <QMenu>

// Forward Declarations
class KComboBox;
class KUrl;


class RSSWidget : public QMenu
{
    Q_OBJECT

public:
    // QMap< feedUrl, feedTitle>
    RSSWidget(const QMap<KUrl, QString> &map, QWidget *parent = 0);

    void showAt(const QPoint &pos);

private Q_SLOTS:
    void accept();

private:
    void addWithAkregator(const QString &url);
//     void addWithGoogleReader(const QString &url);

    QMap<KUrl, QString> m_map;

    KComboBox *m_agregators;
    KComboBox *m_feeds;
};

#endif // RSSWIDGET_H
