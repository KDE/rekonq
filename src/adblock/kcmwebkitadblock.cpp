/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2009 by Andrea Diamantini <adjam7 at gmail dot com>
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
#include "kcmwebkitadblock.h"
#include "kcmwebkitadblock.moc"

// KDE Includes
#include <KDE/KPluginFactory>
#include <KDE/KPluginLoader>
#include <KDE/KAboutData>
#include <KDE/KTemporaryFile>
#include <KDE/KIO/NetAccess>
#include <KDE/KDebug>

// Qt Includes
#include <QtCore/QTextStream>
#include <QtGui/QWhatsThis>
#include <QtGui/QListWidgetItem>


K_PLUGIN_FACTORY(RekonqPluginFactory,
                    registerPlugin<KCMWebkitAdblock>("webkitAdblock");
                )
                
K_EXPORT_PLUGIN(RekonqPluginFactory("kcmrekonqfactory"))


KCMWebkitAdblock::KCMWebkitAdblock(QWidget *parent, const QVariantList &args)
    : KCModule(KGlobal::mainComponent(), parent, args)
    , _group("adblock")
{
    KAboutData *about = new KAboutData( I18N_NOOP("kcmrekonqfactory"), 0, 
                                        ki18n( "rekonq Browsing Control Module" ), 0, 
                                        KLocalizedString(), KAboutData::License_GPL, 
                                        ki18n( "(c) 2009 Andrea Diamantini" ) );
    
    about->addAuthor( ki18n("Andrea Diamantini"), KLocalizedString(), "adjam7@gmail.com" );
    setAboutData( about );
    
    setupUi(this);
    connect(label, SIGNAL(linkActivated(const QString &)), SLOT(infoLinkActivated(const QString &)) );
    searchLine->setListWidget(listWidget);
    
    connect(addButton,SIGNAL(clicked()),this,SLOT(addExpr()));
    connect(removeButton, SIGNAL(clicked()), this, SLOT(removeSelected()));
    connect(importButton, SIGNAL(clicked()), this, SLOT(importExpr()));

    _config = KSharedConfig::openConfig("webkitrc", KConfig::NoGlobals);
}


KCMWebkitAdblock::~KCMWebkitAdblock()
{
}


void KCMWebkitAdblock::defaults()
{
    searchLine->clear();
    lineEdit->clear();
    listWidget->clear();
    groupBox->setChecked(false);
}


void KCMWebkitAdblock::load()
{
    KConfigGroup cg(_config, _group);
    groupBox->setChecked( cg.readEntry("Enabled", false) );
        
    int num = cg.readEntry("Count", 0);
    for (int i = 0; i < num; ++i)
    {
        QString key = "Filter-" + QString::number(i);
        QString filter = cg.readEntry( key, QString() );
        listWidget->addItem(filter);
    }
}


void KCMWebkitAdblock::save()
{
    KConfigGroup cg(_config, _group);
    cg.deleteGroup();
    cg = KConfigGroup(_config, _group);

    cg.writeEntry("Enabled", groupBox->isChecked());

    for(int i = 0; i < listWidget->count(); ++i )
    {
        QString key = "Filter-" + QString::number(i);
        cg.writeEntry(key, listWidget->item(i)->text());
    }
    cg.writeEntry("Count", listWidget->count());
    cg.sync();
}


void KCMWebkitAdblock::infoLinkActivated(const QString &url)
{
    QString helpString = i18n("<qt><p>Enter an expression to filter. Filters can be defined as either:"
        "<ul><li>a shell-style wildcard, e.g. <tt>http://www.example.com/ads*</tt>, the wildcards <tt>*?[]</tt> may be used</li>"
        "<li>a full regular expression by surrounding the string with '<tt>/</tt>', e.g. <tt>/\\/(ad|banner)\\./</tt></li></ul>"
        "<p>Any filter string can be preceded by '<tt>@@</tt>' to whitelist (allow) any matching URL, "
        "which takes priority over any blacklist (blocking) filter.");

    
    if ( url == "filterhelp" )
        QWhatsThis::showText( QCursor::pos(), helpString );
}


void KCMWebkitAdblock::addExpr()
{
    listWidget->addItem( lineEdit->text() );
    lineEdit->clear();
    emit changed(true);
}


void KCMWebkitAdblock::removeSelected()
{
    listWidget->takeItem(listWidget->currentRow());
    searchLine->clear();
    emit changed(true);
}


void KCMWebkitAdblock::importExpr()
{
   
    QString target;
    KUrl url("http://adblockplus.mozdev.org/easylist/easylist.txt");
        
    kDebug() << "downloading list..";
    
    bool success = KIO::NetAccess::download(url, target, 0);
    if(!success)
    {
        kDebug() << "not success";
        return;
    }
    
    QFile temp(target);
    if (!temp.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        kDebug() << "File not open";
        return;
    }
    
    QTextStream stream(&temp);
    QString line;
    do 
    {
        line = stream.readLine();
        if(!line.startsWith('!') && !line.startsWith('['))
            listWidget->addItem(line);
    }
    while (!line.isNull());
    
    KIO::NetAccess::removeTempFile(target);
    emit changed(true);
}
