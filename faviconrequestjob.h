#ifndef FAVICONREQUESTJOB_H
#define FAVICONREQUESTJOB_H

#include <QObject>
#include <QUrl>
#include <QString>
#include <QNetworkReply>
#include <QList>
#include <QSslError>
#include <QLoggingCategory>

class FaviconRequestJob: public QObject
{
    Q_OBJECT

public:
    FaviconRequestJob(const QUrl &requestUrl, QObject *parent = nullptr);
    ~FaviconRequestJob();

    int errorCode() const;
    QString iconFile() const;
    QUrl requestUrl() const;
    void abort();

Q_SIGNALS:
    void iconReady(FaviconRequestJob *);

private Q_SLOTS:
    void makeRequest();
    void httpFinished();
    void httpReadyRead();
#ifndef QT_NO_SSL
    void sslErrors(QNetworkReply *, const QList<QSslError> &errors);
#endif

private:
    struct FaviconRequestJobPrivate;
    FaviconRequestJobPrivate *const d;
};

Q_DECLARE_LOGGING_CATEGORY(FAVICONREQUESTJOB)

#endif
