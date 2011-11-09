/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2008-2011 by Andrea Diamantini <adjam7 at gmail dot com>
* Copyright (C) 2009-2011 by Lionel Chauvin <megabigbug@yahoo.fr>
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
#include "zoombar.h"
#include "zoombar.moc"

// local includes
#include "application.h"
#include "mainview.h"
#include "mainwindow.h"
#include "webtab.h"

// KDE Includes
#include <KAction>
#include <KIcon>
#include <KLocalizedString>
#include <KStandardAction>
#include <KConfig>
#include <KConfigGroup>

// Qt Includes
#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QSlider>
#include <QtGui/QToolButton>


ZoomBar::ZoomBar(QWidget *parent)
    : QWidget(parent)
    , m_zoomIn(new QToolButton(this))
    , m_zoomOut(new QToolButton(this))
    , m_zoomNormal(new QToolButton(this))
    , m_zoomSlider(new QSlider(Qt::Horizontal, this))
{
    QHBoxLayout *layout = new QHBoxLayout;

    // cosmetic
    layout->setContentsMargins(2, 0, 2, 0);

    QToolButton *hideButton = new QToolButton(this);
    hideButton->setAutoRaise(true);
    hideButton->setIcon(KIcon("dialog-close"));
    connect(hideButton, SIGNAL(clicked()), this, SLOT(hide()));

    layout->addWidget(hideButton);
    layout->setAlignment(hideButton, Qt::AlignLeft | Qt::AlignTop);

    // label
    QLabel *label = new QLabel(i18n("Zoom:"));
    layout->addWidget(label);

    //Show the current zoom percentage of the page
    m_percentage = new QLabel(i18nc("percentage of the website zoom", "100%"), this);

    m_zoomSlider->setTracking(true);
    m_zoomSlider->setRange(1, 19);      // divide by 10 to obtain a qreal for zoomFactor()
    m_zoomSlider->setValue(10);
    m_zoomSlider->setPageStep(3);
    connect(m_zoomSlider, SIGNAL(valueChanged(int)), this, SLOT(setValue(int)));

    m_zoomIn->setAutoRaise(true);
    m_zoomOut->setAutoRaise(true);
    m_zoomNormal->setAutoRaise(true);

    layout->addWidget(m_zoomOut);
    layout->addWidget(m_zoomSlider, 8);
    layout->addWidget(m_zoomIn);
    layout->addWidget(m_zoomNormal);
    layout->addWidget(m_percentage, 5);

    layout->addStretch();

    setLayout(layout);

    // we start off hidden
    hide();
}


void ZoomBar::setupActions(MainWindow *window)
{
    KAction *a;
    a = window->actionCollection()->addAction(KStandardAction::ZoomIn, this, SLOT(zoomIn()));
    a = window->actionCollection()->addAction(KStandardAction::ZoomOut, this, SLOT(zoomOut()));
    a = window->actionCollection()->addAction(KStandardAction::ActualSize, this, SLOT(zoomNormal()));
    a->setShortcut(KShortcut(Qt::CTRL | Qt::Key_0));
    a = window->actionCollection()->addAction(KStandardAction::Zoom, this, SLOT(toggleVisibility()));
    a->setIcon(KIcon("page-zoom"));
    a->setShortcut(KShortcut(Qt::CTRL | Qt::Key_Y));

    m_zoomIn->setDefaultAction(window->actionByName(KStandardAction::name(KStandardAction::ZoomIn)));
    m_zoomOut->setDefaultAction(window->actionByName(KStandardAction::name(KStandardAction::ZoomOut)));
    m_zoomNormal->setDefaultAction(window->actionByName(KStandardAction::name(KStandardAction::ActualSize)));
}


void ZoomBar::show()
{
    // show findbar if not visible
    if (isHidden())
    {
        emit visibilityChanged(true);
        QWidget::show();
        m_zoomSlider->setValue(rApp->mainWindow()->currentTab()->view()->zoomFactor() * 10);
    }
}


void ZoomBar::hide()
{
    emit visibilityChanged(false);
    QWidget::hide();
}


void ZoomBar::zoomIn()
{
    setValue(m_zoomSlider->value() + 1);
}


void ZoomBar::zoomOut()
{
    setValue(m_zoomSlider->value() - 1);
}


void ZoomBar::zoomNormal()
{
    setValue(10);
}


void ZoomBar::updateSlider(int webview)
{
    WebTab *tab = 0;
    if (!rApp->mainWindowList().isEmpty())
        tab = rApp->mainWindow()->mainView()->webTab(webview);

    if (!tab)
        return;

    m_zoomSlider->setValue(tab->view()->zoomFactor() * 10);
    connect(tab->view(), SIGNAL(zoomChanged(int)), this, SLOT(setValue(int)));
}


void ZoomBar::setValue(int value)
{
    m_zoomSlider->setValue(value);
    m_percentage->setText(i18nc("percentage of the website zoom", "%1%", QString::number(value * 10)));

    WebTab *tab = rApp->mainWindow()->currentTab();
    saveZoomValue(tab->url().host(), value);
    tab->view()->setZoomFactor(QVariant(value).toReal() / 10);  // Don't allox max +1 values
}


void ZoomBar::toggleVisibility()
{
    isVisible() ? hide() : show();
}


void ZoomBar::saveZoomValue(const QString &host, int value)
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group(config, "Zoom");
    group.writeEntry(host, QString::number(value));
    config->sync();
}
