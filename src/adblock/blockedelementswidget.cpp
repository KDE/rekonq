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


// Self Includes
#include "blockedelementswidget.h"
#include "blockedelementswidget.moc"

// Local Includes
#include "adblockmanager.h"

// Qt Includes
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>


BlockedElementsWidget::BlockedElementsWidget(QObject *manager, QWidget *parent)
    : QWidget(parent)
    , _manager(manager)
    , _reloadPage(false)
{
    setupUi(this);
}


void BlockedElementsWidget::setBlockedElements(const QStringList &list)
{
    QVBoxLayout *frameLayout = new QVBoxLayout(frame);

    Q_FOREACH(const QString & block, list)
    {
        QString truncatedUrl = block;
        const int maxTextSize = 73;
        if (truncatedUrl.length() > maxTextSize)
        {
            const int truncateSize = 70;
            truncatedUrl.truncate(truncateSize);
            truncatedUrl += QL1S("...");
        }
        QWidget *w = new QWidget(this);
        QHBoxLayout *l = new QHBoxLayout(w);
        l->addWidget(new QLabel(truncatedUrl, this));

        QPushButton *button = new QPushButton(KIcon("dialog-ok-apply"), i18n("Unblock"), this);
        button->setProperty("URLTOUNBLOCK", block);
        button->setFixedWidth(100);
        connect(button, SIGNAL(clicked()), this, SLOT(unblockElement()));
        l->addWidget(button);

        w->setMinimumWidth(500);
        frameLayout->addWidget(w);
    }
}


void BlockedElementsWidget::setHidedElements(int n)
{
    AdBlockManager *m = qobject_cast<AdBlockManager *>(_manager);
    if (m->isHidingElements())
        label->setText(i18np("There is %1 hidden element in this page.", "There are %1 hidden elements in this page.", QString::number(n)));
    else
        label->setText(i18n("Hiding elements is disabled."));
}


void BlockedElementsWidget::unblockElement()
{
    QPushButton *buttonClicked = qobject_cast<QPushButton *>(sender());
    if (!buttonClicked)
        return;

    QString urlString = QL1S("@@") + buttonClicked->property("URLTOUNBLOCK").toString();
    kDebug() << "urlString: " << urlString;

    QString newText = i18n("Unblocked");
    QString buttonText = buttonClicked->text().remove('&');
    if (buttonText == newText)
    {
        buttonClicked->setText(i18n("Unblock"));
        buttonClicked->setIcon(KIcon("dialog-ok-apply"));

        _rulesToAdd.removeOne(urlString);
    }
    else
    {
        buttonClicked->setText(newText);
        buttonClicked->setIcon(KIcon("dialog-ok"));

        _rulesToAdd << urlString;
    }

    _reloadPage = true;
}
