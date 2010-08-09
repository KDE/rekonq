/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2008-2010 by Andrea Diamantini <adjam7 at gmail dot com>
* Copyright (C) 2009-2010 by Lionel Chauvin <megabigbug@yahoo.fr>
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
#include "mainview.h"

// KDE Includes
#include <KIcon>
#include <KStandardAction>
#include <KAction>

// Qt Includes
#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>
#include <QtCore/QString>


ZoomBar::ZoomBar(QWidget *parent)
        : QWidget(parent)
        ,m_zoomIn(new QToolButton(this))
        ,m_zoomOut(new QToolButton(this))
        ,m_zoomNormal(new QToolButton(this))
        ,m_zoomSlider(new QSlider(Qt::Horizontal, this))
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

    m_zoomSlider->setTracking(true);
    m_zoomSlider->setRange(1, 19);      // divide by 10 to obtain a qreal for zoomFactor()
    m_zoomSlider->setValue(10);
    m_zoomSlider->setPageStep(3);
    connect(m_zoomSlider, SIGNAL(valueChanged(int)), this, SLOT(setValue(int)));

    m_zoomIn->setAutoRaise(true);
    m_zoomOut->setAutoRaise(true);
    m_zoomNormal->setAutoRaise(true);

    layout->setSpacing(0);
    layout->setMargin(0);
    layout->addWidget(m_zoomOut);
    layout->addWidget(m_zoomSlider);
    layout->addWidget(m_zoomIn);
    layout->addWidget(m_zoomNormal);

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
    a = window->actionCollection()->addAction(KStandardAction::Zoom, this, SLOT(show()));
    a->setIcon(KIcon("page-zoom"));
    a->setShortcut(KShortcut(Qt::CTRL | Qt::Key_Y));

    m_zoomIn->setDefaultAction(window->actionByName(KStandardAction::name(KStandardAction::ZoomIn)));
    m_zoomOut->setDefaultAction(window->actionByName(KStandardAction::name(KStandardAction::ZoomOut)));
    m_zoomNormal->setDefaultAction(window->actionByName(KStandardAction::name(KStandardAction::ActualSize)));
}


ZoomBar::~ZoomBar()
{
    delete m_zoomIn;
    delete m_zoomOut;
    delete m_zoomNormal;
    delete m_zoomSlider;
}


void ZoomBar::show()
{
    // show findbar if not visible
    if (isHidden())
    {
        QWidget::show();
    }
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
    if (!Application::instance()->mainWindowList().isEmpty())
          tab = Application::instance()->mainWindow()->mainView()->webTab(webview);

    if (!tab)
        return;

    m_zoomSlider->setValue(tab->view()->zoomFactor() * 10);
}


void ZoomBar::setValue(int value)
{
    m_zoomSlider->setValue(value);
    Application::instance()->mainWindow()->currentTab()->view()->setZoomFactor(QVariant(value).toReal() / 10);
}
