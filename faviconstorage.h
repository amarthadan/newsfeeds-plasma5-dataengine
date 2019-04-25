#ifndef FAVICONSTORAGE_H
#define FAVICONSTORAGE_H

#include <QObject>
#include <QByteArray>
#include <QUrl>
#include <QLoggingCategory>

class FavIconStorage : public QObject
{
    Q_OBJECT

public:
    FavIconStorage();

    QString saveIcon(QByteArray *data, const QUrl &url);

private:
    void ensureStorageExists();
    QString storagePathForIconUrl(const QUrl &url);

    QString storageDir;
};

Q_DECLARE_LOGGING_CATEGORY(FAVICONSTORAGE)

#endif // FAVICONSTORAGE_H
