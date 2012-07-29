/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2009-2012 by Andrea Diamantini <adjam7 at gmail dot com>
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


#ifndef WEB_SNAP_H
#define WEB_SNAP_H


// Rekonq Includes
#include "rekonq_defines.h"

// Qt Includes
#include <QObject>
#include <QWebPage>
#include <QUrl>

// Forward Declarations
class QPixmap;

/**
 * This class is used in many classes of rekonq to produce an image
 * based on the site corresponding to the url passed as argument.
 * It also cached the images to not retrieve them every time :)
 *
 * Heavily based on Graphics-Dojo WebSnap example (thanks!)
 *
 * We use this in the following rekonq classes:
 *
 * - TabBar class:          to show a tab preview                (given a page, you show AND save an image)
 * - PreviewSelector class: to save new favorite selection       (given a page, you show AND save an image)
 *
 * - NewTabPage class:      to show the favorites page "preview" (given an url, you show AND save an image)
 *
 */

class WebSnap : public QObject
{
    Q_OBJECT

public:
    /**
     * Creates a WebSnap object. It will load the url in one WebPage
     * and snap an image from it.
     *
     * @param url the url to load
     * @param parent the object parent
     */
    explicit WebSnap(const QUrl &url, QObject *parent = 0);

    ~WebSnap();

    /**
     * Snaps a pixmap of size w * h from a page
     *
     * @param page the page to snap
     * @param w the image width
     * @param h the image height
     *
     * @return the pixmap snapped from the page
     */
    static QPixmap renderPagePreview(const QWebPage &page, int w = defaultWidth, int h = defaultHeight);

    /**
     * Guess the local path where the image for the url provided
     * should be
     *
     * @param url the url to guess snap path
     *
     * @return the local path of the url snap
     */
    static QString imagePathFromUrl(const QUrl &url);

    /**
     * Determines if a snap exists for that url
     *
     */
    static bool existsImage(const QUrl &url);


private Q_SLOTS:
    void saveResult(bool ok = true);
    void load();

Q_SIGNALS:
    void snapDone(bool ok);

private:
    // Constants
    static const int defaultWidth = 200;
    static const int defaultHeight = 150;
    QWebPage m_page;
    QUrl m_url;

    //render a preview: common part of renderPagePreview() and renderTabPreview()
    static QPixmap render(const QWebPage &page, int w, int h);
};

#endif // WEB_SNAP_H
