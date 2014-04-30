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
#include "webappshortcutdialog.h"

// Auto Includes
#include "rekonq.h"

// Local Includes
#include "application.h"
#include "iconmanager.h"
#include "rekonqwindow.h"
#include "webwindow.h"

// KDE Includes
#include <KLocalizedString>

// Qt Includes
#include <QPushButton>
#include <QStandardPaths>


WebAppShortcutDialog::WebAppShortcutDialog(const QString & urlString, const QString & titleString, QWidget *parent)
    : QDialog(parent)
    , _buttonBox(new QDialogButtonBox(this))
    , _urlString(urlString)
    , _titleString(titleString)
{
    // icon url
    if (_urlString.isEmpty())
    {
        _webAppIconUrl = rApp->rekonqWindow()->currentWebWindow()->url();
    }
    else
    {
        _webAppIconUrl = QUrl(_urlString);
    }
    QString h = _webAppIconUrl.host();

    // title && icon
    setWindowTitle(i18nc("@title:window", "Create Application Shortcut"));
    setWindowIcon(QIcon(IconManager::self()->iconForUrl(_webAppIconUrl).pixmap(16)));

    // buttonbox
    _buttonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    _buttonBox->button(QDialogButtonBox::Ok)->setText(i18n("Create"));

    connect(_buttonBox, SIGNAL(accepted()), this, SLOT(createShortcut()));
    connect(_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    // the session widget
    QWidget *widget = new QWidget(this);
    _webAppWidget.setupUi(widget);
    
    // insert everything inside the dialog...
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(widget);
    mainLayout->addWidget(_buttonBox);
    setLayout(mainLayout);

    setMinimumSize(400, 50);

    // Ok, now the fine grained settings...
    if (_titleString.isEmpty())
    {
        _webAppTitle = rApp->rekonqWindow()->currentWebWindow()->title();
    }
    else
    {
        _webAppTitle = _titleString;
    }
    _webAppTitle = _webAppTitle.remove(QL1C('&'));
    
    _webAppWidget.nameLineEdit->setText(_webAppTitle);
    _webAppWidget.kcfg_createDesktopAppShortcut->setChecked(ReKonfig::createDesktopAppShortcut());
    _webAppWidget.kcfg_createMenuAppShortcut->setChecked(ReKonfig::createMenuAppShortcut());
}


void WebAppShortcutDialog::createShortcut()
{
    ReKonfig::setCreateDesktopAppShortcut(_webAppWidget.kcfg_createDesktopAppShortcut->isChecked());
    ReKonfig::setCreateMenuAppShortcut(_webAppWidget.kcfg_createMenuAppShortcut->isChecked());

    IconManager::self()->saveDesktopIconForUrl(_webAppIconUrl);
    QString iconPath = QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + QL1S("/favicons/") + _webAppIconUrl.host() + QL1S("_WEBAPPICON.png");

    if (!_webAppWidget.nameLineEdit->text().isEmpty())
        _webAppTitle = _webAppWidget.nameLineEdit->text();

    QString webAppDescription;
    if (!_webAppWidget.descriptionLineEdit->text().isEmpty())
        webAppDescription = _webAppWidget.descriptionLineEdit->text();

    QString shortcutString = QL1S("#!/usr/bin/env xdg-open\n")
                                + QL1S("[Desktop Entry]\n")
                                + QL1S("Name=") + _webAppTitle
                                + QL1S("\n")
                                + QL1S("GenericName=") + webAppDescription
                                + QL1S("\n")
                                + QL1S("Icon=") + iconPath + QL1S("\n")
                                + QL1S("Exec=rekonq --webapp ") + _webAppIconUrl.url() + QL1S("\n")
                                + QL1S("Type=Application\n")
                                + QL1S("Categories=Application;Network\n")
                                ;

    if (ReKonfig::createDesktopAppShortcut())
    {
        QString desktop = QStandardPaths::displayName(QStandardPaths::DesktopLocation);
        QFile wAppFile(desktop + QL1C('/') + _webAppTitle);

        if (!wAppFile.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            qDebug() << "Unable to open file: " << wAppFile.errorString();
            return;
        }

        QTextStream out(&wAppFile);
        out.setCodec("UTF-8");
        out << shortcutString;

        wAppFile.setPermissions(QFile::ReadUser | QFile::WriteUser | QFile::ExeUser | QFile::ReadGroup | QFile::ReadOther);
        wAppFile.close();
    }

    if (ReKonfig::createMenuAppShortcut())
    {
        QString appMenuDir = QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation);
        QFile wAppFile(appMenuDir + QL1C('/') + _webAppTitle + QL1S(".desktop"));

        if (!wAppFile.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            qDebug() << "Unable to open file: " << wAppFile.errorString();
            return;
        }

        QTextStream out(&wAppFile);
        out.setCodec("UTF-8");
        out << shortcutString;

        wAppFile.close();
    }

    // and now accept!
    close();
}
