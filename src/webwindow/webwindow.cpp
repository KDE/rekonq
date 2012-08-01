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


#include "webwindow.h"
#include "webwindow.moc"

#include "webpage.h"
#include "webtab.h"

#include "websnap.h"

#include <KUrl>
#include <KLineEdit>

#include <QWebView>
#include <QVBoxLayout>


WebWindow::WebWindow(QWidget *parent)
    : QWidget(parent)
    , _progress(0)
    , _tab(new WebTab(this))
    , _edit(new KLineEdit(this))
{
    init();
}


WebWindow::WebWindow(WebPage *page, QWidget *parent)
    : QWidget(parent)
    , _tab(new WebTab(this))
    , _edit(new KLineEdit(this))
{
    _tab->view()->setPage(page);
    
    init();
}


void WebWindow::init()
{
    // layout
    QVBoxLayout *l = new QVBoxLayout;
    l->addWidget(_edit);
    l->addWidget(_tab);
    l->setContentsMargins(0, 0, 0, 0);
    setLayout(l);

    setContentsMargins(0, 0, 0, 0);

    // line edit signals
    connect(_edit, SIGNAL(returnPressed()), this, SLOT(checkLoadUrl()));

    // url signal
    connect(_tab->view(), SIGNAL(urlChanged(QUrl)), this, SLOT(setUrlText(QUrl)));

    // things changed signals
    connect(_tab->view(), SIGNAL(titleChanged(QString)), this, SIGNAL(titleChanged(QString)));

    // load signals
    connect(_tab->view(), SIGNAL(loadStarted()), this, SIGNAL(loadStarted()));
    connect(_tab->view(), SIGNAL(loadFinished(bool)), this, SIGNAL(loadFinished(bool)));

    connect(_tab->view(), SIGNAL(loadProgress(int)), this, SLOT(checkLoadProgress(int)));

    // page signals
    connect(page(), SIGNAL(pageCreated(WebPage *)), this, SIGNAL(pageCreated(WebPage *)));
}


void WebWindow::load(const QUrl &url)
{
    _tab->view()->load(url);
}


WebPage *WebWindow::page()
{
    return _tab->page();
}


void WebWindow::checkLoadUrl()
{
    QString urlString = _edit->text();
    QUrl u = QUrl::fromUserInput(urlString);
    load(u);
}


void WebWindow::checkLoadProgress(int p)
{
    _progress = p;
    emit loadProgress(p);
}

void WebWindow::setUrlText(const QUrl &u)
{
    _edit->setText(u.toString());
}

KUrl WebWindow::url() const
{
    return _tab->url();
}


QString WebWindow::title() const
{
    return _tab->view()->title();
}


QIcon WebWindow::icon() const
{
    return _tab->view()->icon();
}


QPixmap WebWindow::tabPreview(int width, int height)
{
    return WebSnap::renderPagePreview(*page(), width, height);
}


bool WebWindow::isLoading()
{
    return _progress != 0 && _progress != 100;
}
