// a standard include guard to prevent problems if the
// header is included multiple times
#ifndef NEWSFEEDSENGINE_H
#define NEWSFEEDSENGINE_H

#include <Plasma/DataEngine>

#include <Syndication/Loader>
#include <Syndication/Feed>
#include <Syndication/Item>
#include <Syndication/Person>
#include <Syndication/Category>
#include <Syndication/Enclosure>
#include <Syndication/Global>

#include <KJob>

#include <QNetworkConfigurationManager>
#include <QHash>
#include <QVariantList>
#include <QSignalMapper>
#include <QSet>
#include <QLoggingCategory>
#include <QTimer>

#include <chrono>
#include <memory>
/**
 * This engine provides Atom and RSS news feeds in an unified way
 */
class NewsFeedsEngine: public Plasma::DataEngine
{
    Q_OBJECT

public:
// every engine needs a constructor with these arguments
    NewsFeedsEngine(QObject* parent, const QVariantList &args);
    ~NewsFeedsEngine();

protected:
// this virtual function is called when a new source is requested
    bool sourceRequestEvent(const QString& source) override;

// this virtual function is called when an automatic update
// is triggered for an existing source (ie: when a valid update
// interval is set when requesting a source)
protected Q_SLOTS:
    bool updateSourceEvent(const QString& source) override;

private Q_SLOTS:
    void networkStatusChanged(bool isOnline);
    void feedReady(QString source,
                   Syndication::Loader* loader,
                   Syndication::FeedPtr feed,
                   Syndication::ErrorCode errorCode);
    void iconReady(QString source, KJob* kjob);
    void iconExpired(QString source);

private:
    QSet<QString> loadingNews;
    QSet<QString> loadingIcons;
    QSet<QString> sourcesWithIcon;
    QNetworkConfigurationManager networkConfigurationManager;

    QVariantList getAuthors(QList<Syndication::PersonPtr> authors);
    QVariantList getCategories(QList<Syndication::CategoryPtr> categories);
    QVariantList getItems(QList<Syndication::ItemPtr> items);
    QVariantList getEnclosures(QList<Syndication::EnclosurePtr> enclosures);

    static const std::chrono::minutes iconsExpirationTime;
};

Q_DECLARE_LOGGING_CATEGORY(NEWSFEEDSENGINE)

#endif // NEWSFEEDSENGINE_H
