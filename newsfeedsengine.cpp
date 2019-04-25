#include "newsfeedsengine.h"

#include "fileretriever.h"

#include <Syndication/Image>
#include <Syndication/DataRetriever>

#include <KLocalizedString>

#include <QUrl>
#include <QString>
#include <QVariant>
#include <QMap>

#define MINIMUM_INTERVAL 5000 // 5 seconds

NewsFeedsEngine::NewsFeedsEngine(QObject* parent, const QVariantList& args)
    : Plasma::DataEngine(parent, args), networkConfigurationManager(this)
{
    // We ignore any arguments - data engines do not have much use for them
    Q_UNUSED(args)

    // This prevents applets from setting an unnecessarily high
    // update interval and using too much CPU.
    setMinimumPollingInterval(MINIMUM_INTERVAL);

    connect(&networkConfigurationManager, &QNetworkConfigurationManager::onlineStateChanged,
            this, &NewsFeedsEngine::networkStatusChanged);
}

NewsFeedsEngine::~NewsFeedsEngine()
{
    qCDebug(NEWSFEEDSENGINE) << "~NewsFeedsEngine";
}

bool NewsFeedsEngine::sourceRequestEvent(const QString &source)
{
    qCDebug(NEWSFEEDSENGINE) << "NewsFeedsEngine::sourceRequestEvent(source =" << source << ")";

    setData(source, Data());

    Syndication::Loader * loader = loadingNews.take(source);
    if (loader != nullptr) {
      loader->abort();
    }

    FaviconRequestJob *job = loadingIcons.take(source);
    if (job != nullptr) {
      job->abort();
    }

    updateSourceEvent(source);

    return true;
}

bool NewsFeedsEngine::updateSourceEvent(const QString &source)
{
    qCDebug(NEWSFEEDSENGINE) << "NewsFeedsEngine::updateSourceEvent(source =" << source << ")";

    if (loadingNews.contains(source) || loadingIcons.contains(source)) {
        qCDebug(NEWSFEEDSENGINE) << "Source" << source << "still loading";
        return false;
    }

    // load news
    qCDebug(NEWSFEEDSENGINE) << "Loading news for source" << source;

    Syndication::Loader *loader = Syndication::Loader::create();
    loadingNews.insert(source, loader);
    connect(loader, &Syndication::Loader::loadingComplete, this,
            [this, source](Syndication::Loader* l, Syndication::FeedPtr fp, Syndication::ErrorCode ec)
            {
                feedReady(std::move(source), l, std::move(fp), std::move(ec));
            });
    loader->loadFrom(QUrl(source), new FileRetriever);

    //load icon
    qCDebug(NEWSFEEDSENGINE) << "Loading icon for source" << source;

    FaviconRequestJob *job = new FaviconRequestJob(QUrl(source));
    loadingIcons.insert(source, job);
    connect(job, &FaviconRequestJob::iconReady, this,
            [this, source](FaviconRequestJob* job)
            {
                iconReady(std::move(source), job);
            });

    return false;
}

void NewsFeedsEngine::feedReady(QString source, Syndication::Loader* /*loader*/, Syndication::FeedPtr feed, Syndication::ErrorCode errorCode)
{
    qCDebug(NEWSFEEDSENGINE) << "NewsFeedsEngine::feedReady(source =" << source << ")";

    if (errorCode != Syndication::Success) {
        qCWarning(NEWSFEEDSENGINE) << "Fetching feed" << source << "failed";
        setData(source, QStringLiteral("Title"), i18n("Fetching feed failed."));
        setData(source, QStringLiteral("Link"), source);
    } else {
        setData(source, QStringLiteral("Title"),       feed->title());
        setData(source, QStringLiteral("Link"),        feed->link());
        setData(source, QStringLiteral("Description"), feed->description());
        setData(source, QStringLiteral("Language"),    feed->language());
        setData(source, QStringLiteral("Copyright"),   feed->copyright());
        setData(source, QStringLiteral("Authors"),     getAuthors(feed->authors()));
        setData(source, QStringLiteral("Categories"),  getCategories(feed->categories()));
        setData(source, QStringLiteral("Items"),       getItems(feed->items()));
    }

    loadingNews.remove(source);
}

void NewsFeedsEngine::iconReady(QString source, FaviconRequestJob* job)
{
    qCDebug(NEWSFEEDSENGINE) << "NewsFeedsEngine::iconReady(source =" << source << ")";

    QString iconFile;

    if (job->errorCode() != 0) {
        qCWarning(NEWSFEEDSENGINE) << "Error during icon download for" << source << ".";
    } else {
        iconFile = job->iconFile();
        setData(source, QStringLiteral("Image"), iconFile);
    }

    loadingIcons.remove(source);
}

QVariantList NewsFeedsEngine::getAuthors(QList<Syndication::PersonPtr> authors)
{
    QVariantList authorsData;
    for (const auto& a: authors) {
        if (a->isNull() || (a->name().isNull() && a->email().isNull() && a->uri().isNull())) {
            continue;
        }

        QMap<QString, QVariant> authorData;

        authorData[QStringLiteral("Name")] = a->name();
        authorData[QStringLiteral("Email")] = a->email();
        authorData[QStringLiteral("Uri")] = a->uri();

        authorsData.append(authorData);
    }

    return authorsData;
}

QVariantList NewsFeedsEngine::getCategories(QList<Syndication::CategoryPtr> categories)
{
    QVariantList categoriesData;
    for(const auto& category: categories) {
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
    for(const auto& enclosure: enclosures) {
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
    for (const auto& item: items) {
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
        for(const auto& feedUrl: sources()) {
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
