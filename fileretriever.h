#ifndef FILERETIREVER_H
#define FILERETIREVER_H

#include <Syndication/DataRetriever>

#include <QUrl>
#include <QString>
#include <QByteArray>
#include <QNetworkReply>
#include <QList>
#include <QSslError>
#include <QLoggingCategory>

class FileRetriever: public Syndication::DataRetriever
{
    Q_OBJECT

public:
    FileRetriever();
    ~FileRetriever() override;

    /**
     * Downloads the file referenced by the given URL and passes it's
     * contents on to the Loader.
     * @param url An URL referencing a file which is assumed to
     * reference valid XML.
     * @see Loader::loadFrom()
     */
    void retrieveData(const QUrl &url) override;

    /**
     * @return The error code for the last process of retrieving data.
     * The returned numbers correspond directly to the error codes
     * <a href="https://doc.qt.io/qt-5/qnetworkreply.html#NetworkError-enum">as
     * defined by QNetworkReply</a>.
     */
    int errorCode() const override;

    /**
     * aborts the retrieval process.
     */
    void abort() override;

private Q_SLOTS:
    void httpFinished();
    void httpReadyRead();
#ifndef QT_NO_SSL
    void sslErrors(QNetworkReply *, const QList<QSslError> &errors);
#endif

private:
    FileRetriever(const FileRetriever &other);
    FileRetriever &operator=(const FileRetriever &other);

    struct FileRetrieverPrivate;
    FileRetrieverPrivate *const d;
};

Q_DECLARE_LOGGING_CATEGORY(FILERETRIEVER)

#endif // FILERETIREVER_H
