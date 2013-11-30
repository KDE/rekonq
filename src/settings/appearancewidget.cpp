/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2010-2011 by Andrea Diamantini <adjam7 at gmail dot com>
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

// KDE Includes
#include <KGlobal>
#include <KCharsets>


int zoomFactorList[13] = {5, 6, 7, 8, 9, 10, 11, 13, 15, 20, 25, 30};



AppearanceWidget::AppearanceWidget(QWidget *parent)
    : QWidget(parent)
    , _changed(false)
{
    setupUi(this);

    fixedFontChooser->setOnlyFixed(true);

    standardFontChooser->setCurrentFont(QFont(ReKonfig::standardFontFamily()));
    fixedFontChooser->setCurrentFont(QFont(ReKonfig::fixedFontFamily()));
    serifFontChooser->setCurrentFont(QFont(ReKonfig::serifFontFamily()));
    sansSerifFontChooser->setCurrentFont(QFont(ReKonfig::sansSerifFontFamily()));
    cursiveFontChooser->setCurrentFont(QFont(ReKonfig::cursiveFontFamily()));
    fantasyFontChooser->setCurrentFont(QFont(ReKonfig::fantasyFontFamily()));

    connect(standardFontChooser,    SIGNAL(currentFontChanged(QFont)), this, SLOT(hasChanged()));
    connect(fixedFontChooser,       SIGNAL(currentFontChanged(QFont)), this, SLOT(hasChanged()));
    connect(serifFontChooser,       SIGNAL(currentFontChanged(QFont)), this, SLOT(hasChanged()));
    connect(sansSerifFontChooser,   SIGNAL(currentFontChanged(QFont)), this, SLOT(hasChanged()));
    connect(cursiveFontChooser,     SIGNAL(currentFontChanged(QFont)), this, SLOT(hasChanged()));
    connect(fantasyFontChooser,     SIGNAL(currentFontChanged(QFont)), this, SLOT(hasChanged()));

    populateEncodingMenu();
    populateZoomMenu();
}


void AppearanceWidget::save()
{
    ReKonfig::setStandardFontFamily(standardFontChooser->currentFont().family());
    ReKonfig::setFixedFontFamily(fixedFontChooser->currentFont().family());
    ReKonfig::setSerifFontFamily(serifFontChooser->currentFont().family());
    ReKonfig::setSansSerifFontFamily(sansSerifFontChooser->currentFont().family());
    ReKonfig::setCursiveFontFamily(cursiveFontChooser->currentFont().family());
    ReKonfig::setFantasyFontFamily(fantasyFontChooser->currentFont().family());

    // zoom
    int index = zoomCombo->currentIndex();
    int zoomFactor = zoomFactorList[index];
    ReKonfig::setDefaultZoom(zoomFactor);
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
    // TODO: implement me!!

    return !_changed;
}


void AppearanceWidget::populateEncodingMenu()
{
    encodingCombo->setEditable(false);
    QStringList encodings = KGlobal::charsets()->availableEncodingNames();
    encodingCombo->addItems(encodings);

    encodingCombo->setWhatsThis(i18n("Select the default encoding to be used; normally, you will be fine with 'Use language encoding' "
                                     "and should not have to change this."));

    QString enc = ReKonfig::defaultEncoding();
    int indexOfEnc = encodings.indexOf(enc);
    encodingCombo->setCurrentIndex(indexOfEnc);

    connect(encodingCombo, SIGNAL(activated(QString)), this, SLOT(setEncoding(QString)));
    connect(encodingCombo, SIGNAL(activated(QString)), this, SLOT(hasChanged()));
}


void AppearanceWidget::populateZoomMenu()
{
    zoomCombo->setEditable(false);
    QStringList availableZooms;

    int actualZoom = 0;
    int defZoom = ReKonfig::defaultZoom();

    for (int i = 0; i < 13; i++)
    {
        int zoomFactor = zoomFactorList[i];
        QString zoomString = QString::number(zoomFactor*10) + QL1S("%");
        availableZooms << zoomString;

        if (zoomFactor == defZoom)
        {
            actualZoom = i;
        }
    }

    zoomCombo->addItems(availableZooms);
    zoomCombo->setCurrentIndex(actualZoom);

    connect(zoomCombo, SIGNAL(activated(QString)), this, SLOT(hasChanged()));
}


void AppearanceWidget::setEncoding(const QString &enc)
{
    ReKonfig::setDefaultEncoding(enc);
}
