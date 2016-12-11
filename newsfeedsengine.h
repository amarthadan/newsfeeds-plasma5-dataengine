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

// a standard include guard to prevent problems if the
// header is included multiple times
#ifndef NEWSFEEDSENGINE_H
#define NEWSFEEDSENGINE_H

#include <Plasma/DataEngine>

#include <QNetworkConfigurationManager>
#include <QHash>
#include <QVariantList>
#include <QList>

#include <Syndication/Loader>
#include <Syndication/Feed>
#include <Syndication/Item>
#include <Syndication/Person>
#include <Syndication/Category>
#include <Syndication/Enclosure>
#include <Syndication/Global>

#include <KJob>

/**
 * This engine provides Atom and RSS news feeds in a unified way
 */
class NewsFeedsEngine: public Plasma::DataEngine
{
    Q_OBJECT

public:
// every engine needs a constructor with these arguments
    NewsFeedsEngine(QObject* parent, const QVariantList &args);

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
    void feedReady(Syndication::Loader* loader,
                   Syndication::FeedPtr feed,
                   Syndication::ErrorCode errorCode);
    void iconReady(KJob* kjob);

private:
    QHash<Syndication::Loader*, QString> loaderSourceMap;
    QNetworkConfigurationManager networkConfigurationManager;

    QVariantList getAuthors(QList<Syndication::PersonPtr> authors);
    QVariantList getCategories(QList<Syndication::CategoryPtr> categories);
    QVariantList getItems(QList<Syndication::ItemPtr> items);
    QVariantList getEnclosures(QList<Syndication::EnclosurePtr> enclosures);
};

#endif // NEWSFEEDSENGINE_H
