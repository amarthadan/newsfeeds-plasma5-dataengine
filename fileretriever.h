#ifndef FILERETIREVER_H
#define FILERETIREVER_H

#include <Syndication/DataRetriever>

#include <KIO/Job>
#include <KJob>

#include <QUrl>
#include <QString>
#include <QByteArray>

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
     * <a href="http://developer.kde.org/documentation/library/cvs-api/classref/kio/KIO.html#Error">as
     * defined by KIO</a>.
     */
    int errorCode() const override;

    /**
     * aborts the retrieval process.
     */
    void abort() override;

Q_SIGNALS:
    /**
     * Signals a permanent redirection. The redirection itself is
     * handled internally, so you don't need to call Loader::loadFrom()
     * with the new URL. This signal is useful in case you want to
     * notify the user, or adjust a database entry.
     *
     * @param url the new URL after the redirection
     *
     * @see Loader::loadFrom()
     */
    void permanentRedirection(const QUrl &url);

protected Q_SLOTS:

    void slotTimeout();

private Q_SLOTS:

    void slotData(KIO::Job *job, const QByteArray &data);
    void slotResult(KJob *job);
    void slotPermanentRedirection(KIO::Job *job, const QUrl &fromUrl,
                                  const QUrl &toUrl);

private:
    FileRetriever(const FileRetriever &other);
    FileRetriever &operator=(const FileRetriever &other);

    struct FileRetrieverPrivate;
    FileRetrieverPrivate *const d;
};
#endif // FILERETIREVER_H
