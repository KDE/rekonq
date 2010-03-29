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

ListItem::ListItem(const UrlSearchItem &item, QWidget *parent)
    : QWidget(parent),
    m_option()
{
    QHBoxLayout *hLayout = new QHBoxLayout;
    QVBoxLayout *vLayout = new QVBoxLayout;

    QLabel *previewLabel = new QLabel;
    previewLabel->setFixedSize(40,30);
    QPixmap preview;
    QString path = KStandardDirs::locateLocal("cache", QString("thumbs/") + guessNameFromUrl(item.url) + ".png", true);
    if(QFile::exists(path))
    {
        preview.load(path);
        previewLabel->setPixmap(preview.scaled(40,30));
    }
    else
    {
        if(item.icon.startsWith( QLatin1String("http://") ) )
            preview = Application::icon( item.icon ).pixmap(22);
    }
    previewLabel->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);
    
    hLayout->addWidget(previewLabel);
    hLayout->addLayout(vLayout);
    
    QLabel *titleLabel = new QLabel("<b>" + item.title + "</b>");
    titleLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    
    QLabel *urlLabel = new QLabel("<i>" + item.url + "</i>");
    urlLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        
    vLayout->addWidget(titleLabel);
    vLayout->addWidget(urlLabel);


    QLabel *iconLabel = new QLabel;
    QPixmap pixmap;
    if(item.icon.startsWith( QLatin1String("http://") ) )
        pixmap = Application::icon( item.icon ).pixmap(18);
    else
        pixmap = KIcon(item.icon).pixmap(18);

    iconLabel->setPixmap(pixmap);
    iconLabel->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);
    hLayout->addWidget(iconLabel);

    setLayout(hLayout);
    
    m_option.initFrom(this);
    m_option.direction = Qt::LeftToRight;
    
    deactivate();
}

ListItem::~ListItem()
{
    disconnect();
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
    repaint();
}


void ListItem::deactivate()
{
    m_option.state  &= ~QStyle::State_Selected;
    repaint();
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
    repaint();
    QWidget::enterEvent(e);
}

void ListItem::leaveEvent(QEvent *e)
{
    m_option.state &= ~QStyle::State_MouseOver;
    repaint();
    QWidget::enterEvent(e);
}

void ListItem::mousePressEvent(QMouseEvent *e)
{
    emit itemClicked(this);
    QWidget::mousePressEvent(e);
}

