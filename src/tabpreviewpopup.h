/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2011 by Vyacheslav Blinov <blinov dot vyacheslav at gmail dot com>
* Copyright (C) 2011-2012 by Andrea Diamantini <adjam7 at gmail dot com>
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


#ifndef TABPREVIEWPOPUP_H
#define TABPREVIEWPOPUP_H

// Rekonq Includes
#include "rekonq_defines.h"

// KDE Includes
#include <KPassivePopup>

// forward declatrations
class WebTab;
class QLabel;


class REKONQ_TESTS_EXPORT TabPreviewPopup : public KPassivePopup
{

public:
    /**
    * @brief This constructs a new Tab Preview Popup witch will create a thumbnail and title with url from WebTab object
    *
    * @param tab a WebTab object witch will be used to create a preview
    * @param parent
    **/
    explicit TabPreviewPopup(WebTab *tab, QWidget *parent = 0);
    virtual ~TabPreviewPopup();

    QSize thumbnailSize() const;

    static const int previewBaseSize = 200;

private:
    void setWebTab(WebTab *tab = 0);
    void setUrl(const QString& text);
    void setThumbnail(const QPixmap& pixmap);
    void setFixedSize(int w, int h);

protected:
    void paintEvent(QPaintEvent *event);

private:
    QLabel *m_thumbnail;
    QLabel *m_url;
};

#endif // TABPREVIEWPOPUP_H
