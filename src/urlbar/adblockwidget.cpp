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
#include "adblockwidget.h"

// Auto Includes
#include "rekonq.h"

// KDE Includes
#include <KLocalizedString>

// Qt Includes
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QIcon>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>


AdBlockWidget::AdBlockWidget(const QUrl &url, QWidget *parent)
    : QMenu(parent)
    , _pageUrl(url)
    , _chBox(new QCheckBox(this))
    , _isAdblockEnabledHere(true)
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

    QStringList hList = ReKonfig::whiteReferer();
    const QString urlHost = _pageUrl.host();
    Q_FOREACH(const QString & host, hList)
    {
        if (host.contains(urlHost))
        {
            _isAdblockEnabledHere = false;
            break;
        }
    }

    // Checkbox
    _chBox->setText(i18n("Enable AdBlock for this site"));
    _chBox->setChecked(_isAdblockEnabledHere);

    layout->addWidget(title);
    layout->addWidget(_chBox);

    // Ok & Cancel buttons
    QDialogButtonBox *buttonBox = new QDialogButtonBox(this, Qt::Horizontal);

    buttonBox->addButton(KStandardGuiItem::ok(), QDialogButtonBox::AcceptRole, this, SLOT(accept()));
    buttonBox->addButton(KStandardGuiItem::cancel(), QDialogButtonBox::RejectRole, this, SLOT(close()));

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
        QStringList hosts = ReKonfig::whiteReferer();

        if (on)
        {
            qDebug() << "REMOVING IT...";
            hosts.removeOne(_pageUrl.host());
        }
        else
        {
            hosts << _pageUrl.host();
        }

        ReKonfig::setWhiteReferer(hosts);

        emit updateIcon();
    }
    close();
}
