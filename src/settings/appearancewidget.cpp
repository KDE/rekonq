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
                        
    connect(standardFontChooser,    SIGNAL(currentFontChanged(const QFont &)), this, SLOT( hasChanged() ));
    connect(fixedFontChooser,       SIGNAL(currentFontChanged(const QFont &)), this, SLOT( hasChanged() ));
    connect(serifFontChooser,       SIGNAL(currentFontChanged(const QFont &)), this, SLOT( hasChanged() ));
    connect(sansSerifFontChooser,   SIGNAL(currentFontChanged(const QFont &)), this, SLOT( hasChanged() ));
    connect(cursiveFontChooser,     SIGNAL(currentFontChanged(const QFont &)), this, SLOT( hasChanged() ));
    connect(fantasyFontChooser,     SIGNAL(currentFontChanged(const QFont &)), this, SLOT( hasChanged() ));
}


void AppearanceWidget::save()
{
    ReKonfig::setStandardFontFamily(    standardFontChooser->currentFont().family() );
    ReKonfig::setFixedFontFamily(       fixedFontChooser->currentFont().family() );
    ReKonfig::setSerifFontFamily(       serifFontChooser->currentFont().family() );
    ReKonfig::setSansSerifFontFamily(   sansSerifFontChooser->currentFont().family() );
    ReKonfig::setCursiveFontFamily(     cursiveFontChooser->currentFont().family() );
    ReKonfig::setFantasyFontFamily(     fantasyFontChooser->currentFont().family() );
}


bool AppearanceWidget::changed()
{
    return _changed;
}


void AppearanceWidget::hasChanged()
{
    _changed = true;
    emit changed(true);
}


bool AppearanceWidget::isDefault()
{
    bool def = true;
    
    // TODO: implement me!!
    
    return def;
}