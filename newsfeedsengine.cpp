// the following header is required by the LGPL because
// we are using the actual time engine code
/*
 *   Copyright 2016 Michal Kimle <kimle.michal@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2 or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "newsfeedsengine.h"

#include <QUrl>
#include <QString>
#include <QVariant>
#include <QMap>

#include <KLocalizedString>
#include <KIO/FavIconRequestJob>

#include <Syndication/Image>

#define MINIMUM_INTERVAL 5000 // 5 seconds

const std::chrono::minutes NewsFeedsEngine::iconsExpirationTime = std::chrono::minutes(30);

NewsFeedsEngine::NewsFeedsEngine(QObject* parent, const QVariantList& args)
    : Plasma::DataEngine(parent, args)
{
    // We ignore any arguments - data engines do not have much use for them
    Q_UNUSED(args)

    // This prevents applets from setting an unnecessarily high
    // update interval and using too much CPU.
    // In the case of a clock that only has second precision,
    // a third of a second should be more than enough.
    setMinimumPollingInterval(MINIMUM_INTERVAL);

    connect(&networkConfigurationManager, SIGNAL(onlineStateChanged(bool)),
            this, SLOT(networkStatusChanged(bool)));

    iconsExpirationTimer = std::make_unique<QTimer>(this);
        connect(iconsExpirationTimer.get(), SIGNAL(timeout()), this, SLOT(iconsExpired()));
        iconsExpirationTimer->start(iconsExpirationTime);
}

bool NewsFeedsEngine::sourceRequestEvent(const QString &source)
{
    qCDebug(NEWSFEEDSENGINE) << "NewsFeedsEngine::sourceRequestEvent";

    // We do not have any special code to execute the
    // first time a source is requested, so we just call
    // updateSourceEvent().
    setData(source, Data());
    loadingNews.remove(source);
    loadingIcons.remove(source);
    sourcesWithIcon.remove(source);

    updateSourceEvent(source);

    return true;
}

bool NewsFeedsEngine::updateSourceEvent(const QString &source)
{
    qCDebug(NEWSFEEDSENGINE) << "NewsFeedsEngine::updateSourceEvent(source = " << source << ")";

    if (loadingNews.contains(source) || loadingIcons.contains(source)) {
      qCDebug(NEWSFEEDSENGINE) << "Source" << source << "still loading";
      return false;
    }

    // load news
    qCDebug(NEWSFEEDSENGINE) << "Loading news for source" << source;
    Syndication::Loader *loader = Syndication::Loader::create();
    connect(loader, SIGNAL(loadingComplete(Syndication::Loader*,Syndication::FeedPtr,Syndication::ErrorCode)),
            this, SLOT(feedReady(Syndication::Loader*,Syndication::FeedPtr,Syndication::ErrorCode)));

    loaderSourceMap.insert(loader, source);
    loadingNews.insert(source);
    loader->loadFrom(QUrl(source));

    //load icon
    if (!sourcesWithIcon.contains(source)) {
      qCDebug(NEWSFEEDSENGINE) << "Loading icon for source" << source;
      loadingIcons.insert(source);
      KIO::FavIconRequestJob *job = new KIO::FavIconRequestJob(QUrl(source));
      connect(job, SIGNAL(result(KJob*)), this, SLOT(iconReady(KJob*)));
    }

    return false;
}

void NewsFeedsEngine::iconsExpired()
{
    qCDebug(NEWSFEEDSENGINE) << "NewsFeedsEngine::iconExpired";
    sourcesWithIcon.clear();
}

void NewsFeedsEngine::feedReady(Syndication::Loader* loader, Syndication::FeedPtr feed, Syndication::ErrorCode errorCode)
{
    QString source = loaderSourceMap.take(loader);
    loaderSourceMap.remove(loader);
    qCDebug(NEWSFEEDSENGINE) << "NewsFeedsEngine::feedReady(source = " << source << ")";

    if (errorCode != Syndication::Success) {
      setData(source, QStringLiteral("Title"), i18n("Fetching feed failed."));
      setData(source, QStringLiteral("Link"), source);
    } else {
      QString title = feed->title();
      QString link = feed->link();
      QString description = feed->description();
      QString language = feed->language();
      QString copyright = feed->copyright();

      QVariantList authors = getAuthors(feed->authors());
      QVariantList categories = getCategories(feed->categories());
      QVariantList items = getItems(feed->items());

      setData(source, QStringLiteral("Title"), title);
      setData(source, QStringLiteral("Link"), link);
      setData(source, QStringLiteral("Description"), description);
      setData(source, QStringLiteral("Language"), language);
      setData(source, QStringLiteral("Copyright"), copyright);
      setData(source, QStringLiteral("Authors"), authors);
      setData(source, QStringLiteral("Categories"), categories);
      setData(source, QStringLiteral("Items"), items);
    }


    loadingNews.remove(source);
}

void NewsFeedsEngine::iconReady(KJob* kjob)
{
    qCDebug(NEWSFEEDSENGINE) << "NewsFeedsEngine::iconReady";

    KIO::FavIconRequestJob *job = static_cast<KIO::FavIconRequestJob *>(kjob);

    const QString url = job->hostUrl().toString().toLower();
    QString iconFile;
    if (job->error() != 0) {
      qCDebug(NEWSFEEDSENGINE) << "Error during icon download, setting 'NO_ICON' flag";
      iconFile = "NO_ICON";
    } else {
      iconFile = job->iconFile();
    }

    setData(url, QStringLiteral("Image"), iconFile);
    sourcesWithIcon << url;
    loadingIcons.remove(url);
}

QVariantList NewsFeedsEngine::getAuthors(QList<Syndication::PersonPtr> authors)
{
    QVariantList authorsData;
    foreach (const Syndication::PersonPtr &author, authors) {
        QMap<QString, QVariant> authorData;

        if (author->isNull() || (author->name().isNull() && author->email().isNull() && author->uri().isNull())) {
            continue;
        }

        authorData[QStringLiteral("Name")] = author->name();
        authorData[QStringLiteral("Email")] = author->email();
        authorData[QStringLiteral("Uri")] = author->uri();

        authorsData.append(authorData);
    }

    return authorsData;
}

QVariantList NewsFeedsEngine::getCategories(QList<Syndication::CategoryPtr> categories)
{
    QVariantList categoriesData;
    foreach (const Syndication::CategoryPtr &category, categories) {
        QMap<QString, QVariant> categoryData;

        if (category->isNull() || (category->term().isNull() && category->scheme().isNull() && category->label().isNull())) {
            continue;
        }

        categoryData[QStringLiteral("Term")] = category->term();
        categoryData[QStringLiteral("Scheme")] = category->scheme();
        categoryData[QStringLiteral("Label")] = category->label();

        categoriesData.append(categoryData);
    }

    return categoriesData;
}

QVariantList NewsFeedsEngine::getEnclosures(QList<Syndication::EnclosurePtr> enclosures)
{
    QVariantList enclosuresData;
    foreach (const Syndication::EnclosurePtr &enclosure, enclosures) {
        QMap<QString, QVariant> enclosureData;

        if (enclosure->isNull() || (enclosure->url().isNull() && enclosure->title().isNull())) {
            continue;
        }

        enclosureData[QStringLiteral("Url")] = enclosure->url();
        enclosureData[QStringLiteral("Title")] = enclosure->title();
        enclosureData[QStringLiteral("Type")] = enclosure->type();
        enclosureData[QStringLiteral("Length")] = enclosure->length();
        enclosureData[QStringLiteral("Duration")] = enclosure->duration();

        enclosuresData.append(enclosureData);
    }

    return enclosuresData;
}

QVariantList NewsFeedsEngine::getItems(QList<Syndication::ItemPtr> items)
{
    QVariantList itemsData;
    foreach (const Syndication::ItemPtr &item, items) {
        QMap<QString, QVariant> itemData;

        if (item->title().isNull() && item->content().isNull()) {
            continue;
        }

        itemData[QStringLiteral("Title")] = item->title();
        itemData[QStringLiteral("Link")] = item->link();
        itemData[QStringLiteral("Description")] = item->description();
        itemData[QStringLiteral("Content")] = item->content();
        itemData[QStringLiteral("DatePublished")] = (qlonglong) item->datePublished();
        itemData[QStringLiteral("DateUpdated")] = (qlonglong) item->dateUpdated();
        itemData[QStringLiteral("Id")] = item->id();
        itemData[QStringLiteral("Language")] = item->language();
        itemData[QStringLiteral("CommentsCount")] = item->commentsCount();
        itemData[QStringLiteral("CommentsLink")] = item->commentsLink();
        itemData[QStringLiteral("CommentsFeed")] = item->commentsFeed();
        itemData[QStringLiteral("CommentPostUri")] = item->commentPostUri();

        itemData[QStringLiteral("Authors")] = getAuthors(item->authors());
        itemData[QStringLiteral("Enclosures")] = getEnclosures(item->enclosures());
        itemData[QStringLiteral("Categories")] = getCategories(item->categories());

        itemsData.append(itemData);
    }

    return itemsData;
}

void NewsFeedsEngine::networkStatusChanged(bool isOnline)
{
    if (isOnline) {
        // start updating the feeds
        foreach(const QString &feedUrl, sources()) {
            updateSourceEvent(feedUrl);
        }
    }
}

// This does the magic that allows Plasma to load
// this plugin.  The first argument must match
// the X-Plasma-EngineName in the .desktop file.
// The second argument is the name of the class in
// your plugin that derives from Plasma::DataEngine
K_EXPORT_PLASMA_DATAENGINE_WITH_JSON(newsfeeds, NewsFeedsEngine, "plasma-dataengine-newsfeeds.json")
Q_LOGGING_CATEGORY(NEWSFEEDSENGINE, "newsfeedsengine")

#include "newsfeedsengine.moc"
