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

#include <KDebug>

ClickToFlash::ClickToFlash(const WebPluginFactory *factory, QUrl pluginUrl, QWidget *parent)
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
    connect(button, SIGNAL(clicked(bool)), this, SLOT(load(bool)));
}

/*void ClickToFlash::showContextMenu()
{
    QMenu menu;
    menu.addAction(tr("Load"), this, SLOT(load()));
    menu.addAction(tr("Load All"), this, SLOT(loadAll()));
    menu.addSeparator();
    QString host = url.host();
    QAction *add = menu.addAction(tr("Add %1 to Whitelist").arg(host), this, SLOT(addToWhitelist()));
    QAction *remove = menu.addAction(tr("Remove from Whitelist"), this, SLOT(removeFromWhitelist()));
    bool onWhitelist = m_plugin->onWhitelist(host);
    add->setEnabled(!onWhitelist);
    remove->setEnabled(onWhitelist);
    menu.addSeparator();
    menu.addAction(tr("Settings"), this, SLOT(configure()));
    menu.exec(QCursor::pos());
}

void ClickToFlash::addToWhitelist()
{
    m_plugin->addToWhitelist(url.host());
}

void ClickToFlash::removeFromWhitelist()
{
    m_plugin->removeFromWhitelist(url.host());
}

void ClickToFlash::configure()
{
    m_plugin->configure();
}

void ClickToFlash::loadAll()
{
    load(true);
}
*/
void ClickToFlash::load(bool loadAll)
{
    kDebug() << "called";
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

        QWebElement element;
        kDebug() << m_url;
        bool isRightElement = false;
        foreach (element, elements) 
        {
            // TODO : find a proper solution to compare a QWebElement with a plugin
            if(QUrl(element.attribute("data")) == m_url
            || QUrl(element.attribute("src")) == m_url)
                isRightElement = true;
            
            QWebElementCollection collec = element.findAll("param");
            int i = 0;
            while(i < collec.count() && isRightElement == false)
            {
                if(QUrl(collec.at(i).attribute("value")) == m_url)
                    isRightElement = true;
                i++;
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
    
    //deleteLater();
}


