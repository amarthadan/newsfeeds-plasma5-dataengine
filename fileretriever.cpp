#include "fileretriever.h"

#include <KIO/TransferJob>

#include <QBuffer>
#include <QTimer>

struct FileRetriever::FileRetrieverPrivate {
    FileRetrieverPrivate()
        : buffer(nullptr),
          lastError(0), job(nullptr)
    {
    }

    ~FileRetrieverPrivate()
    {
        delete buffer;
    }

    QBuffer *buffer;
    int lastError;
    KIO::TransferJob *job;
};

FileRetriever::FileRetriever()
    : d(new FileRetrieverPrivate)
{
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

    QUrl u = url;

    if (u.scheme() == QLatin1String("feed")) {
        u.setScheme(QStringLiteral("http"));
    }

    d->job = KIO::get(u, KIO::NoReload, KIO::HideProgressInfo);

    d->job->addMetaData(QStringLiteral("UserAgent"), QStringLiteral("KDE Plasma NewsfeedsEngine"));
    d->job->addMetaData(QStringLiteral("cache"), QStringLiteral("refresh"));

    QTimer::singleShot(30 * 1000, this, &FileRetriever::slotTimeout);

    connect(d->job, &KIO::TransferJob::data, this, &FileRetriever::slotData);
    connect(d->job, &KIO::TransferJob::result, this, &FileRetriever::slotResult);
    connect(d->job, &KIO::TransferJob::permanentRedirection, this, &FileRetriever::slotPermanentRedirection);
}

void FileRetriever::slotTimeout()
{
    abort();

    delete d->buffer;
    d->buffer = nullptr;

    d->lastError = KIO::ERR_SERVER_TIMEOUT;

    emit dataRetrieved(QByteArray(), false);
}

int FileRetriever::errorCode() const
{
    return d->lastError;
}

void FileRetriever::slotData(KIO::Job *, const QByteArray &data)
{
    d->buffer->write(data.data(), data.size());
}

void FileRetriever::slotResult(KJob *job)
{
    QByteArray data = d->buffer->buffer();
    data.detach();

    delete d->buffer;
    d->buffer = nullptr;

    d->lastError = job->error();
    emit dataRetrieved(data, d->lastError == 0);
}

void FileRetriever::slotPermanentRedirection(KIO::Job *, const QUrl &,
        const QUrl &newUrl)
{
    emit permanentRedirection(newUrl);
}

void FileRetriever::abort()
{
    if (d->job) {
        d->job->kill();
        d->job = nullptr;
    }
}
