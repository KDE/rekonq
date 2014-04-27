/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2012-2014 by Andrea Diamantini <adjam7 at gmail dot com>
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
#include "cleardatadialog.h"

// Auto Includes
#include "rekonq.h"

// Local Manager(s) Includes
#include "downloadmanager.h"
#include "historymanager.h"
#include "iconmanager.h"

// Qt Includes
#include <QDBusInterface>
#include <QDBusReply>
#include <QDir>
#include <QPushButton>
#include <QStandardPaths>

// KDE Includes
#include <KProcess>


ClearDataDialog::ClearDataDialog(QWidget *parent)
    : QDialog(parent)
{
    // the title
    setWindowTitle(i18nc("@title:window", "Clear Private Data"));

    // the button box
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    
    buttonBox->button(QDialogButtonBox::Ok)->setIcon(QIcon::fromTheme(QL1S("edit-clear")));
    buttonBox->button(QDialogButtonBox::Ok)->setText(i18n("Clear"));

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(acceptIt()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    
    // the main widget
    QWidget *widget = new QWidget(this);
    clearWidget.setupUi(widget);
    
    clearWidget.clearHistory->setChecked(ReKonfig::clearHistory());
    clearWidget.clearDownloads->setChecked(ReKonfig::clearDownloads());
    clearWidget.clearCookies->setChecked(ReKonfig::clearCookies());
    clearWidget.clearCachedPages->setChecked(ReKonfig::clearCachedPages());
    clearWidget.clearWebIcons->setChecked(ReKonfig::clearWebIcons());
    clearWidget.homePageThumbs->setChecked(ReKonfig::clearHomePageThumbs());

    // insert everything inside the dialog...
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(widget);
    mainLayout->addWidget(buttonBox);
    setLayout(mainLayout);
}


void ClearDataDialog::acceptIt()
{
    //Save current state
    ReKonfig::setClearHistory(clearWidget.clearHistory->isChecked());
    ReKonfig::setClearDownloads(clearWidget.clearDownloads->isChecked());
    ReKonfig::setClearCookies(clearWidget.clearDownloads->isChecked());
    ReKonfig::setClearCachedPages(clearWidget.clearCachedPages->isChecked());
    ReKonfig::setClearWebIcons(clearWidget.clearWebIcons->isChecked());
    ReKonfig::setClearHomePageThumbs(clearWidget.homePageThumbs->isChecked());

    if (clearWidget.clearHistory->isChecked())
    {
        HistoryManager::self()->clear();
    }

    if (clearWidget.clearDownloads->isChecked())
    {
        DownloadManager::self()->clearDownloadsHistory();
    }

    if (clearWidget.clearCookies->isChecked())
    {
        QDBusInterface kcookiejar(QL1S("org.kde.kded"), QL1S("/modules/kcookiejar"), QL1S("org.kde.KCookieServer"));
        QDBusReply<void> reply = kcookiejar.call(QL1S("deleteAllCookies"));
    }

    if (clearWidget.clearCachedPages->isChecked())
    {
        KProcess::startDetached(QStandardPaths::findExecutable(QL1S("kio_http_cache_cleaner")),
                                    QStringList(QL1S("--clear-all")));
    }

    if (clearWidget.clearWebIcons->isChecked())
    {
        IconManager::self()->clearIconCache();
    }

    if (clearWidget.homePageThumbs->isChecked())
    {
        QString path = QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + QL1S("/thumbs/");
        QDir cacheDir(path);
        QStringList fileList = cacheDir.entryList();
        Q_FOREACH(const QString & str, fileList)
        {
            QFile file(path + str);
            file.remove();
        }
    }
    
    accept();
}
