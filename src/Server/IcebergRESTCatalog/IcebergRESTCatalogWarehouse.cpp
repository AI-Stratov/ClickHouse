#include <Server/IcebergRESTCatalog/IcebergRESTCatalogWarehouse.h>

#include "config.h"

#include <Common/Exception.h>
#include <Disks/DiskObjectStorage/ObjectStorages/IObjectStorage.h>
#include <Interpreters/Context.h>
#include <Parsers/ASTIdentifier.h>
#include <Server/IcebergRESTCatalog/IcebergRESTCatalogLocation.h>
#include <Server/IcebergRESTCatalog/KeeperIcebergRESTCatalogStore.h>
#include <Storages/ObjectStorage/StorageObjectStorageConfiguration.h>

#if USE_AWS_S3
#include <Storages/ObjectStorage/S3/Configuration.h>
#endif

#include <fmt/format.h>


namespace DB
{

namespace ErrorCodes
{
    extern const int INCORRECT_DATA;
    extern const int BAD_ARGUMENTS;
    extern const int SUPPORT_IS_DISABLED;
}

namespace
{

/// S3 client of a warehouse, built from its named collection.
/// The client is built once per warehouse, so a later `ALTER NAMED COLLECTION` applies only after a server restart.
/// TODO: On each request, hash the current content of the named collection and rebuild the client when the hash changed.
ObjectStoragePtr createObjectStorage([[maybe_unused]] ContextPtr context, [[maybe_unused]] const String & storage_named_collection)
{
#if USE_AWS_S3
    auto configuration = std::make_shared<StorageS3Configuration>();
    ASTs args;
    args.push_back(make_intrusive<ASTIdentifier>(storage_named_collection));
    StorageObjectStorageConfiguration::initialize(*configuration, args, context, /*with_table_structure*/ false);

    return configuration->createObjectStorage(context, /*is_readonly*/ false, std::nullopt);
#else
    throw Exception(ErrorCodes::SUPPORT_IS_DISABLED, "The Iceberg REST catalog needs S3 support, but this build has none");
#endif
}

}

IcebergRESTCatalogWarehouse::IcebergRESTCatalogWarehouse(
    String name_, String base_location_, KeeperIcebergRESTCatalogStorePtr store_, ObjectStoragePtr object_storage_)
    : name(std::move(name_)), base_location(std::move(base_location_)), store(std::move(store_)), object_storage(std::move(object_storage_))
{
    /// The server has credentials for one bucket only, so the default table location must be in it.
    if (!isInBucket(base_location))
        throw Exception(
            ErrorCodes::BAD_ARGUMENTS,
            "base_location {} of warehouse {} is not inside bucket '{}' of its object storage",
            base_location,
            name,
            object_storage->getObjectsNamespace());
}

bool IcebergRESTCatalogWarehouse::isInBucket(const String & location) const
{
    const auto parsed = IcebergRESTCatalog::parseS3Location(location);
    return parsed && parsed->bucket == object_storage->getObjectsNamespace();
}

String IcebergRESTCatalogWarehouse::objectKey(const String & location) const
{
    const auto parsed = IcebergRESTCatalog::parseS3Location(location);
    if (!parsed || parsed->bucket != object_storage->getObjectsNamespace())
        throw Exception(ErrorCodes::INCORRECT_DATA, "Location {} is outside the bucket of warehouse {}", location, name);
    return parsed->key;
}

IcebergRESTCatalogWarehousePtr createIcebergRESTCatalogWarehouse(
    ContextPtr context, const String & name, const IcebergRESTCatalogWarehouseMetadata & metadata, const IcebergRESTCatalogConfig & catalog_config)
{
    auto keeper_store = std::make_shared<KeeperIcebergRESTCatalogStore>(
        [context, catalog_config] { return catalog_config.getZooKeeper(*context); },
        catalog_config.warehousePath(name));

    auto object_storage = createObjectStorage(context, metadata.storage_named_collection);

    return std::make_shared<const IcebergRESTCatalogWarehouse>(name, metadata.base_location, std::move(keeper_store), std::move(object_storage));
}

}
