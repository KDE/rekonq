/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) <year>  <name of author>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef RSSWIDGET_H
#define RSSWIDGET_H

// Qt Includes
#include <QFrame>
#include <KUrl>

// KDE Includes
#include <KComboBox>


class RSSWidget : public QFrame
{
    Q_OBJECT
    
public:
    // QMap< feedUrl, feedTitle>
    RSSWidget(QMap<KUrl, QString> map, QWidget *parent);
    
    void showAt(QPoint pos);
    
public slots:
    void accepted();
    
private:
    void addWithAkregator(QString url);
    void addWithGoogleReader(QString url);
    
    QMap<KUrl, QString> m_map;
    
    KComboBox *m_agregators;
    KComboBox *m_feeds;
};

#endif // RSSWIDGET_H
