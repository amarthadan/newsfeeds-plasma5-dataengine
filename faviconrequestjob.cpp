#include "faviconrequestjob.h"

#include "faviconstorage.h"

#include <QByteArray>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

QUrl iconUrlForUrl(const QUrl &url)
{
    QUrl iconUrl;
    iconUrl.setScheme(url.scheme());
    iconUrl.setHost(url.host());
    iconUrl.setPath(QStringLiteral("/favicon.ico"));
    iconUrl.setUserInfo(url.userInfo());
    return iconUrl;
}

struct FaviconRequestJob::FaviconRequestJobPrivate {
    FaviconRequestJobPrivate(const QUrl &requestUrl)
        :requestUrl(requestUrl), reply(nullptr), lastError(0), httpRequestAborted(false)
    {
    }

    QUrl requestUrl;
    QUrl iconUrl;
    QString iconFile;
    QByteArray iconData;
    QNetworkAccessManager nam;
    QNetworkReply *reply;
    int lastError;
    bool httpRequestAborted;
};

FaviconRequestJob::FaviconRequestJob(const QUrl &requestUrl, QObject *parent)
    : QObject(parent), d(new FaviconRequestJobPrivate(requestUrl))
{
    d->nam.setRedirectPolicy(QNetworkRequest::NoLessSafeRedirectPolicy);
#ifndef QT_NO_SSL
    connect(&d->nam, &QNetworkAccessManager::sslErrors,
            this, &FaviconRequestJob::sslErrors);
#endif

    QMetaObject::invokeMethod(this, "makeRequest", Qt::QueuedConnection);
}

FaviconRequestJob::~FaviconRequestJob()
{
    delete d;
}

void FaviconRequestJob::makeRequest()
{
    d->iconUrl = iconUrlForUrl(d->requestUrl);

    qCDebug(FAVICONREQUESTJOB) << "downloading" << d->iconUrl;
    QNetworkRequest request = QNetworkRequest(d->iconUrl);
    request.setHeader(QNetworkRequest::UserAgentHeader, "KDE Plasma NewsfeedsEngine");
    request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
    request.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::AlwaysNetwork);

    d->reply = d->nam.get(request);
    connect(d->reply, &QNetworkReply::finished, this, &FaviconRequestJob::httpFinished);
    connect(d->reply, &QIODevice::readyRead, this, &FaviconRequestJob::httpReadyRead);
}

void FaviconRequestJob::httpReadyRead()
{
    QByteArray data = d->reply->readAll();
    unsigned int oldSize = d->iconData.size();
    if (oldSize > 0x10000) { // 65K
      qCWarning(FAVICONREQUESTJOB) << "Favicon too big, aborting download of" << d->iconUrl;
      abort();
    } else {
      d->iconData.resize(oldSize + data.size());
      memcpy(d->iconData.data() + oldSize, data.data(), data.size());
    }
}

void FaviconRequestJob::httpFinished()
{
    if (d->httpRequestAborted) {
        d->reply->deleteLater();
        d->reply = nullptr;
        return;
    }

    if (!d->reply->error()) {
        FavIconStorage storage;
        d->iconFile = storage.saveIcon(&d->iconData, d->iconUrl);
    } else {
        d->lastError = d->reply->error();
    }

    d->iconData.clear(); // release memory
    if (d->iconFile.isEmpty()) {
        d->lastError = QNetworkReply::UnknownContentError;
    }

    d->reply->deleteLater();
    d->reply = nullptr;

    emit iconReady(this);
}

void FaviconRequestJob::abort()
{
    if (d->reply) {
      d->httpRequestAborted = true;
      d->reply->abort();
    }
}

QString FaviconRequestJob::iconFile() const
{
  return d->iconFile;
}

QUrl FaviconRequestJob::requestUrl() const
{
  return d->requestUrl;
}

int FaviconRequestJob::errorCode() const
{
    return d->lastError;
}

#ifndef QT_NO_SSL
void FaviconRequestJob::sslErrors(QNetworkReply *, const QList<QSslError> &errors)
{
  qCCritical(FAVICONREQUESTJOB) << "SSL errors:" << errors;
}
#endif

Q_LOGGING_CATEGORY(FAVICONREQUESTJOB, "faviconrequestjob")
