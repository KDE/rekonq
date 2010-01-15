/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) <year>  <name of author>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef PREVIEWSELECTORBAR_H
#define PREVIEWSELECTORBAR_H

// Local Includes
#include "webpage.h"

// Qt Includes
#include <QWidget>
#include <QPushButton>
#include <QLabel>

class PreviewSelectorBar : public QWidget
{
    Q_OBJECT
    
    public:
        PreviewSelectorBar(QWidget *parent = 0);
        
        void setPage(WebPage *page);
        
    public slots:
        void enable(int previewIndex, WebPage *page);
        void clicked();
        
        void loadProgress();
        void loadFinished();
        
        void verifyUrl();
        
    private:
        void setup();
        
        QPushButton *m_button;
        QLabel *m_label;
        
        int m_previewIndex;
        WebPage *m_page;
    
};

#endif // PREVIEWSELECTORBAR_H
