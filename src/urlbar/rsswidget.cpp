/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2010 by Matthieu Gicquel <matgic78 at gmail dot com>
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
#include "mainwindow.h"
#include "webtab.h"
#include "webview.h"

// KDE Includes
#include <KLocalizedString>
#include <KIcon>
#include <KProcess>
#include <KMessageBox>

// Qt Includes
#include <QtGui/QFormLayout>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>

#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusConnectionInterface>



RSSWidget::RSSWidget(const QMap< KUrl, QString > &map, QWidget *parent)
        : QFrame(parent, Qt::Popup)
        , m_map(map)
{
    setAttribute(Qt::WA_DeleteOnClose);
    
    setMinimumWidth(200);
    setFrameStyle(Panel);

    QFormLayout *layout = new QFormLayout(this);
    setLayout(layout);

    QLabel *title = new QLabel(this);
    title->setText(i18n("<h4>Subscribe to RSS Feeds</h4>"));
    layout->addRow(title);

    // Agregators
    QLabel *agregator = new QLabel(this);
    agregator->setText(i18n("Aggregator:"));

    m_agregators = new KComboBox(this);
    m_agregators->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    
    m_agregators->addItem(KIcon("akregator"), QString("Akregator"));
    m_agregators->addItem(Application::icon(KUrl("http://google.com/reader")), i18n("Google Reader"));

    layout->addRow(agregator, m_agregators);

    // Feeds List
    QLabel *feed = new QLabel(this);
    feed->setText(i18n("Feed:"));

    m_feeds = new KComboBox(this);
    m_feeds->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    foreach(const QString &title, m_map)
    {
        m_feeds->addItem(title);
    }
    
    layout->addRow(feed, m_feeds);

    // Buttons
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Cancel, Qt::Horizontal, this);
    
    QPushButton *addFeed = new QPushButton(KIcon("list-add"), i18n("Add Feed"), buttonBox);
    buttonBox->addButton(addFeed, QDialogButtonBox::AcceptRole);
    
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    layout->addRow(buttonBox);
}


RSSWidget::~RSSWidget()
{
    delete m_agregators;
    delete m_feeds;
}


void RSSWidget::showAt(const QPoint &pos)
{
    QPoint p;
    p.setX(pos.x() - 200);
    p.setY(pos.y() + 10);
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

    reject();
}


void RSSWidget::reject()
{
    close();
    this->deleteLater();
}


void RSSWidget::addWithGoogleReader(const QString &url)
{
    KUrl toLoad = KUrl("http://www.google.com/ig/add?feedurl=" + url);
    Application::instance()->mainWindow()->currentTab()->view()->load(toLoad);
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
