/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2010 by Andrea Diamantini <adjam7 at gmail dot com>
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
#include "appearancewidget.h"
#include "appearancewidget.moc"

// Auto Includes
#include "rekonq.h"


AppearanceWidget::AppearanceWidget(QWidget *parent)
        : QWidget(parent)
        , _changed(false)
{
    setupUi(this);
    
    fixedFontChooser->setOnlyFixed(true);
    
    standardFontChooser->setCurrentFont( QFont( ReKonfig::standardFontFamily() ) );
    fixedFontChooser->setCurrentFont( QFont( ReKonfig::fixedFontFamily() ) );
    serifFontChooser->setCurrentFont( QFont( ReKonfig::serifFontFamily() ) );
    sansSerifFontChooser->setCurrentFont( QFont( ReKonfig::sansSerifFontFamily() ) );
    cursiveFontChooser->setCurrentFont( QFont( ReKonfig::cursiveFontFamily() ) );
    fantasyFontChooser->setCurrentFont( QFont( ReKonfig::fantasyFontFamily() ) );
                        
    connect(standardFontChooser,    SIGNAL(currentFontChanged(const QFont &)), this, SLOT(slotStandardFont(const QFont &)));
    connect(fixedFontChooser,       SIGNAL(currentFontChanged(const QFont &)), this, SLOT(slotFixedFont(const QFont &)));
    connect(serifFontChooser,       SIGNAL(currentFontChanged(const QFont &)), this, SLOT(slotSerifFont(const QFont &)));
    connect(sansSerifFontChooser,   SIGNAL(currentFontChanged(const QFont &)), this, SLOT(slotSansSerifFont(const QFont &)));
    connect(cursiveFontChooser,     SIGNAL(currentFontChanged(const QFont &)), this, SLOT(slotCursiveFont(const QFont &)));
    connect(fantasyFontChooser,     SIGNAL(currentFontChanged(const QFont &)), this, SLOT(slotFantasyFont(const QFont &)));
}


void AppearanceWidget::save()
{
    ReKonfig::setStandardFontFamily(reFont[0]);
    ReKonfig::setFixedFontFamily(reFont[1]);
    ReKonfig::setSerifFontFamily(reFont[2]);
    ReKonfig::setSansSerifFontFamily(reFont[3]);
    ReKonfig::setCursiveFontFamily(reFont[4]);
    ReKonfig::setFantasyFontFamily(reFont[5]);
}


bool AppearanceWidget::changed()
{
    return _changed;
}


void AppearanceWidget::hasChanged()
{
    _changed = true;
}


void AppearanceWidget::slotStandardFont(const QFont &f)
{
    reFont[0] = f.family();
    hasChanged();
}


void AppearanceWidget::slotFixedFont(const QFont &f)
{
    reFont[1] = f.family();
    hasChanged();
}


void AppearanceWidget::slotSerifFont(const QFont &f)
{
    reFont[2] = f.family();
    hasChanged();
}


void AppearanceWidget::slotSansSerifFont(const QFont &f)
{
    reFont[3] = f.family();
    hasChanged();
}


void AppearanceWidget::slotCursiveFont(const QFont &f)
{
    reFont[4] = f.family();
    hasChanged();
}


void AppearanceWidget::slotFantasyFont(const QFont &f)
{
    reFont[5] = f.family();
    hasChanged();
}
