// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#include "settingsdatabase.h"

#include <QDir>
#include <QMap>
#include <QString>
#include <QStringList>
#include <QVariant>

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QDebug>

/*!
    \class Core::SettingsDatabase
    \inheaderfile coreplugin/settingsdatabase.h
    \inmodule QtCreator

    \brief The SettingsDatabase class offers an alternative to the
    application-wide QSettings that is more
    suitable for storing large amounts of data.

    The settings database is SQLite based, and lazily retrieves data when it
    is asked for. It also does incremental updates of the database rather than
    rewriting the whole file each time one of the settings change.

    The SettingsDatabase API mimics that of QSettings.
*/

using namespace Core;
using namespace Core::Internal;

enum { debug_settings = 0 };

namespace Core {
namespace Internal {

using SettingsMap = QMap<QString, QVariant>;

class SettingsDatabasePrivate
{
public:
    QString effectiveGroup() const
    {
        return m_groups.join(QString(QLatin1Char('/')));
    }

    QString effectiveKey(const QString &key) const
    {
        QString g = effectiveGroup();
        if (!g.isEmpty() && !key.isEmpty())
            g += QLatin1Char('/');
        g += key;
        return g;
    }

    SettingsMap m_settings;

    QStringList m_groups;
    QStringList m_dirtyKeys;

    QSqlDatabase m_db;
};

} // namespace Internal
} // namespace Core

SettingsDatabase::SettingsDatabase(const QString &path,
                                   const QString &application,
                                   QObject *parent)
    : QObject(parent)
    , d(new SettingsDatabasePrivate)
{
    const QLatin1Char slash('/');

    // TODO: Don't rely on a path, but determine automatically
    QDir pathDir(path);
    if (!pathDir.exists())
        pathDir.mkpath(pathDir.absolutePath());

    QString fileName = path;
    if (!fileName.endsWith(slash))
        fileName += slash;
    fileName += application;
    fileName += QLatin1String(".db");

    d->m_db = QSqlDatabase::addDatabase(QLatin1String("QSQLITE"), QLatin1String("settings"));
    d->m_db.setDatabaseName(fileName);
    if (!d->m_db.open()) {
        qWarning().nospace() << "Warning: Failed to open settings database at " << fileName << " ("
                             << d->m_db.lastError().driverText() << ")";
    } else {
        // Create the settings table if it doesn't exist yet
        QSqlQuery query(d->m_db);
        query.prepare(QLatin1String("CREATE TABLE IF NOT EXISTS settings ("
                                    "key PRIMARY KEY ON CONFLICT REPLACE, "
                                    "value)"));
        if (!query.exec())
            qWarning().nospace() << "Warning: Failed to prepare settings database! ("
                                 << query.lastError().driverText() << ")";

        // Retrieve all available keys (values are retrieved lazily)
        if (query.exec(QLatin1String("SELECT key FROM settings"))) {
            while (query.next()) {
                d->m_settings.insert(query.value(0).toString(), QVariant());
            }
        }

        // syncing can be slow, especially on Linux and Windows
        d->m_db.exec(QLatin1String("PRAGMA synchronous = OFF;"));
    }
}

SettingsDatabase::~SettingsDatabase()
{
    sync();

    delete d;
    QSqlDatabase::removeDatabase(QLatin1String("settings"));
}

void SettingsDatabase::setValue(const QString &key, const QVariant &value)
{
    const QString effectiveKey = d->effectiveKey(key);

    // Add to cache
    d->m_settings.insert(effectiveKey, value);

    if (!d->m_db.isOpen())
        return;

    // Instant apply (TODO: Delay writing out settings)
    QSqlQuery query(d->m_db);
    query.prepare(QLatin1String("INSERT INTO settings VALUES (?, ?)"));
    query.addBindValue(effectiveKey);
    query.addBindValue(value);
    query.exec();

    if (debug_settings)
        qDebug() << "Stored:" << effectiveKey << "=" << value;
}

QVariant SettingsDatabase::value(const QString &key, const QVariant &defaultValue) const
{
    const QString effectiveKey = d->effectiveKey(key);
    QVariant value = defaultValue;

    SettingsMap::const_iterator i = d->m_settings.constFind(effectiveKey);
    if (i != d->m_settings.constEnd() && i.value().isValid()) {
        value = i.value();
    } else if (d->m_db.isOpen()) {
        // Try to read the value from the database
        QSqlQuery query(d->m_db);
        query.prepare(QLatin1String("SELECT value FROM settings WHERE key = ?"));
        query.addBindValue(effectiveKey);
        query.exec();
        if (query.next()) {
            value = query.value(0);

            if (debug_settings)
                qDebug() << "Retrieved:" << effectiveKey << "=" << value;
        }

        // Cache the result
        d->m_settings.insert(effectiveKey, value);
    }

    return value;
}

bool SettingsDatabase::contains(const QString &key) const
{
    // check exact key
    // this already caches the value
    if (value(key).isValid())
        return true;
    // check for group
    if (d->m_db.isOpen()) {
        const QString glob = d->effectiveKey(key) + "/?*";
        QSqlQuery query(d->m_db);
        query.prepare(
            QLatin1String("SELECT value FROM settings WHERE key GLOB '%1' LIMIT 1").arg(glob));
        query.exec();
        if (query.next())
            return true;
    }
    return false;
}

void SettingsDatabase::remove(const QString &key)
{
    const QString effectiveKey = d->effectiveKey(key);

    // Remove keys from the cache
    const QStringList keys = d->m_settings.keys();
    for (const QString &k : keys) {
        // Either it's an exact match, or it matches up to a /
        if (k.startsWith(effectiveKey)
            && (k.length() == effectiveKey.length()
                || k.at(effectiveKey.length()) == QLatin1Char('/')))
        {
            d->m_settings.remove(k);
        }
    }

    if (!d->m_db.isOpen())
        return;

    // Delete keys from the database
    QSqlQuery query(d->m_db);
    query.prepare(QLatin1String("DELETE FROM settings WHERE key = ? OR key LIKE ?"));
    query.addBindValue(effectiveKey);
    query.addBindValue(QString(effectiveKey + QLatin1String("/%")));
    query.exec();
}

void SettingsDatabase::beginGroup(const QString &prefix)
{
    d->m_groups.append(prefix);
}

void SettingsDatabase::endGroup()
{
    d->m_groups.removeLast();
}

QString SettingsDatabase::group() const
{
    return d->effectiveGroup();
}

QStringList SettingsDatabase::childKeys() const
{
    QStringList children;

    const QString g = group();
    for (auto i = d->m_settings.cbegin(), end = d->m_settings.cend(); i != end; ++i) {
        const QString &key = i.key();
        if (key.startsWith(g) && key.indexOf(QLatin1Char('/'), g.length() + 1) == -1)
            children.append(key.mid(g.length() + 1));
    }

    return children;
}

void SettingsDatabase::beginTransaction()
{
    if (!d->m_db.isOpen())
        return;
    d->m_db.transaction();
}

void SettingsDatabase::endTransaction()
{
    if (!d->m_db.isOpen())
        return;
    d->m_db.commit();
}

void SettingsDatabase::sync()
{
    // TODO: Delay writing of dirty keys and save them here
}
