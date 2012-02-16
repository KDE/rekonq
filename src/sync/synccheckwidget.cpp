/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2011 by Andrea Diamantini <adjam7 at gmail dot com>
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
#include "synccheckwidget.h"
#include "synccheckwidget.moc"

// Auto Includes
#include "rekonq.h"

// Local Includes
#include "syncassistant.h"
#include "syncmanager.h"

#include "application.h"

// KDE Includes
#include <KStandardDirs>
#include <KIcon>

// Qt Includes
#include <QMovie>


SyncCheckWidget::SyncCheckWidget(QWidget *parent)
    : QWizardPage(parent)
{
    setupUi(this);
}


void SyncCheckWidget::initializePage()
{
    // set initial values
    if (ReKonfig::syncType() == 0)
    {
        syncLabel->setText(i18n("FTP"));
        hostLabel->setText(ReKonfig::syncHost());
    }
    else
    {
        syncLabel->setText(i18n("No sync"));
        hostLabel->setText(i18n("none"));
    }

    bkMsgLabel->setText(QString());
    hsMsgLabel->setText(QString());
    psMsgLabel->setText(QString());

    KIcon notSyncedIcon(QL1S("dialog-cancel"));

    if (!ReKonfig::syncEnabled())
    {
        bkLabel->setPixmap(notSyncedIcon.pixmap(16));
        hsLabel->setPixmap(notSyncedIcon.pixmap(16));
        psLabel->setPixmap(notSyncedIcon.pixmap(16));
        return;
    }

    QString loadingGitPath = KStandardDirs::locate("appdata" , "pics/loading.mng");

    // bookmarks
    if (ReKonfig::syncBookmarks())
    {
        QMovie *movie = new QMovie(loadingGitPath, QByteArray(), bkLabel);
        movie->setSpeed(50);
        bkLabel->setMovie(movie);
        movie->start();
    }
    else
    {
        bkLabel->setPixmap(notSyncedIcon.pixmap(16));
    }

    // history
    if (ReKonfig::syncHistory())
    {
        QMovie *movie = new QMovie(loadingGitPath, QByteArray(), hsLabel);
        movie->setSpeed(50);
        hsLabel->setMovie(movie);
        movie->start();
    }
    else
    {
        hsLabel->setPixmap(notSyncedIcon.pixmap(16));
    }

    // passwords
    if (ReKonfig::syncPasswords())
    {
        QMovie *movie = new QMovie(loadingGitPath, QByteArray(), psLabel);
        movie->setSpeed(50);
        psLabel->setMovie(movie);
        movie->start();
    }
    else
    {
        psLabel->setPixmap(notSyncedIcon.pixmap(16));
    }

    // Now, load syncManager settings...
    rApp->syncManager()->loadSettings();

    SyncHandler *h = rApp->syncManager()->handler();
    connect(h, SIGNAL(syncStatus(Rekonq::SyncData,bool,QString)), this, SLOT(updateWidget(Rekonq::SyncData,bool,QString)));
}


void SyncCheckWidget::updateWidget(Rekonq::SyncData type, bool done, QString msg)
{
    KIcon doneIcon(QL1S("dialog-ok-apply"));
    KIcon failIcon(QL1S("edit-delete"));

    switch (type)
    {
    case Rekonq::Bookmarks:
        if (done)
        {
            bkLabel->setPixmap(doneIcon.pixmap(16));
        }
        else
        {
            bkLabel->setPixmap(failIcon.pixmap(16));
        }
        bkMsgLabel->setText(msg);
        break;

    case Rekonq::History:
        if (done)
        {
            hsLabel->setPixmap(doneIcon.pixmap(16));
        }
        else
        {
            hsLabel->setPixmap(failIcon.pixmap(16));
        }
        hsMsgLabel->setText(msg);
        break;

    case Rekonq::Passwords:
        if (done)
        {
            psLabel->setPixmap(doneIcon.pixmap(16));
        }
        else
        {
            psLabel->setPixmap(failIcon.pixmap(16));
        }
        psMsgLabel->setText(msg);
        break;

    default:
        // nothing to do here...
        break;
    };
}
