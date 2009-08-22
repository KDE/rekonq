/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2009 by Andrea Diamantini <adjam7 at gmail dot com>
* Copyright (C) 2009 by Paweł Prażak <pawelprazak at gmail dot com>
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
