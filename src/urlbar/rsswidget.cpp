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


// Auto Includes
#include "rsswidget.h"

// KDE Includes
#include <KLocalizedString>
#include <KProcess>

// Qt Includes
#include <QComboBox>
#include <QDBusConnectionInterface>
#include <QDBusInterface>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QUrl>


RSSWidget::RSSWidget(const QMap< QUrl, QString > &map, QWidget *parent)
    : QMenu(parent)
    , m_map(map)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setFixedWidth(350);

    QFormLayout *layout = new QFormLayout(this);

    // Title
    QLabel *title = new QLabel(this);
    title->setText(i18n("Subscribe to RSS Feeds"));
    QFont f = title->font();
    f.setBold(true);
    title->setFont(f);
    layout->addRow(title);

    // Agregators
    QLabel *agregator = new QLabel(this);
    agregator->setText(i18n("Aggregator:"));

    m_agregators = new QComboBox(this);
    m_agregators->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    m_agregators->addItem(QIcon::fromTheme( QL1S("akregator") ), QL1S("Akregator"));
//     m_agregators->addItem(IconManager::self()->iconForUrl(QUrl("http://google.com/reader")), i18n("Google Reader"));

    layout->addRow(agregator, m_agregators);

    // Feeds List
    QLabel *feed = new QLabel(this);
    feed->setText(i18n("Feed:"));

    m_feeds = new QComboBox(this);
    m_feeds->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    Q_FOREACH(const QString & title, m_map)
    {
        m_feeds->addItem(title);
    }

    layout->addRow(feed, m_feeds);

    // Buttons
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Cancel, Qt::Horizontal, this);

    QPushButton *addFeed = new QPushButton(QIcon::fromTheme( QL1S("list-add") ), i18n("Add Feed"), buttonBox);
    buttonBox->addButton(addFeed, QDialogButtonBox::AcceptRole);

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(close()));

    layout->addRow(buttonBox);
}


void RSSWidget::showAt(const QPoint &pos)
{
    adjustSize();

    QPoint p(pos.x() - width(), pos.y() + 10);
    move(p);
    show();
}


void RSSWidget::accept()
{
    QString url = m_map.key(m_feeds->currentText()).toString();

    if (m_agregators->currentIndex() == 0)
        addWithAkregator(url);
//     else
//         addWithGoogleReader(url);

    close();
}


// void RSSWidget::addWithGoogleReader(const QString &url)
// {
//     QUrl toLoad = QUrl("http://www.google.com/ig/add?feedurl=" + url);
//     rApp->rekonqWindow()->loadUrl(toLoad);
// }


void RSSWidget::addWithAkregator(const QString &url)
{
    // Akregator is running
    if (QDBusConnection::sessionBus().interface()->isServiceRegistered( QL1S("org.kde.akregator") ))
    {
        QDBusInterface akregator( QL1S("org.kde.akregator"), QL1S("/Akregator"), QL1S("org.kde.akregator.part") );
        QDBusReply<void> reply = akregator.call( QL1S("addFeedsToGroup"), QStringList(url) , i18n("Imported Feeds"));

        if (!reply.isValid())
        {
            QMessageBox::critical(this, i18n("Could not add feed to Akregator. Please add it manually:")
                                          + QL1S("<br /><br /> <a href=\"") + url +  QL1S("\">") + url + QL1S("</a>") , QString() );
        }
    }
    // Akregator is not running
    else
    {
        KProcess proc;
        proc << QL1S("akregator") << QL1S("-g") << i18n("Imported Feeds");
        proc << QL1S("-a") << url;
        if (proc.startDetached() == 0)
        {
            QMessageBox::critical(this, i18n("There was an error. Please verify Akregator is installed on your system.")
                                          + QL1S("<br /><br /> <a href=\"") + url + QL1S("\">") + url + QL1S("</a>") , QString() );
        }
    }
}
