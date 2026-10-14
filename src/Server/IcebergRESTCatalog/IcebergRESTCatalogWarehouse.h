#pragma once

#include <base/types.h>
#include <Disks/DiskObjectStorage/ObjectStorages/IObjectStorage_fwd.h>
#include <Interpreters/Context_fwd.h>
#include <Server/IcebergRESTCatalog/IcebergRESTCatalogConfig.h>
#include <Server/IcebergRESTCatalog/IcebergRESTCatalogWarehouseMetadata.h>

#include <memory>

namespace DB
{

class KeeperIcebergRESTCatalogStore;
using KeeperIcebergRESTCatalogStorePtr = std::shared_ptr<KeeperIcebergRESTCatalogStore>;

/// One warehouse of the Iceberg REST catalog (RFC: issue #114697).
struct IcebergRESTCatalogWarehouse
{
    IcebergRESTCatalogWarehouse(
        String name_, String base_location_, KeeperIcebergRESTCatalogStorePtr store_, ObjectStoragePtr object_storage_);

    /// Also the REST `prefix`.
    const String name;
    /// Default storage prefix for tables.
    const String base_location;
    const KeeperIcebergRESTCatalogStorePtr store;
    const ObjectStoragePtr object_storage;

    /// Whether the S3 client of this warehouse can address `location`.
    bool isInBucket(const String & location) const;
    /// Throws `INCORRECT_DATA`. Callers validate client input with `isInBucket` first.
    String objectKey(const String & location) const;
};

using IcebergRESTCatalogWarehousePtr = std::shared_ptr<const IcebergRESTCatalogWarehouse>;

IcebergRESTCatalogWarehousePtr createIcebergRESTCatalogWarehouse(
    ContextPtr context, const String & name, const IcebergRESTCatalogWarehouseMetadata & metadata, const IcebergRESTCatalogConfig & catalog_config);

}
