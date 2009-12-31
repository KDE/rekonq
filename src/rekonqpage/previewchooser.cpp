/* ============================================================
*
* This file is a part of the rekonq project
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
#include "previewchooser.h"

// Local Includes
#include "rekonq.h"
#include <bookmarkstreemodel.h>
#include <bookmarksproxy.h>
#include <mainview.h>
#include <webtab.h>

// Qt Includes
#include <QLabel>
#include <QVBoxLayout>

// KDE Includes
#include <KLineEdit>
#include <KLocalizedString>


PreviewChooser::PreviewChooser(int previewIndex, QString url)
        : KDialog(0)
        , m_treeView(new QTreeView)
        , m_model(new QStandardItemModel)
        , m_previewIndex(previewIndex)
{
    setMinimumSize(350, 100);
    setWindowTitle(i18n("Set Page to preview"));
    setModal(true);
    
    setButtons(KDialog::Cancel | KDialog::Apply | KDialog::Ok);
    setDefaultButton(KDialog::Ok);
    connect(this, SIGNAL(buttonClicked(KDialog::ButtonCode)), this, SLOT(buttonClicked(KDialog::ButtonCode)));
    
    m_treeView->setUniformRowHeights(true);
    m_treeView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_treeView->setTextElideMode(Qt::ElideRight);
    m_treeView->setHeaderHidden(true);
    m_treeView->setIndentation(0);
    
    QWidget *mainWidget = new QWidget(this);

    // add url bar
    QHBoxLayout *urlLayout = new QHBoxLayout;
    urlLayout->setContentsMargins(5, 0, 0, 0);
    QLabel *searchLabel = new QLabel(i18n("adress:"));
    urlLayout->addWidget(searchLabel);
    m_line = new KLineEdit;
    connect(m_line, SIGNAL(textChanged(QString)), SLOT(urlChanged()));
    urlLayout->addWidget(m_line);
    
    if(url.isEmpty() || url.startsWith("about:"))
       m_line->setText(QString("http://"));
    else
    {
        m_line->setText(url);
        m_line->setSelection(8, m_line->text().size());
    }
    

    // setup view
    QVBoxLayout *vBoxLayout = new QVBoxLayout;
    vBoxLayout->setContentsMargins(0, 0, 0, 0);
    vBoxLayout->addLayout(urlLayout);
    vBoxLayout->addWidget(m_treeView);
    mainWidget->setLayout(vBoxLayout);
    setMainWidget(mainWidget);
    
    refreshModel();
    connect(Application::instance()->mainWindow()->mainView(), SIGNAL(tabsChanged()), SLOT(refreshModel()));
    
    m_treeView->setModel(m_model);
    
    connect(m_treeView, SIGNAL(activated(QModelIndex)), SLOT(setUrl(QModelIndex)));
}


PreviewChooser::~PreviewChooser()
{
    delete m_model;
}



void PreviewChooser::refreshModel()
{
    m_model->clear();
    MainView *mv = Application::instance()->mainWindow()->mainView();
    for(int i=0; i < mv->count(); ++i)
    {
        WebView *view = qobject_cast<WebView *>(mv->webTab(i)->view());
        
        if(view->url().scheme() == "about")
            continue;
        
        QStandardItem *it = new QStandardItem(Application::icon(view->url()), view->title());
        it->setData(QVariant(view->url()), Qt::ToolTipRole);
        m_model->insertRow(i, it);
    }
}

void PreviewChooser::setUrl(QModelIndex i)
{
    if(!i.data().canConvert(QVariant::String))
        return;
    
    m_line->setText(i.data(Qt::ToolTipRole).toString());
}


void PreviewChooser::buttonClicked(KDialog::ButtonCode code)
{
    if(code == KDialog::Apply || (code == KDialog::Ok && isButtonEnabled(KDialog::Apply)))
    {
        emit urlChoosed(m_previewIndex, KUrl(m_line->text()));
        enableButtonApply(false);
    }
    
    if(code == KDialog::Cancel || code == KDialog::Ok)
    {
        close();
        deleteLater();
    }
}


void PreviewChooser::urlChanged()
{
    enableButtonApply(true);
}

