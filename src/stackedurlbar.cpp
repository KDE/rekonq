/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2009 by Paweł Prażak <pawelprazak at gmail dot com>
*
*
* This program is free software; you can redistribute it
* and/or modify it under the terms of the GNU General
* Public License as published by the Free Software Foundation;
* either version 3, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* ============================================================ */


// Self Includes
#include "stackedurlbar.h"
#include "stackedurlbar.moc"

// Local Includes
#include "application.h"
#include "history.h"
#include "urlbar.h"

// KDE Includes
#include <KDebug>


StackedUrlBar::StackedUrlBar(QWidget *parent)
        : QStackedWidget(parent)
        , m_completion(0)
        , m_completionModel(0)
{
}


StackedUrlBar::~StackedUrlBar()
{
    delete m_completion;
    delete m_completionModel;
}


UrlBar *StackedUrlBar::currentUrlBar()
{
    return urlBar(currentIndex());
}


UrlBar *StackedUrlBar::urlBar(int index)
{
    UrlBar *urlBar = qobject_cast<UrlBar*>(QStackedWidget::widget(index));
    if (!urlBar)
    {
        kWarning() << "URL bar with index" << index << "not found. Returning NULL.  line:" << __LINE__;
    }

    return urlBar;
}


void StackedUrlBar::addUrlBar(UrlBar* urlBar)
{
    QStackedWidget::addWidget(urlBar);

    // setup completion objects
    urlBar->setCompletionObject(completion());
}


void StackedUrlBar::setCurrentUrlBar(UrlBar* urlBar)
{
    QStackedWidget::setCurrentWidget(urlBar);
}


void StackedUrlBar::removeUrlBar(UrlBar* urlBar)
{
    QStackedWidget::removeWidget(urlBar);
}


void StackedUrlBar::clear()
{
    currentUrlBar()->clearHistory();

    for (int i = 0; i < count(); ++i)
    {
        urlBar(i)->clear();
    }
}


QList<const UrlBar* > StackedUrlBar::urlBars()
{
    QList<const UrlBar *> list;
    for (int i = 0; i < count(); ++i)
    {
        const UrlBar* u = urlBar(i);
        list.append(u);
    }
    return list;
}


KCompletion *StackedUrlBar::completion()
{
    // make sure completion was created
    if (!m_completion)
    {
        m_completion = new KCompletion();
        m_completion->setCompletionMode(KGlobalSettings::CompletionPopupAuto);
        m_completion->setOrder(KCompletion::Weighted);
        m_completion->setIgnoreCase(true);

        kDebug() << "Initialize completion list...";

        HistoryCompletionModel *model = completionModel();
        int count = model->rowCount();

        kDebug() << "...initialize history items" << count;

        // change order to insertion to avoid confusion of the addItem method
        // in weighted it expects format string:number and it thinks http it the whole string
        m_completion->setOrder(KCompletion::Insertion);
        for (int i = 0; i < count; ++i)
        {
            QString item = model->data(model->index(i, 0)).toString();
            item.remove(QRegExp("^http://|/$"));
            m_completion->addItem(item);
        }
        m_completion->setOrder(KCompletion::Weighted);
    }

    return m_completion;
}


HistoryCompletionModel *StackedUrlBar::completionModel()
{
    if (!m_completionModel)
    {
        m_completionModel = new HistoryCompletionModel(this);
        m_completionModel->setSourceModel(Application::historyManager()->historyFilterModel());
    }
    return m_completionModel;
}
