#include "faviconstorage.h"

#include <QDir>
#include <QFile>
#include <QStandardPaths>
#include <QBuffer>
#include <QIODevice>
#include <QImageReader>
#include <QImage>
#include <QSize>
#include <QSaveFile>

static QString portForUrl(const QUrl &url)
{
    if (url.port() > 0) {
        return QLatin1Char('_') + QString::number(url.port());
    }
    return QString();
}

static QString simplifyUrl(const QUrl &url)
{
    // splat any = in the URL so it can be safely used as a config key
    QString result = url.host() + portForUrl(url) + url.path();
    result.replace(QLatin1Char('='), QLatin1Char('_'));
    while (result.endsWith(QLatin1Char('/'))) {
        result.chop(1);
    }
    return result;
}

static QString iconNameFromUrl(const QUrl &iconUrl)
{
    if (iconUrl.path() == QLatin1String("/favicon.ico")) {
        return iconUrl.host() + portForUrl(iconUrl);
    }

    QString result = simplifyUrl(iconUrl);
    // splat / so it can be safely used as a file name
    result.replace(QLatin1Char('/'), QLatin1Char('_'));

    const QStringRef ext = result.rightRef(4);
    if (ext == QLatin1String(".ico") || ext == QLatin1String(".png") || ext == QLatin1String(".xpm")) {
        result.chop(4);
    }

    return result;
}

FavIconStorage::FavIconStorage()
    : storageDir(QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation) + QStringLiteral("/favicons/"))
{
}

QString FavIconStorage::saveIcon(QByteArray *data, const QUrl &url)
{
    QString iconFile;
    QBuffer buffer(data);
    buffer.open(QIODevice::ReadOnly);
    QImageReader ir(&buffer);
    QSize desired(16, 16);
    if (ir.canRead()) {
        while (ir.imageCount() > 1
                && ir.currentImageRect() != QRect(0, 0, desired.width(), desired.height())) {
            if (!ir.jumpToNextImage()) {
                break;
            }
        }
        ir.setScaledSize(desired);
        const QImage img = ir.read();
        if (!img.isNull()) {
            ensureStorageExists();
            const QString localPath = storagePathForIconUrl(url);
            qCDebug(FAVICONSTORAGE) << "Saving image to" << localPath;
            QSaveFile saveFile(localPath);
            if (saveFile.open(QIODevice::WriteOnly) && img.save(&saveFile, "PNG") && saveFile.commit()) {
                iconFile = localPath;
            } else {
                qCCritical(FAVICONSTORAGE) << "Couldn't write file" << localPath;
            }
        } else {
            qCCritical(FAVICONSTORAGE) << "QImageReader read() returned a null image";
        }
    } else {
        qCCritical(FAVICONSTORAGE) << "QImageReader canRead returned false";
    }

    return iconFile;
}

QString FavIconStorage::storagePathForIconUrl(const QUrl &url)
{
    const QString iconName = iconNameFromUrl(url);
    return storageDir + iconName  + QLatin1String(".png");
}

void FavIconStorage::ensureStorageExists()
{
    QDir().mkpath(storageDir);
}

Q_LOGGING_CATEGORY(FAVICONSTORAGE, "faviconstorage")
