/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2010 by Matthieu Gicquel <matgic78 at gmail dot com>
* Copyright (C) 2010 by Andrea Diamantini <adjam7 at gmail dot com>
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
#include "previewselectorbar.h"
#include "previewselectorbar.moc"

// Local Include
#include "rekonq.h"
#include "websnap.h"
#include "application.h"
#include "mainwindow.h"
#include "webtab.h"

// KDE Includes
#include <KIcon>
#include <KLocalizedString>

// Qt Includes
#include <QToolButton>
#include <QHBoxLayout>
#include <QString>


PreviewSelectorBar::PreviewSelectorBar(int index, QWidget* parent)
        : QWidget(parent)
        , m_button(0)
        , m_label(0)
        , m_previewIndex(index)
{
    m_label = new QLabel(i18n("Please open up the webpage you want to add as favorite"), this);
    m_label->setWordWrap(true);

    QToolButton *closeButton = new QToolButton(this);
    closeButton->setAutoRaise(true);
    closeButton->setIcon(KIcon("dialog-close"));
    connect(closeButton, SIGNAL(clicked(bool)), this, SLOT(destroy()));

    m_button = new QPushButton(KIcon("insert-image"), i18n("Set to This Page"), this);
    m_button->setMaximumWidth(250);
    connect(m_button, SIGNAL(clicked(bool)), this, SLOT(clicked()));

    // layout
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->addWidget(closeButton);
    layout->addWidget(m_label);
    layout->addWidget(m_button);

    layout->setContentsMargins(2, 0, 2, 0);

    setLayout(layout);
}


PreviewSelectorBar::~PreviewSelectorBar()
{
}


void PreviewSelectorBar::verifyUrl()
{

    if (Application::instance()->mainWindow()->currentTab()->page()->mainFrame()->url().scheme() != "about")
    {
        m_button->setEnabled(true);
        m_button->setToolTip("");
    }
    else
    {
        m_button->setEnabled(false);
        m_button->setToolTip(i18n("You can not add this webpage as favorite"));
    }
}


void PreviewSelectorBar::loadProgress()
{
    m_button->setEnabled(false);
    m_button->setToolTip(i18n("Page is loading..."));
}


void PreviewSelectorBar::loadFinished()
{
    m_button->setEnabled(true);
    m_button->setToolTip("");

    verifyUrl();
}


void PreviewSelectorBar::clicked()
{
    WebPage *page = Application::instance()->mainWindow()->currentTab()->page();

    if (page)
    {
        // this is done just lo let the render process being faster..
        WebSnap::renderPreview(*page);

        KUrl url = page->mainFrame()->url();
        QStringList names = ReKonfig::previewNames();
        QStringList urls = ReKonfig::previewUrls();

        urls.replace(m_previewIndex, url.toMimeDataString());
        names.replace(m_previewIndex, page->mainFrame()->title());

        ReKonfig::setPreviewNames(names);
        ReKonfig::setPreviewUrls(urls);

        ReKonfig::self()->writeConfig();


        page->mainFrame()->load(KUrl("about:favorites"));
    }

    destroy();
}


void PreviewSelectorBar::destroy()
{
    if (parentWidget() && parentWidget()->layout())
    {
        parentWidget()->layout()->removeWidget(this);
    }
    this->deleteLater();
}
