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
#include "thumbupdater.h"

// Local Includes
#include "application.h"
#include "iconmanager.h"
#include "websnap.h"

// KDE Includes
#include <KLocalizedString>

// Qt Includes
#include <QStandardPaths>
#include <QWebFrame>


ThumbUpdater::ThumbUpdater(QWebElement el, const QString & urlString, const QString & nameString, QObject *parent)
    : QObject(parent)
    , _thumb(el)
    , _url(urlString)
    , _title(nameString)
{
}


void ThumbUpdater::updateThumb()
{
    // Set loading animation
    _thumb.findFirst(QL1S(".preview img")).setAttribute(QL1S("src"), QL1S("file:///") + 
        QStandardPaths::locate(QStandardPaths::DataLocation, QL1S("/pics/busywidget.gif")));
    _thumb.findFirst(QL1S("span a")).setPlainText(i18n("Loading Preview..."));

    // Load URL
    QWebFrame *frame = qobject_cast<QWebFrame *>(parent());
    WebSnap *snap = new WebSnap(QUrl(_url), frame);
    connect(snap, SIGNAL(snapDone(bool)), this, SLOT(updateImage(bool)), Qt::UniqueConnection);
}


ThumbUpdater::~ThumbUpdater()
{
    qDebug() << "bye bye";
}


void ThumbUpdater::updateImage(bool ok)
{
    QUrl u(_url);

    QString previewPath = ok
                          ? QL1S("file://") + WebSnap::imagePathFromUrl(u)
                          : IconManager::self()->iconPathForUrl(u)
                          ;

    _thumb.findFirst(QL1S(".preview img")).setAttribute(QL1S("src"), previewPath);
    _thumb.findFirst(QL1S("span a")).setPlainText(_title);

    this->deleteLater();
}
