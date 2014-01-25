/* ============================================================
*
* This is a part of the GSoC project 2011 - Fancy Bookmarking
*
* Copyright (c) 2011-2012 by Phaneendra Hegde <pnh.pes@gmail.com>
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
#include "resourcelinkdialog.h"

// Local Includes
#include "newresourcedialog.h"

// Qt Includes
#include <QGridLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QAbstractItemView>
#include <QLabel>
#include <QListView>
#include <QPushButton>
#include <QColumnView>
#include <QStringListModel>
#include <QStandardItem>
#include <QModelIndexList>
#include <QItemSelectionModel>
#include <QMenu>
#include <QListWidget>

// KDE Includes
#include <KLocale>
#include <KDebug>
#include <KAction>
#include <KIcon>

// Nepomuk Includes
#include <Nepomuk2/Query/Term>
#include <Nepomuk2/Query/Result>
#include <Nepomuk2/Query/ResourceTypeTerm>
#include <Nepomuk2/Query/QueryServiceClient>
#include <Nepomuk2/Vocabulary/PIMO>
#include <Nepomuk2/Vocabulary/NCO>
#include <Nepomuk2/Query/QueryParser>
#include <Nepomuk2/Variant>

// Nepomuk Ported Classes
#include "nepomuk/utils/simpleresourcemodel.h"

// Soprano Includes
#include <Soprano/Vocabulary/NAO>

class Nepomuk2::ResourceLinkDialog::Private
{
public:
    void _k_selectionChanged();

    KLineEdit *m_searchBox;
    QListView *m_resourceView;
    QListView *m_linkedResources;
    KAction *m_removeResourceAction;
    QComboBox *m_resourceSelect;
    QLabel *m_resourceLabel;
    QLabel *m_linkedResourceLabel;
    QColumnView *m_leftPanel;
    QStringListModel *m_model;
    QPushButton *m_newResourceButton;
    Utils::SimpleResourceModel *m_resourceModel;
    Utils::SimpleResourceModel *m_linkedResourceModel;
    Nepomuk2::ResourceLinkDialog *q;

    Nepomuk2::Resource m_nfoResource;

};


void Nepomuk2::ResourceLinkDialog::Private::_k_selectionChanged()
{
    q->enableButton(KDialog::User1, !m_resourceView->selectionModel()->selectedRows().isEmpty());
}


Nepomuk2::ResourceLinkDialog::ResourceLinkDialog(Nepomuk2::Resource &nfoResource, QWidget* parent):
    KDialog(parent),
    d(new Private())
{
    d->m_nfoResource = nfoResource;
    setWindowTitle(i18n("Resource Linker"));
    setButtons(Ok | User1 | User2 | Cancel);
    enableButtonCancel(true);
    enableButtonOk(true);
    enableButton(User1, false);
    setButtonText(Ok, i18n("Done"));
    setButtonText(User1, i18n("Link"));
    setButtonText(User2, i18n("Unlink"));
    setMinimumSize(400, 350);
//    d->m_resourceView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    QGridLayout *mainLayout = new QGridLayout(mainWidget());

    d->q = this;

    d->m_linkedResources = new QListView(mainWidget());
    d->m_linkedResourceModel = new Utils::SimpleResourceModel(this);
    d->m_linkedResources->setModel(d->m_linkedResourceModel);
    setRelatedResources();

    d->m_searchBox = new KLineEdit(mainWidget());
    d->m_searchBox->setPlaceholderText(i18n("Search resources"));
    connect(d->m_searchBox, SIGNAL(textChanged(QString)), this, SLOT(dynamicSearchingSlot()));

    d->m_resourceView = new QListView(mainWidget());
    d->m_resourceView->setToolTip(i18n("Double click to link resource"));
    d->m_resourceModel = new Utils::SimpleResourceModel(this);
    d->m_resourceView->setModel(d->m_resourceModel);

    d->m_resourceSelect = new QComboBox(mainWidget());
    QStringList rlist;
    rlist << i18n("Any resource") << i18n("Persons") << i18n("Projects") << i18n("Tasks") << i18n("Places") << i18n("Notes");
    d->m_resourceSelect->addItems(rlist);
    d->m_resourceSelect->setItemIcon(1, KIcon("user-identity"));
    d->m_resourceSelect->setItemIcon(2, KIcon("project-development"));
    d->m_resourceSelect->setItemIcon(3, KIcon("view-pim-tasks"));
    d->m_resourceSelect->setItemIcon(4, KIcon("user-location"));
    d->m_resourceSelect->setItemIcon(5, KIcon("knotes"));
    connect(d->m_resourceSelect, SIGNAL(currentIndexChanged(int)), this, SLOT(resourceSelectedSlot(int)));

    d->m_resourceLabel = new QLabel(i18n("Matching resources:"), mainWidget());
    d->m_linkedResourceLabel = new QLabel(i18n("Linked Resources:"), mainWidget());


    d->m_newResourceButton = new QPushButton(mainWidget());
    d->m_newResourceButton->setText(i18n("Create New Resource"));
    if (d->m_resourceSelect->currentIndex() == 0)
    {
        d->m_newResourceButton->setEnabled(false);
    }
    connect(d->m_newResourceButton, SIGNAL(clicked()), this, SLOT(createNewResourceSlot()));

    QVBoxLayout *vlayoutR = new QVBoxLayout;
    QVBoxLayout *vlayoutL = new QVBoxLayout;
    vlayoutL->addWidget(d->m_searchBox);
    vlayoutL->addWidget(d->m_resourceLabel);
    vlayoutL->addWidget(d->m_resourceView);
    vlayoutR->addWidget(d->m_resourceSelect);
    vlayoutR->addWidget(d->m_linkedResourceLabel);
    vlayoutR->addWidget(d->m_linkedResources);
    vlayoutR->addWidget(d->m_newResourceButton);
    mainLayout->addLayout(vlayoutL, 1 , 1);
    mainLayout->addLayout(vlayoutR, 1, 2);
    mainLayout->setColumnMinimumWidth(1, 100);

    d->m_linkedResources->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(d->m_resourceView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            this, SLOT(_k_selectionChanged()));
    connect(d->m_linkedResources->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            this, SLOT(_k_selectionChanged()));
    connect(this, SIGNAL(user1Clicked()), this, SLOT(linkResourceSlot()));
    connect(this, SIGNAL(user2Clicked()), this, SLOT(unlinkResourceSlot()));
    connect(d->m_resourceView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(linkResourceSlot()));
    connect(d->m_linkedResources, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showContextMenu(QPoint)));

    if (!d->m_linkedResources->selectionModel()->selectedRows().isEmpty())
    {
        enableButton(User2, true);
    }
}


Nepomuk2::ResourceLinkDialog::~ResourceLinkDialog()
{
    delete d;
}


void Nepomuk2::ResourceLinkDialog::setRelatedResources()
{
    QList<Nepomuk2::Resource> relatedResourceList = d->m_nfoResource.isRelateds();
    d->m_linkedResourceModel->setResources(relatedResourceList);

}


void Nepomuk2::ResourceLinkDialog::linkResourceSlot()
{
    QModelIndexList selectedResourceList;
    selectedResourceList << d->m_resourceView->selectionModel()->selectedIndexes();
    Q_FOREACH(const QModelIndex & i, selectedResourceList)
    {
        d->m_resourceView->selectionModel()->setCurrentIndex(i, QItemSelectionModel::NoUpdate);
        d->m_nfoResource.addIsRelated(d->m_resourceModel->resourceForIndex(d->m_resourceView->selectionModel()->currentIndex()));
    }
    setRelatedResources();
}


void Nepomuk2::ResourceLinkDialog::unlinkResourceSlot()
{
    d->m_nfoResource.removeProperty(Soprano::Vocabulary::NAO::isRelated().toString(),
                                    d->m_linkedResourceModel->resourceForIndex(
                                        d->m_linkedResources->selectionModel()->currentIndex()));
    setRelatedResources();
}


void Nepomuk2::ResourceLinkDialog::showContextMenu(const QPoint &pos)
{
    d->m_removeResourceAction = new KAction(this);
    d->m_removeResourceAction->setText(i18n("&Unlink"));
    d->m_removeResourceAction->setIcon(KIcon("edit-delete"));
    connect(d->m_removeResourceAction, SIGNAL(triggered(bool)), this, SLOT(unlinkResourceSlot()));

    QMenu myMenu;
    QPoint globalPos = d->m_linkedResources->mapToGlobal(pos);
    myMenu.addAction(d->m_removeResourceAction);
    myMenu.exec(globalPos);
}


void Nepomuk2::ResourceLinkDialog::createNewResourceSlot()
{
    QPointer<Nepomuk2::NewResourceDialog> r = new Nepomuk2::NewResourceDialog(d->m_resourceSelect->currentIndex(), d->m_nfoResource);
    r->exec();

    setRelatedResources();

    r->deleteLater();
}


void Nepomuk2::ResourceLinkDialog::dynamicSearchingSlot()
{
    Nepomuk2::Query::Query query;
    Nepomuk2::Query::QueryServiceClient *test;
    switch (d->m_resourceSelect->currentIndex())
    {
    case 1:
        query =  Nepomuk2::Query::QueryParser::parseQuery(d->m_searchBox->text());
        query = query && Nepomuk2::Query::ResourceTypeTerm(Nepomuk2::Vocabulary::PIMO::Person());
        test = new Nepomuk2::Query::QueryServiceClient(this);
        test->query(query);
        d->m_resourceModel->clear();
        connect(test, SIGNAL(newEntries(QList<Nepomuk2::Query::Result>)),
                d->m_resourceModel, SLOT(addResults(QList<Nepomuk2::Query::Result>)));
        break;

    case 2:
        query =  Nepomuk2::Query::QueryParser::parseQuery(d->m_searchBox->text());
        query = query && Nepomuk2::Query::ResourceTypeTerm(Nepomuk2::Vocabulary::PIMO::Project());
        test = new Nepomuk2::Query::QueryServiceClient(this);
        test->query(query);
        d->m_resourceModel->clear();
        connect(test, SIGNAL(newEntries(QList<Nepomuk2::Query::Result>)),
                d->m_resourceModel, SLOT(addResults(QList<Nepomuk2::Query::Result>)));
        break;

    case 3:
        query = Nepomuk2::Query::QueryParser::parseQuery(d->m_searchBox->text());
        query = query && Nepomuk2::Query::ResourceTypeTerm(Nepomuk2::Vocabulary::PIMO::Task());
        test = new Nepomuk2::Query::QueryServiceClient(this);
        test->query(query);
        d->m_resourceModel->clear();
        connect(test, SIGNAL(newEntries(QList<Nepomuk2::Query::Result>)),
                d->m_resourceModel, SLOT(addResults(QList<Nepomuk2::Query::Result>)));
        break;

    case 4:
        query = Nepomuk2::Query::QueryParser::parseQuery(d->m_searchBox->text());
        query = query && Nepomuk2::Query::ResourceTypeTerm(Nepomuk2::Vocabulary::PIMO::Location());
        test = new Nepomuk2::Query::QueryServiceClient(this);
        test->query(query);
        d->m_resourceModel->clear();
        connect(test, SIGNAL(newEntries(QList<Nepomuk2::Query::Result>)),
                d->m_resourceModel, SLOT(addResults(QList<Nepomuk2::Query::Result>)));
        break;

    case 5:
        query =  Nepomuk2::Query::QueryParser::parseQuery(d->m_searchBox->text());
        query = query && Nepomuk2::Query::ResourceTypeTerm(Nepomuk2::Vocabulary::PIMO::Note());
        test = new Nepomuk2::Query::QueryServiceClient(this);
        test->query(query);
        d->m_resourceModel->clear();
        connect(test, SIGNAL(newEntries(QList<Nepomuk2::Query::Result>)),
                d->m_resourceModel, SLOT(addResults(QList<Nepomuk2::Query::Result>)));
        break;

    default:
        break;
    }
}


void Nepomuk2::ResourceLinkDialog::resourceSelectedSlot(int index)
{
    enableButton(User1, true);
    d->m_newResourceButton->setEnabled(true);
    if (index == 0)
    {
        d->m_resourceModel->clear();
        d->m_newResourceButton->setEnabled(false);
    }
    //List Personal Contacts
    if (index == 1)
    {
        Nepomuk2::Query::Term term = Nepomuk2::Query::ResourceTypeTerm(Nepomuk2::Vocabulary::PIMO::Person());
        Nepomuk2::Query::Query query(term);
        query.setLimit(20);
        QList<Nepomuk2::Query::Result>results = Nepomuk2::Query::QueryServiceClient::syncQuery(query);
        QList <Nepomuk2::Resource> resource;
        Q_FOREACH(const Nepomuk2::Query::Result & result, results)
        {
            resource.append(result.resource());
        }
        d->m_resourceModel->setResources(resource);
    }
    //List Projects
    else if (index == 2)
    {
        Nepomuk2::Query::Term term = Nepomuk2::Query::ResourceTypeTerm(Nepomuk2::Vocabulary::PIMO::Project());
        Nepomuk2::Query::Query query(term);
        query.setLimit(20);
        QList<Nepomuk2::Query::Result>results = Nepomuk2::Query::QueryServiceClient::syncQuery(query);
        QList <Nepomuk2::Resource> resource;
        Q_FOREACH(const Nepomuk2::Query::Result & result, results)
        {
            resource.append(result.resource());
        }
        d->m_resourceModel->setResources(resource);
    }
    //List Tasks
    else if (index == 3)
    {
        Nepomuk2::Query::Term term = Nepomuk2::Query::ResourceTypeTerm(Nepomuk2::Vocabulary::PIMO::Task());
        Nepomuk2::Query::Query query(term);
        query.setLimit(20);
        QList<Nepomuk2::Query::Result>results = Nepomuk2::Query::QueryServiceClient::syncQuery(query);
        QList <Nepomuk2::Resource> resource;
        Q_FOREACH(const Nepomuk2::Query::Result & result, results)
        {
            resource.append(result.resource());
        }
        d->m_resourceModel->setResources(resource);
    }
    //List Places
    else if (index == 4)
    {
        Nepomuk2::Query::Term term = Nepomuk2::Query::ResourceTypeTerm(Nepomuk2::Vocabulary::PIMO::Location());
        Nepomuk2::Query::Query query(term);
        query.setLimit(20);
        QList<Nepomuk2::Query::Result>results = Nepomuk2::Query::QueryServiceClient::syncQuery(query);
        QList <Nepomuk2::Resource> resource;
        Q_FOREACH(const Nepomuk2::Query::Result & result, results)
        {
            resource.append(result.resource());
        }
        d->m_resourceModel->setResources(resource);
    }
    //List Notes
    else if (index == 5)
    {
        Nepomuk2::Query::Term term = Nepomuk2::Query::ResourceTypeTerm(Nepomuk2::Vocabulary::PIMO::Note());
        Nepomuk2::Query::Query query(term);
        query.setLimit(20);
        QList<Nepomuk2::Query::Result>results = Nepomuk2::Query::QueryServiceClient::syncQuery(query);
        QList <Nepomuk2::Resource> resource;
        Q_FOREACH(const Nepomuk2::Query::Result & result, results)
        {
            resource.append(result.resource());
        }
        d->m_resourceModel->setResources(resource);
    }
}
