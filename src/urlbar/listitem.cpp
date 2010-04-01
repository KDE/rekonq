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


// Self Includes
#include "listitem.h"   

// Local Includes
#include "urlresolver.h"
#include "application.h"

// KDE Includes
#include <KIcon>
#include <KStandardDirs>
#include <KDebug>

// Qt Includes
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QSizePolicy>
#include <QPixmap>
#include <QStylePainter>
#include <QFile>
#include <QMouseEvent>
#include <QWebSettings>


ListItem::ListItem(const UrlSearchItem &item, QWidget *parent)
    : QWidget(parent)
    , m_option()
{
    //preview and icon
    
    QHBoxLayout *hLayout = new QHBoxLayout;
 
    QLabel *previewLabelIcon = new QLabel;
    previewLabelIcon->setFixedSize(45,33);
    hLayout->addWidget(previewLabelIcon);

    QPixmap pixmapIcon = KIcon(QWebSettings::iconForUrl(item.url)).pixmap(16);
    QString path = KStandardDirs::locateLocal("cache", QString("thumbs/") + guessNameFromUrl(item.url) + ".png", true);
    if(QFile::exists(path))
    {
        QLabel *previewLabel = new QLabel(previewLabelIcon);
        previewLabel->setFrameStyle(QFrame::StyledPanel | QFrame::Raised);
        QPixmap preview;
        preview.load(path);
        if (!pixmapIcon.isNull())
        {
            previewLabel->setFixedSize(38,29);
            previewLabel->setPixmap(preview.scaled(38,29, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
        }
        else
        {
            previewLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
            previewLabel->setFixedSize(45,33);
            previewLabel->setPixmap(preview.scaled(45,33, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));  
        }
    } 

    if (!pixmapIcon.isNull())
    {
        QLabel *iconLabel = new QLabel(previewLabelIcon);
        iconLabel->setPixmap(pixmapIcon);
        iconLabel->move(27, 16);
    }
    
    //title and url
    
    QVBoxLayout *vLayout = new QVBoxLayout;  
    hLayout->addLayout(vLayout);
    
    QLabel *titleLabel = new QLabel("<b>" + item.title + "</b>");
    titleLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    
    QLabel *urlLabel = new QLabel("<i>" + item.url + "</i>");
    urlLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        
    vLayout->addWidget(titleLabel);
    vLayout->addWidget(urlLabel);

    //type icon

    if (item.type & UrlSearchItem::Browse)
    {
        insertIcon(hLayout, "applications-internet");
    }
    
    if (item.type & UrlSearchItem::Search)
    {
         insertIcon(hLayout, "edit-find");
    }
   
    if (item.type & UrlSearchItem::Bookmark)
    {
         insertIcon(hLayout, "rating");
    }

    if (item.type & UrlSearchItem::History)
    {
         insertIcon(hLayout, "view-history");
    }

    setLayout(hLayout);
    
    m_option.initFrom(this);
    m_option.direction = Qt::LeftToRight;
    
    deactivate();
}


ListItem::~ListItem()
{
    disconnect();
}


void ListItem::insertIcon(QLayout *layout, QString icon)
{
    QLabel *iconLabel = new QLabel;
    QPixmap pixmap = KIcon(icon).pixmap(18);
    iconLabel->setPixmap(pixmap);
    iconLabel->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);
    layout->addWidget(iconLabel);
}


//TODO: REMOVE DUPLICATE CODE WITH PREVIEWIMAGE
QString ListItem::guessNameFromUrl(QUrl url)
{
    QString name = url.toString( QUrl::RemoveScheme | QUrl::RemoveUserInfo | QUrl::StripTrailingSlash );
    
    // TODO learn Regular Expressions :)
    // and implement something better here..
    name.remove('/');
    name.remove('&');
    name.remove('.');
    name.remove('-');
    name.remove('_');
    name.remove('?');
    name.remove('=');
    name.remove('+');
    
    return name;
}


void ListItem::activate()
{
    m_option.state |= QStyle::State_Selected;
    update();
}


void ListItem::deactivate()
{
    m_option.state  &= ~QStyle::State_Selected;
    update();
}


void ListItem::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    
    if( m_option.state.testFlag(QStyle::State_Selected) ||  m_option.state.testFlag(QStyle::State_MouseOver))
    {
        QPainter painter(this);
        m_option.rect=QRect(QPoint(),size());
        style()->drawPrimitive(QStyle::PE_PanelItemViewItem, &m_option, &painter, this);
    }
}


void ListItem::enterEvent(QEvent *e)
{
    m_option.state |= QStyle::State_MouseOver;
    update();
    QWidget::enterEvent(e);
}


void ListItem::leaveEvent(QEvent *e)
{
    m_option.state &= ~QStyle::State_MouseOver;
    update();
    QWidget::enterEvent(e);
}


void ListItem::mousePressEvent(QMouseEvent *e)
{
    emit itemClicked(this, e->button());
    QWidget::mousePressEvent(e);
}
