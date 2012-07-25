/***************************************************************************
 *   Copyright (C) 2012 by Andrea Diamantini <adjam7@gmail.com>                            *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/


#include "webwindow.h"
#include "webwindow.moc"

#include "webpage.h"
#include "websnap.h"

#include <QUrl>
#include <QLineEdit>
#include <QWebView>
#include <QVBoxLayout>


WebWindow::WebWindow(QWidget *parent)
    : QWidget(parent)
    , _view(new QWebView(this))
    , _edit(new QLineEdit(this))
{
    WebPage *p = new WebPage(_view);
    _view->setPage(p);

    // layout
    QVBoxLayout *l = new QVBoxLayout;
    l->addWidget(_edit);
    l->addWidget(_view);
    l->setContentsMargins(0, 0, 0, 0);
    setLayout(l);

    setContentsMargins(0, 0, 0, 0);

    // line edit signals
    connect(_edit, SIGNAL(returnPressed()), this, SLOT(checkLoadUrl()));

    // url signal
    connect(_view, SIGNAL(urlChanged(QUrl)), this, SLOT(setUrlText(QUrl)));

    // things changed signals
    connect(_view, SIGNAL(titleChanged(QString)), this, SIGNAL(titleChanged(QString)));

    // load signals
    connect(_view, SIGNAL(loadStarted()), this, SIGNAL(loadStarted()));
    connect(_view, SIGNAL(loadProgress(int)), this, SIGNAL(loadProgress(int)));
    connect(_view, SIGNAL(loadFinished(bool)), this, SIGNAL(loadFinished(bool)));

    // page signals
    connect(p, SIGNAL(pageCreated(WebPage *)), this, SIGNAL(pageCreated(WebPage *)));
}


WebWindow::WebWindow(WebPage *page, QWidget *parent)
    : QWidget(parent)
    , _view(new QWebView(this))
    , _edit(new QLineEdit(this))
{
    _view->setPage(page);
    page->setParent(_view);

    // layout
    QVBoxLayout *l = new QVBoxLayout;
    l->addWidget(_edit);
    l->addWidget(_view);
    l->setContentsMargins(0, 0, 0, 0);
    setLayout(l);

    setContentsMargins(0, 0, 0, 0);

    // line edit signals
    connect(_edit, SIGNAL(returnPressed()), this, SLOT(checkLoadUrl()));

    // url signal
    connect(_view, SIGNAL(urlChanged(QUrl)), this, SLOT(setUrlText(QUrl)));

    // things changed signals
    connect(_view, SIGNAL(titleChanged(QString)), this, SIGNAL(titleChanged(QString)));

    // load signals
    connect(_view, SIGNAL(loadStarted()), this, SIGNAL(loadStarted()));
    connect(_view, SIGNAL(loadProgress(int)), this, SIGNAL(loadProgress(int)));
    connect(_view, SIGNAL(loadFinished(bool)), this, SIGNAL(loadFinished(bool)));

    // page signals
    connect(page, SIGNAL(pageCreated(WebPage *)), this, SIGNAL(pageCreated(WebPage *)));

}


void WebWindow::load(const QUrl &url)
{
    _view->load(url);
}


WebPage *WebWindow::page()
{
    if (!_view)
        return 0;

    WebPage *p = qobject_cast<WebPage *>(_view->page());
    return p;
}


void WebWindow::checkLoadUrl()
{
    QString urlString = _edit->text();
    QUrl u = QUrl::fromUserInput(urlString);
    load(u);
}


void WebWindow::setUrlText(const QUrl &u)
{
    _edit->setText(u.toString());
}


QUrl WebWindow::url() const
{
    return _view->url();
}


QString WebWindow::title() const
{
    return _view->title();
}


QIcon WebWindow::icon() const
{
    return _view->icon();
}


QPixmap WebWindow::tabPreview(int width, int height)
{
    return WebSnap::renderPagePreview(*page(), width, height);
}
