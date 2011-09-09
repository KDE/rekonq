/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2011 by Andrea Diamantini <adjam7 at gmail dot com>
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
#include "favoritewidget.h"
#include "favoritewidget.moc"

// Auto Includes
#include "rekonq.h"

// Local includes
#include "application.h"
#include "bookmarkprovider.h"
#include "bookmarkowner.h"

// KDE Includes
#include <KLocalizedString>
#include <KIcon>
#include <KLineEdit>

// Qt Includes
#include <QtGui/QDialogButtonBox>
#include <QtGui/QFormLayout>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>


FavoriteWidget::FavoriteWidget(WebTab *tab, QWidget *parent)
    : QMenu(parent)
    , m_tab(tab)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setFixedWidth(350);

    QFormLayout *layout = new QFormLayout(this);
    QVBoxLayout *vLay = new QVBoxLayout(this);

    // Favorite icon
    QLabel *bookmarkIcon = new QLabel(this);
    bookmarkIcon->setPixmap(KIcon("emblem-favorite").pixmap(32, 32));

    // Title
    QLabel *favoriteInfo = new QLabel(this);
    favoriteInfo->setText(i18n("<h4>Remove this favorite?</h4>"));
    vLay->addWidget(favoriteInfo);

    // Favorite name
    QLabel *nameLabel = new QLabel(this);
    nameLabel->setText(i18n("Name: %1", m_tab->view()->title()));
    vLay->addWidget(nameLabel);

    // Favorite url
    QLabel *urlLabel = new QLabel(this);
    urlLabel->setText(i18n("URL: %1", m_tab->url().url()));
    vLay->addWidget(urlLabel);

    layout->addRow(bookmarkIcon, vLay);

    // Ok & Cancel buttons
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(close()));
    layout->addWidget(buttonBox);

    setLayout(layout);
}


void FavoriteWidget::showAt(const QPoint &pos)
{
    adjustSize();

    QPoint p(pos.x() - width(), pos.y() + 10);
    move(p);
    show();
}


void FavoriteWidget::accept()
{
    QStringList urls = ReKonfig::previewUrls();
    if (urls.removeOne(m_tab->url().url()))
    {
        ReKonfig::setPreviewUrls(urls);
        QStringList titles = ReKonfig::previewNames();
        titles.removeOne(m_tab->view()->title());
        ReKonfig::setPreviewNames(titles);

        emit updateIcon();
    }

    close();
}
