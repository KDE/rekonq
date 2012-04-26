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
#include "rsswidget.moc"

// Local includes
#include "application.h"
#include "iconmanager.h"
#include "mainwindow.h"
#include "webtab.h"

// KDE Includes
#include <KComboBox>
#include <KLocalizedString>
#include <KMessageBox>
#include <KProcess>
#include <KUrl>

// Qt Includes
#include <QtDBus/QDBusConnectionInterface>
#include <QtDBus/QDBusInterface>

#include <QtGui/QDialogButtonBox>
#include <QtGui/QFormLayout>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>


RSSWidget::RSSWidget(const QMap< KUrl, QString > &map, QWidget *parent)
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

    m_agregators = new KComboBox(this);
    m_agregators->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    m_agregators->addItem(KIcon("akregator"), QString("Akregator"));
    m_agregators->addItem(rApp->iconManager()->iconForUrl(KUrl("http://google.com/reader")), i18n("Google Reader"));

    layout->addRow(agregator, m_agregators);

    // Feeds List
    QLabel *feed = new QLabel(this);
    feed->setText(i18n("Feed:"));

    m_feeds = new KComboBox(this);
    m_feeds->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    Q_FOREACH(const QString & title, m_map)
    {
        m_feeds->addItem(title);
    }

    layout->addRow(feed, m_feeds);

    // Buttons
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Cancel, Qt::Horizontal, this);

    QPushButton *addFeed = new QPushButton(KIcon("list-add"), i18n("Add Feed"), buttonBox);
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
    QString url = m_map.key(m_feeds->currentText()).toMimeDataString();

    if (m_agregators->currentIndex() == 0)
        addWithAkregator(url);
    else
        addWithGoogleReader(url);

    close();
}


void RSSWidget::addWithGoogleReader(const QString &url)
{
    KUrl toLoad = KUrl("http://www.google.com/ig/add?feedurl=" + url);
    rApp->mainWindow()->currentTab()->view()->load(toLoad);
}


void RSSWidget::addWithAkregator(const QString &url)
{
    // Akregator is running
    if (QDBusConnection::sessionBus().interface()->isServiceRegistered("org.kde.akregator"))
    {
        QDBusInterface akregator("org.kde.akregator", "/Akregator", "org.kde.akregator.part");
        QDBusReply<void> reply = akregator.call("addFeedsToGroup", QStringList(url) , i18n("Imported Feeds"));

        if (!reply.isValid())
        {
            KMessageBox::error(0, QString(i18n("Could not add feed to Akregator. Please add it manually:")
                                          + "<br /><br /> <a href=\"" + url + "\">" + url + "</a>"));
        }
    }
    // Akregator is not running
    else
    {
        KProcess proc;
        proc << "akregator" << "-g" << i18n("Imported Feeds");
        proc << "-a" << url;
        if (proc.startDetached() == 0)
        {
            KMessageBox::error(0, QString(i18n("There was an error. Please verify Akregator is installed on your system.")
                                          + "<br /><br /> <a href=\"" + url + "\">" + url + "</a>"));
        }
    }
}
