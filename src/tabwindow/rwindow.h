/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2012-2013 by Andrea Diamantini <adjam7 at gmail dot com>
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



#ifndef R_WINDOW_H
#define R_WINDOW_H


// Rekonq Includes
#include "rekonq_defines.h"

// KDE Includes
#include <KConfig>
#include <KConfigGroup>

// Qt Includes
#include <QWidget>

/**
 * This is rekonq (re)implementation of KMainWindow,
 * given that we'd like to NOT use a "real" xMainWindow
 * but a widget with the nice mainwindow properties
 * (eg: session management, auto save dimension, etc) but
 * NOT its peculiar containers (eg: toolbars, menubar, statusbar,
 * central widget...)
 *
 */
class RWindow : public QWidget
{
    friend class KRWSessionManager;

    Q_OBJECT

public:
    explicit RWindow(QWidget* parent = 0);

    virtual ~RWindow();

    QSize sizeHint() const;

    /**
     * List of members of RekonqWindow class.
     */
    static QList<RWindow*> windowList();

    /**
     * If the session did contain so high a @p number, @p true is returned,
     * else @p false.
     * @see restore()
     **/
    static bool canBeRestored(int number);

    /**
     * Try to restore the toplevel widget as defined by @p number (1..X).
     *
     * You should call canBeRestored() first.
     *
     **/
    bool restore(int number, bool show = true);

protected:
    /**
     * Save your instance-specific properties. The function is
     * invoked when the session manager requests your application
     * to save its state.
     *
     * Please reimplement these function in childclasses.
     *
     * Note: No user interaction is allowed
     * in this function!
     *
     */
    virtual void saveProperties(KConfigGroup &) {}

    /**
    * Read your instance-specific properties.
    *
    * Is called indirectly by restore().
    */
    virtual void readProperties(const KConfigGroup &) {}

    void savePropertiesInternal(KConfig*, int);
    bool readPropertiesInternal(KConfig*, int);

    /**
     * For inherited classes
     */
    void saveWindowSize(const KConfigGroup &config) const;
    /**
     * For inherited classes
     * Note that a -geometry on the command line has priority.
     */
    void restoreWindowSize(const KConfigGroup & config);

    /// parse the geometry from the geometry command line argument
    void parseGeometry();

    virtual void resizeEvent(QResizeEvent *);

private Q_SLOTS:
    void saveAutoSaveSettings();

private:
    /// This has been added to just fix window position && not let them be overlying
    void checkPosition();
};

#endif // R_WINDOW_H
