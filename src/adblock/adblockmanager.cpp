/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2010-2014 by Andrea Diamantini <adjam7 at gmail dot com>
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
#include "adblockmanager.h"

// Auto Includes
#include "rekonq.h"

// Local Includes
#include "adblocksettingwidget.h"

#include "webpage.h"

// KDE Includes
#include <KIO/FileCopyJob>

// Qt Includes
#include <QDialog>
#include <QUrl>
#include <QTimer>
#include <QStandardPaths>

#include <QWebElement>
#include <QWebFrame>
#include <QNetworkReply>
#include <QNetworkRequest>

#include <QtConcurrent>


QPointer<AdBlockManager> AdBlockManager::s_adBlockManager;


AdBlockManager *AdBlockManager::self()
{
    if (s_adBlockManager.isNull())
    {
        s_adBlockManager = new AdBlockManager(qApp);
    }
    return s_adBlockManager.data();
}


// ----------------------------------------------------------------------------------------------


AdBlockManager::AdBlockManager(QObject *parent)
    : QObject(parent)
    , _isAdblockEnabled(false)
    , _isHideAdsEnabled(false)
{
    // NOTE: launch this in a second thread so that it does not delay startup
    _settingsLoaded = QtConcurrent::run(this, &AdBlockManager::loadSettings);
}


AdBlockManager::~AdBlockManager()
{
    _whiteList.clear();
    _blackList.clear();
}


bool AdBlockManager::isEnabled()
{
    return _isAdblockEnabled;
}


bool AdBlockManager::isHidingElements()
{
    return _isHideAdsEnabled;
}


void AdBlockManager::loadSettings()
{
    // first, check this...
    const QString adblockFilePath = QStandardPaths::writableLocation(QStandardPaths::DataLocation) + QL1S("/adblockrc");
    if (!QFile::exists(adblockFilePath))
    {
        const QString generalAdblockFilePath = QStandardPaths::locate(QStandardPaths::DataLocation, QL1S("adblockrc"));
        QFile adblockFile(generalAdblockFilePath);
        // FIXME problem with non extant directory
        const bool copied = adblockFile.copy(adblockFilePath);
        if (!copied)
        {
            qDebug() << "oh oh... Problems copying default adblock file";
            return;
        }
    }
    _adblockConfig = KSharedConfig::openConfig( QL1S("adblockrc"), KConfig::SimpleConfig, QStandardPaths::DataLocation );
    // ----------------

    _hostWhiteList.clear();
    _hostBlackList.clear();

    _whiteList.clear();
    _blackList.clear();

    _elementHiding.clear();

    KConfigGroup settingsGroup(_adblockConfig, "Settings");

    // no need to load filters if adblock is not enabled :)
    if (!settingsGroup.readEntry("adBlockEnabled", false))
    {
        _isAdblockEnabled = false;
        return;
    }

    // just to be sure..
    _isHideAdsEnabled = settingsGroup.readEntry("hideAdsEnabled", false);

    // ----------------------------------------------------------

    const QDateTime today = QDateTime::currentDateTime();
    QDateTime lastUpdate = QDateTime::fromString(settingsGroup.readEntry("lastUpdate", QString()));
    int days = settingsGroup.readEntry("updateInterval", 7);

    bool allSubscriptionsNeedUpdate = (today > lastUpdate.addDays(days));
    if (allSubscriptionsNeedUpdate)
    {
        settingsGroup.writeEntry("lastUpdate", today.toString());
    }

    // (Eventually) update and load automatic rules
    KConfigGroup filtersGroup(_adblockConfig, "FiltersList");
    for (int i = 0; i < 60; i++)
    {
        QString n = QString::number(i + 1);
        if (!filtersGroup.hasKey( QL1S("FilterEnabled-") + n))
            continue;

        bool isFilterEnabled = filtersGroup.readEntry( QL1S("FilterEnabled-") + n, false);
        if (!isFilterEnabled)
            continue;

        bool fileExists = subscriptionFileExists(i);
        if (allSubscriptionsNeedUpdate || !fileExists)
        {
            qDebug() << "FILE SHOULDN'T EXIST. updating subscription";
            updateSubscription(i);
        }
        else
        {
            QString rulesFilePath = QStandardPaths::writableLocation(QStandardPaths::DataLocation) + QL1C('/') + QL1S("adblockrules_") + n;
            loadRules(rulesFilePath);
        }
    }

    // load local rules
    QString localRulesFilePath = QStandardPaths::writableLocation(QStandardPaths::DataLocation) + QL1C('/') + QL1S("adblockrules_local");
    loadRules(localRulesFilePath);

    _isAdblockEnabled = true;
}


void AdBlockManager::loadRules(const QString &rulesFilePath)
{
    QFile ruleFile(rulesFilePath);
    if (!ruleFile.open(QFile::ReadOnly | QFile::Text))
    {
        qDebug() << "Unable to open rule file" << rulesFilePath;
        return;
    }

    QTextStream in(&ruleFile);
    while (!in.atEnd())
    {
        QString stringRule = in.readLine();
        loadRuleString(stringRule);
    }
}


void AdBlockManager::loadRuleString(const QString &stringRule)
{
    // ! rules are comments
    if (stringRule.startsWith( QL1C('!') ))
        return;

    // [ rules are ABP info
    if (stringRule.startsWith( QL1C('[') ))
        return;

    // empty rules are just dangerous..
    // (an empty rule in whitelist allows all, in blacklist blocks all..)
    if (stringRule.isEmpty())
        return;

    // white rules
    if (stringRule.startsWith(QL1S("@@")))
    {
        if (_hostWhiteList.tryAddFilter(stringRule))
            return;

        const QString filter = stringRule.mid(2);
        if (filter.isEmpty())
            return;

        AdBlockRule rule(filter);
        _whiteList << rule;
        return;
    }

    // hide (CSS) rules
    if (stringRule.contains(QL1S("##")))
    {
        _elementHiding.addRule(stringRule);
        return;
    }

    if (_hostBlackList.tryAddFilter(stringRule))
        return;

    AdBlockRule rule(stringRule);
    _blackList << rule;
}


bool AdBlockManager::blockRequest(const QNetworkRequest &request)
{
    if (!_isAdblockEnabled)
        return false;

    // we (ad)block just http & https traffic
    if (request.url().scheme() != QL1S("http")
            && request.url().scheme() != QL1S("https"))
        return false;

    QStringList whiteRefererList = ReKonfig::whiteReferer();
    const QString referer = QL1S(request.rawHeader("referer"));
    Q_FOREACH(const QString & host, whiteRefererList)
    {
        if (referer.contains(host))
            return false;
    }

    QString urlString = request.url().toString();
    // We compute a lowercase version of the URL so each rule does not
    // have to do it.
    const QString urlStringLowerCase = urlString.toLower();
    const QString host = request.url().host();

    // check white rules before :)
    if (_hostWhiteList.match(host))
    {
        qDebug() << "ADBLOCK: WHITE RULE (@@) Matched by string: " << urlString;
        return false;
    }

    Q_FOREACH(const AdBlockRule & filter, _whiteList)
    {
        if (filter.match(request, urlString, urlStringLowerCase))
        {
            qDebug() << "ADBLOCK: WHITE RULE (@@) Matched by string: " << urlString;
            return false;
        }
    }

    // then check the black ones :(
    if (_hostBlackList.match(host))
    {
        qDebug() << "ADBLOCK: BLACK RULE Matched by string: " << urlString;
        return true;
    }

    Q_FOREACH(const AdBlockRule & filter, _blackList)
    {
        if (filter.match(request, urlString, urlStringLowerCase))
        {
            qDebug() << "ADBLOCK: BLACK RULE Matched by string: " << urlString;
            return true;
        }
    }

    // no match
    return false;
}


void AdBlockManager::updateSubscription(int i)
{
    KConfigGroup filtersGroup(_adblockConfig, "FiltersList");
    QString n = QString::number(i + 1);

    const QString fUrl = filtersGroup.readEntry( QL1S("FilterURL-") + n, QString());
    QUrl subUrl = QUrl(fUrl);

    QString rulesFilePath = QStandardPaths::writableLocation(QStandardPaths::DataLocation) + QL1C('/') + QL1S("adblockrules_") + n;
    QUrl destUrl = QUrl(rulesFilePath);

    KIO::FileCopyJob* job = KIO::file_copy(subUrl , destUrl, -1, KIO::HideProgressInfo | KIO::Overwrite);
    job->metaData().insert( QL1S("ssl_no_client_cert"),  QL1S("true") );
    job->metaData().insert( QL1S("ssl_no_ui"),  QL1S("true") );
    job->metaData().insert( QL1S("UseCache"),  QL1S("false") );
    job->metaData().insert( QL1S("cookies"),  QL1S("none") );
    job->metaData().insert( QL1S("no-auth"),  QL1S("true") );

    connect(job, SIGNAL(finished(KJob*)), this, SLOT(slotFinished(KJob*)));
}


void AdBlockManager::slotFinished(KJob *job)
{
    if (job->error())
        return;

    KIO::FileCopyJob *fJob = qobject_cast<KIO::FileCopyJob *>(job);
    QUrl url = fJob->destUrl();
    url.setScheme(QString()); // this is needed to load local url well :(
    loadRules(url.url());
}


bool AdBlockManager::subscriptionFileExists(int i)
{
    const QString n = QString::number(i + 1);

    const QString rulesFilePath = QStandardPaths::writableLocation(QStandardPaths::DataLocation) + QL1C('/') + QL1S("adblockrules_") + n;
    return QFile::exists(rulesFilePath);
}


void AdBlockManager::showSettings()
{
    // at this point, the settings should be loaded
    _settingsLoaded.waitForFinished();

    AdBlockSettingWidget* dialog = new AdBlockSettingWidget(_adblockConfig);

    connect(dialog, SIGNAL(okClicked()), this, SLOT(loadSettings()));
    dialog->show();
}


void AdBlockManager::addCustomRule(const QString &stringRule, bool reloadPage)
{
    // at this point, the settings should be loaded
    _settingsLoaded.waitForFinished();

    // save rule in local filters
    const QString localRulesFilePath = QStandardPaths::writableLocation(QStandardPaths::DataLocation) + QL1C('/') + QL1S("adblockrules_local");

    QFile ruleFile(localRulesFilePath);
    if (!ruleFile.open(QFile::WriteOnly | QFile::Append))
    {
        qDebug() << "Unable to open rule file" << localRulesFilePath;
        return;
    }

    QTextStream out(&ruleFile);
    out << stringRule << QL1C('\n');

    ruleFile.close();

    // load it
    loadRuleString(stringRule);

    // eventually reload page
    if (reloadPage)
        emit reloadCurrentPage();
}


bool AdBlockManager::isAdblockEnabledForHost(const QString &host)
{
    if (!_isAdblockEnabled)
        return false;

    return ! _hostWhiteList.match(host);
}


void AdBlockManager::applyHidingRules(QWebFrame *frame)
{
    if (!frame)
        return;

    if (!_isAdblockEnabled)
        return;

    connect(frame, SIGNAL(loadFinished(bool)), this, SLOT(applyHidingRules(bool)));
}


void AdBlockManager::applyHidingRules(bool ok)
{
    if (!ok)
        return;

    QWebFrame *frame = qobject_cast<QWebFrame *>(sender());
    if (!frame)
        return;

    WebPage *page = qobject_cast<WebPage *>(frame->page());
    if (!page)
        return;

    QString mainPageHost = page->loadingUrl().host();
    QStringList hosts = ReKonfig::whiteReferer();
    if (hosts.contains(mainPageHost))
        return;

    QWebElement document = frame->documentElement();

    _elementHiding.apply(document, mainPageHost);
}
