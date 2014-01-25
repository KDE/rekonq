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
#include "newresourcedialog.h"

// Nepomuk Includes
#include <Nepomuk2/Vocabulary/NCO>
#include <Nepomuk2/Vocabulary/PIMO>
#include <Nepomuk2/Resource>
#include <Nepomuk2/Tag>

// Qt Includes
#include <QPlainTextEdit>
#include <QVBoxLayout>
#include <QLabel>


class Nepomuk2::NewResourceDialog::Private
{
public:
    KLineEdit *m_resourceName;
    QPlainTextEdit *m_description;
    QLabel *m_titleResource;
    QLabel *m_desResource;
    Nepomuk2::NewResourceDialog *q;
    Nepomuk2::Resource m_nofResource;
    int m_index;
};


Nepomuk2::NewResourceDialog::NewResourceDialog(int index, Nepomuk2::Resource& nfoResource, QWidget* parent):
    KDialog(parent),
    d(new Private())
{
    d->q = this;
    d->m_index = index;
    d->m_nofResource = nfoResource;
    setWindowTitle(i18n("Link to new Resource"));
    setButtonText(Ok, i18n("Link"));
    setMinimumSize(200, 150);

    QVBoxLayout *layout = new QVBoxLayout(mainWidget());
    d->m_resourceName = new KLineEdit(mainWidget());
    d->m_titleResource = new QLabel(mainWidget());
    d->m_titleResource->setText(i18n("* Resource Name:"));
    layout->addWidget(d->m_titleResource);
    layout->addWidget(d->m_resourceName);
    d->m_description = new QPlainTextEdit(mainWidget());
    d->m_desResource = new QLabel(mainWidget());
    d->m_desResource->setText(i18n("Description (Optional)"));
    layout->addWidget(d->m_desResource);
    layout->addWidget(d->m_description);

    connect(this, SIGNAL(okClicked()), this, SLOT(newResourceSlot()));
}


Nepomuk2::NewResourceDialog::~NewResourceDialog()
{
    delete d;
}


void Nepomuk2::NewResourceDialog::newResourceSlot()
{
    if (d->m_index == 1)
    {
        Nepomuk2::Resource newResource(d->m_resourceName->text(), Nepomuk2::Vocabulary::PIMO::Person());
        newResource.addSymbol("user-identity");
        d->m_nofResource.addIsRelated(newResource);
    }
    else if (d->m_index == 2)
    {
        Nepomuk2::Resource newResource(d->m_resourceName->text(), Nepomuk2::Vocabulary::PIMO::Project());
        newResource.addSymbol("project-development");
        d->m_nofResource.addIsRelated(newResource);
    }
    else if (d->m_index == 3)
    {
        Nepomuk2::Resource newResource(d->m_resourceName->text(), Nepomuk2::Vocabulary::PIMO::Task());
        newResource.addSymbol("view-pim-tasks");
        d->m_nofResource.addIsRelated(newResource);
    }
    else if (d->m_index == 4)
    {
        Nepomuk2::Resource newResource(d->m_resourceName->text(), Nepomuk2::Vocabulary::PIMO::Location());
        newResource.addSymbol("user-location");
        d->m_nofResource.addIsRelated(newResource);
    }
    else if (d->m_index == 5)
    {
        Nepomuk2::Resource newResource(d->m_resourceName->text(), Nepomuk2::Vocabulary::PIMO::Note());
        newResource.addSymbol("knotes");
        d->m_nofResource.addIsRelated(newResource);
    }
}
