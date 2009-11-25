/**
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
 */

#include "clicktoflash.h"

#include <KLocalizedString>

#include <QFile>
#include <QMenu>
#include <QWebFrame>
#include <QWebView>
#include <QWebElement>
#include <QHBoxLayout>
#include <QContextMenuEvent>

#include <KMenu>
#include <KDebug>

ClickToFlash::ClickToFlash(QUrl pluginUrl, QWidget *parent)
    : QWidget(parent)
    , m_url(pluginUrl)
{
    
    kDebug() << "creating clicktoflash";
    QHBoxLayout *l = new QHBoxLayout(this);
    setLayout(l);
    
    QToolButton *button = new QToolButton(this);
    button->setPopupMode(QToolButton::InstantPopup);
    button->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    button->setText(i18n("Load animation"));
    button->setAutoRaise(false);
    layout()->addWidget(button);
    connect(button, SIGNAL(clicked(bool)), this, SLOT(load()));
}


void ClickToFlash::load()
{
    //bool loadAll = true;
    
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
                kDebug() << "called";
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


