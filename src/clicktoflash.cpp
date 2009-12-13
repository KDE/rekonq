/*
 * Copyright (c) 2009, Benjamin C. Meyer
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Benjamin Meyer nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * ============================================================
 *
 * This file is a part of the rekonq project
 *
 * Copyright (C) 2009 by Matthieu Gicquel <matgic78@gmail.com>
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


#include "clicktoflash.h"

#include <QWebFrame>
#include <QWebView>
#include <QWebElement>
#include <QHBoxLayout>
#include <QToolButton>

#include <KLocalizedString>


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
        
        bool isRightElement = false;
        foreach (QWebElement element, elements) 
        {
            // TODO : find a proper solution to compare a QWebElement with a plugin
            // With this "manual" test, it's probably not working everywhere
            if(QUrl(element.attribute("data")) == m_url
                || QUrl(element.attribute("src")) == m_url)
                isRightElement = true;
            else
            {
                QWebElementCollection collec = element.findAll("param");
                int i = 0;
                while(i < collec.count() && isRightElement == false)
                {
                    if(QUrl(collec.at(i).attribute("value")) == m_url)
                    isRightElement = true;
                    i++;
                }
            }
            
            if(isRightElement)
            {
                QWebElement substitute = element.clone();
                emit signalLoadClickToFlash(true);
                element.replace(substitute);
                return;
            }
        }

        frames += frame->childFrames();
    }
    
    deleteLater();
}


