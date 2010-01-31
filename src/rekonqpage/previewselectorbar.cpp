/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) <year>  <name of author>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/


// Auto Includes
#include "previewselectorbar.h"

// Local Include
#include "rekonq.h"
#include "websnap.h"

// KDE Includes
#include <KIcon>
#include <KLocalizedString>

// Qt Includes
#include <QToolButton>
#include <QHBoxLayout>
#include <QString>


PreviewSelectorBar::PreviewSelectorBar(QWidget* parent)
        : QWidget(parent)
        , m_button(0)
        , m_label(0)
        , m_page(0)
{
    hide();
}


void PreviewSelectorBar::setup()
{
    if(m_button != 0)
        return;
    
    m_label = new QLabel(i18n("Please go to the page you want to preview"), this);
    m_label->setWordWrap(true);
    
    QToolButton *closeButton = new QToolButton(this);
    closeButton->setAutoRaise(true);
    closeButton->setIcon(KIcon("dialog-close"));
    connect(closeButton, SIGNAL(clicked(bool)), SLOT(hide()));
    
    m_button = new QPushButton(KIcon("insert-image"), i18n("Set to this page"), this);
    m_button->setMaximumWidth(250);
    connect(m_button, SIGNAL(clicked(bool)), SLOT(clicked()));
    
    // layout
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->addWidget(closeButton);
    layout->addWidget(m_label);
    layout->addWidget(m_button);
    
    layout->setContentsMargins(2, 0, 2, 0);
    
    setLayout(layout);
}


void PreviewSelectorBar::setPage(WebPage* page)
{
    m_page = page;
    verifyUrl();
}


void PreviewSelectorBar::verifyUrl()
{
    if(m_page->mainFrame()->url().scheme() != "about")
    {
        m_button->setEnabled(true);
        m_button->setToolTip("");
    }
    else
    {
        m_button->setEnabled(false);
        m_button->setToolTip(i18n("You can't set this page as preview"));
    }
}


void PreviewSelectorBar::enable(int previewIndex, WebPage* page)
{
    if(m_page != 0)
        disconnect(m_page, 0, this, 0);

    
    setup();
    m_previewIndex = previewIndex;
    m_page = page;
    
    verifyUrl();
    
    show();
    
    connect(page, SIGNAL(loadStarted()), SLOT(loadProgress()));
    connect(page, SIGNAL(loadProgress(int)), SLOT(loadProgress()));
    connect(page, SIGNAL(loadFinished(bool)), SLOT(loadFinished()));
    connect(page->mainFrame(), SIGNAL(urlChanged(QUrl)), SLOT(verifyUrl()));
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
    KUrl url = m_page->mainFrame()->url();

    WebSnap::savePreview(WebSnap::renderPreview(*m_page), url);
    
    QStringList names = ReKonfig::previewNames();
    QStringList urls = ReKonfig::previewUrls();
    
    urls.replace(m_previewIndex, url.toMimeDataString());
    names.replace(m_previewIndex, m_page->mainFrame()->title());
    
    ReKonfig::setPreviewNames(names);
    ReKonfig::setPreviewUrls(urls);
    
    ReKonfig::self()->writeConfig();
    
    
    m_page->mainFrame()->load(KUrl("about:favorites"));
    
    hide();
}


