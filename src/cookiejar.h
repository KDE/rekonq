

#ifndef RECOOKIEJAR_H
#define RECOOKIEJAR_H


// Qt Includes
#include <QtNetwork/QNetworkCookieJar>

// Forward Declarations
class QUrl;

class CookieJar : public QNetworkCookieJar
{
public:
    CookieJar(QObject* parent = 0);
    virtual ~CookieJar();

    virtual QList<QNetworkCookie> cookiesForUrl(const QUrl & url) const;
    
    virtual bool setCookiesFromUrl(const QList<QNetworkCookie> & cookieList, const QUrl & url);
    
    void setWindowId(qlonglong id);
    
private:
    qlonglong m_windowId;
};

#endif // RECOOKIEJAR_H
