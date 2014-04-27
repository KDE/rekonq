/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2010 by Matthieu Gicquel <matgic78 at gmail dot com>
* Copyright (C) 2010-2014 by Andrea Diamantini <adjam7 at gmail dot com>
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
#include "previewselectorbar.h"

// Auto Includes
#include "rekonq.h"

// Local Include
#include "webpage.h"
#include "webtab.h"
#include "websnap.h"

// KDE Includes
#include <KLocalizedString>

// Qt Includes
#include <QAction>
#include <QIcon>


PreviewSelectorBar::PreviewSelectorBar(int index, QWidget* parent)
    : KMessageWidget(parent)
    , m_previewIndex(index)
    , m_insertAction(0)
{
    setMessageType(KMessageWidget::Information);

    QSize sz = size();
    sz.setWidth(qobject_cast<QWidget *>(parent)->size().width());
    resize(sz);

    setCloseButtonVisible(false);

    setText(i18n("Please open up the webpage you want to add as favorite"));

    m_insertAction = new QAction(QIcon::fromTheme( QL1S("insert-image") ), i18n("Set to This Page"), this);
    connect(m_insertAction, SIGNAL(triggered(bool)), this, SLOT(clicked()));
    addAction(m_insertAction);
}


void PreviewSelectorBar::verifyUrl()
{
    WebTab *tab = qobject_cast<WebTab *>(parent());
    if (tab->url().scheme() != QL1S("rekonq"))
    {
        m_insertAction->setEnabled(true);
        m_insertAction->setToolTip( QL1S("") );
    }
    else
    {
        m_insertAction->setEnabled(false);
        m_insertAction->setToolTip(i18n("You cannot add this webpage as favorite"));
    }
}


void PreviewSelectorBar::loadProgress()
{
    m_insertAction->setEnabled(false);
    m_insertAction->setToolTip(i18n("Page is loading..."));
}


void PreviewSelectorBar::loadFinished()
{
    m_insertAction->setEnabled(true);
    m_insertAction->setToolTip( QL1S("") );

    verifyUrl();
}


void PreviewSelectorBar::clicked()
{
    WebTab *tab = qobject_cast<WebTab *>(parent());

    if (tab->page())
    {
        QUrl url = tab->url();
        QStringList names = ReKonfig::previewNames();
        QStringList urls = ReKonfig::previewUrls();

        //cleanup the previous image from the cache (useful to refresh the snapshot)
        QUrl uToR = QUrl(urls.at(m_previewIndex));
        QFile::remove(WebSnap::imagePathFromUrl(uToR));
        QPixmap preview = WebSnap::renderPagePreview(*tab->page());
        preview.save(WebSnap::imagePathFromUrl(url));

        urls.replace(m_previewIndex, url.toString());
        names.replace(m_previewIndex, tab->page()->mainFrame()->title());

        ReKonfig::setPreviewNames(names);
        ReKonfig::setPreviewUrls(urls);

        ReKonfig::self()->save();


        tab->page()->mainFrame()->load(QUrl( QL1S("rekonq:favorites") ));
    }

    animatedHide();
    deleteLater();
}
