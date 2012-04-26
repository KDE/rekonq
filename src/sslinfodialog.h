/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2011-2012 by Andrea Diamantini <adjam7 at gmail dot com>
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


#ifndef SSL_INFO_DIALOG_H
#define SSL_INFO_DIALOG_H

#include "rekonq_defines.h"

#include "websslinfo.h"

// Ui Includes
#include "ui_sslinfo.h"

#include <KDE/KDialog>

#include <QList>

class QSslCertificate;
class QString;

/**
 * Rekonq SSL Information Dialog
 *
 * This class creates a dialog that can be used to display information about
 * an SSL session.
 *
 */
class SslInfoDialog : public KDialog
{
    Q_OBJECT

public:
    explicit SslInfoDialog(const QString &host, const WebSslInfo &info, QWidget *parent = 0);

    static QList<QStringList> errorsFromString(const QString &s);

private Q_SLOTS:
    void displayFromChain(int);
    void exportCert();

private:
    void showCertificateInfo(QSslCertificate, const QStringList &certErrors);

    QString m_host;
    WebSslInfo m_info;

    Ui::SslInfo ui;
};

#endif // SSL_INFO_DIALOG_H
