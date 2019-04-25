#include "fileretriever.h"

#include <QBuffer>
#include <QNetworkAccessManager>

struct FileRetriever::FileRetrieverPrivate {
    FileRetrieverPrivate()
        : buffer(nullptr), reply(nullptr), lastError(0), httpRequestAborted(false)
    {
    }

    ~FileRetrieverPrivate()
    {
        delete buffer;
    }

    QBuffer *buffer;
    QNetworkAccessManager nam;
    QNetworkReply *reply;
    int lastError;
    bool httpRequestAborted;
};

FileRetriever::FileRetriever()
    : d(new FileRetrieverPrivate)
{
    d->nam.setRedirectPolicy(QNetworkRequest::NoLessSafeRedirectPolicy);
#ifndef QT_NO_SSL
    connect(&d->nam, &QNetworkAccessManager::sslErrors,
            this, &FileRetriever::sslErrors);
#endif
}

FileRetriever::~FileRetriever()
{
    delete d;
}

void FileRetriever::retrieveData(const QUrl &url)
{
    if (d->buffer) {
        return;
    }

    d->buffer = new QBuffer;
    d->buffer->open(QIODevice::WriteOnly);

    d->httpRequestAborted = false;

    QUrl u = url;

    if (u.scheme() == QLatin1String("feed")) {
        u.setScheme(QStringLiteral("http"));
    }

    qCDebug(FILERETRIEVER) << "downloading" << u;
    QNetworkRequest request = QNetworkRequest(u);
    request.setHeader(QNetworkRequest::UserAgentHeader, "KDE Plasma NewsfeedsEngine");
    request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);

    d->reply = d->nam.get(request);
    connect(d->reply, &QNetworkReply::finished, this, &FileRetriever::httpFinished);
    connect(d->reply, &QIODevice::readyRead, this, &FileRetriever::httpReadyRead);
}

void FileRetriever::httpReadyRead()
{
    d->buffer->write(d->reply->readAll());
}

void FileRetriever::httpFinished()
{
    if (d->httpRequestAborted) {
        d->reply->deleteLater();
        d->reply = nullptr;
        return;
    }

    qCDebug(FILERETRIEVER) << "finished downloading" << d->reply->request().url();

    d->lastError = d->reply->error();
    d->reply->deleteLater();
    d->reply = nullptr;

    QByteArray data = d->buffer->buffer();
    data.detach();

    delete d->buffer;
    d->buffer = nullptr;

    emit dataRetrieved(data, d->lastError == QNetworkReply::NoError);
}

void FileRetriever::abort()
{
    if (d->reply) {
      qCDebug(FILERETRIEVER) << "aborting" << d->reply->request().url();

      d->httpRequestAborted = true;
      d->reply->abort();
      delete d->buffer;
      d->buffer = nullptr;
    }
}

int FileRetriever::errorCode() const
{
    return d->lastError;
}

#ifndef QT_NO_SSL
void FileRetriever::sslErrors(QNetworkReply *, const QList<QSslError> &errors)
{
  qCCritical(FILERETRIEVER) << "SSL errors:" << errors;
}
#endif

Q_LOGGING_CATEGORY(FILERETRIEVER, "fileretriever")
