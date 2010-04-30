/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2009 by Benjamin C. Meyer <ben@meyerhome.net>
* Copyright (C) 2010 by Matthieu Gicquel <matgic78@gmail.com>
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
#include "clicktoflash.h"
#include "clicktoflash.moc"

// KDE Includes
#include <KLocalizedString>
#include <KDebug>

// Qt Includes
#include <QWebFrame>
#include <QWebView>
#include <QWebElement>
#include <QHBoxLayout>
#include <QToolButton>



ClickToFlash::ClickToFlash(QUrl pluginUrl, QWidget *parent)
        : QWidget(parent)
        , m_url(pluginUrl)
{
    QHBoxLayout *l = new QHBoxLayout(this);
    setLayout(l);

    QToolButton *button = new QToolButton(this);
    button->setPopupMode(QToolButton::InstantPopup);
    button->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    button->setText(i18n("Load Plugin"));
    button->setAutoRaise(false);
    layout()->addWidget(button);
    connect(button, SIGNAL(clicked(bool)), this, SLOT(load()));
}


void ClickToFlash::load()
{
    QWidget *parent = parentWidget();
    QWebView *view = 0;
    while (parent)
    {
        if (QWebView *aView = qobject_cast<QWebView*>(parent))
        {
            view = aView;
            break;
        }
        parent = parent->parentWidget();
    }
    if (!view)
        return;

    const QString selector = QLatin1String("%1[type=\"application/x-shockwave-flash\"]");

    hide();

    QList<QWebFrame*> frames;
    frames.append(view->page()->mainFrame());
    while (!frames.isEmpty())
    {
        QWebFrame *frame = frames.takeFirst();
        QWebElement docElement = frame->documentElement();

        QWebElementCollection elements;
        elements.append(docElement.findAll(selector.arg(QLatin1String("object"))));
        elements.append(docElement.findAll(selector.arg(QLatin1String("embed"))));

        foreach(QWebElement element, elements)
        {
            if (checkElement(element))
            {
                kDebug() << "RETURNED TRUE ...........................";
                QWebElement substitute = element.clone();
                emit signalLoadClickToFlash(true);
                element.replace(substitute);
                deleteLater();
                return;
            }
        }
        frames += frame->childFrames();
    }
}


bool ClickToFlash::checkElement(QWebElement el)
{
    kDebug() << "src: " << QUrl(el.attribute("src"));
    kDebug() << "url: " << m_url;

    QString checkString;
    QString urlString;

    checkString = QUrl(el.attribute("src")).toString(QUrl::RemoveQuery);
    urlString = m_url.toString(QUrl::RemoveQuery);

    if (urlString.contains(checkString))
        return true;

    QWebElementCollection collec = el.findAll("*");
    int i = 0;
    while (i < collec.count())
    {
        QWebElement el = collec.at(i);

        checkString = QUrl(el.attribute("src")).toString(QUrl::RemoveQuery);
        urlString = m_url.toString(QUrl::RemoveQuery);

        if (urlString.contains(checkString))
            return true;

        i++;
    }

    return false;
}
