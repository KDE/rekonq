/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2012 by Andrea Diamantini <adjam7 at gmail dot com>
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
#include "adblockwidget.h"
#include "adblockwidget.moc"

// Local includes
#include "adblockmanager.h"

// KDE Includes
#include <KIcon>
#include <KLocalizedString>

// Qt Includes
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>


AdBlockWidget::AdBlockWidget(const QUrl &url, QWidget *parent)
    : QMenu(parent)
    , _pageUrl(url)
    , _chBox(new QCheckBox(this))
    , _isAdblockEnabledHere(AdBlockManager::self()->isAdblockEnabledForHost(_pageUrl.host()))
{
    setAttribute(Qt::WA_DeleteOnClose);
    setFixedWidth(320);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setSpacing(10);

    // Title
    QLabel *title = new QLabel(this);
    title->setText(i18n(" AdBlock"));
    QFont f = title->font();
    f.setBold(true);
    title->setFont(f);

    // Checkbox
    _chBox->setText(i18n("Enable adblock for this site"));
    _chBox->setChecked(_isAdblockEnabledHere);
    
    layout->addWidget(title);
    layout->addWidget(_chBox);

    // Ok & Cancel buttons
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(close()));
    layout->addWidget(buttonBox);
}


AdBlockWidget::~AdBlockWidget()
{
}


void AdBlockWidget::showAt(const QPoint &pos)
{
    adjustSize();

    QPoint p(pos.x() - width(), pos.y() + 10);
    move(p);
    show();
}


void AdBlockWidget::accept()
{
    bool on = _chBox->isChecked();
    if (on != _isAdblockEnabledHere)
    {
        if (on)
        {
            kDebug() << "REMOVING IT...";
            AdBlockManager::self()->removeCustomHostRule(_pageUrl.host());
        }
        else
        {
            AdBlockManager::self()->addCustomRule(QL1S("@@") + _pageUrl.host());            
        }
        
        emit updateIcon();
    }
    close();
}
