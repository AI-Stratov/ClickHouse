#pragma once

#include <base/types.h>
#include <Common/SettingsChanges.h>

#include <map>

namespace DB
{

/// What `CREATE WAREHOUSE` stores in Keeper (RFC: issue #114697).
struct IcebergRESTCatalogWarehouseMetadata
{
    /// Default storage prefix for tables, without trailing slashes.
    String base_location;
    /// Named collection with the S3 credentials of the bucket.
    String storage_named_collection;

    static IcebergRESTCatalogWarehouseMetadata fromSettings(const SettingsChanges & changes);

    /// Data of the warehouse root node: `{"format": ..., "settings": {...}}`.
    String toKeeperData() const;
    static IcebergRESTCatalogWarehouseMetadata fromKeeperData(const String & data, const String & path);

    std::map<String, String> toSettingsMap() const;
};

void validateIcebergRESTCatalogWarehouseName(const String & name);

}
