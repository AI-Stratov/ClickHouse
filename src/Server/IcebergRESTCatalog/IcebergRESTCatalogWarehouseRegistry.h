#pragma once

#include <base/types.h>
#include <Common/Logger.h>
#include <Interpreters/Context_fwd.h>
#include <Server/IcebergRESTCatalog/IcebergRESTCatalogWarehouse.h>
#include <Server/IcebergRESTCatalog/IcebergRESTCatalogConfig.h>

#include <map>
#include <memory>
#include <mutex>

namespace DB
{

/// Registry of the warehouses stored in Keeper. Every lookup reads the warehouse root node, so a
/// `CREATE WAREHOUSE` or `DROP WAREHOUSE` on any server is visible on the next request.
class IcebergRESTCatalogWarehouseRegistry
{
public:
    IcebergRESTCatalogWarehouseRegistry(ContextPtr context_, IcebergRESTCatalogConfig catalog_config_);

    /// Returns nullptr if the warehouse does not exist.
    IcebergRESTCatalogWarehousePtr tryGet(const String & name) const;

private:
    struct Entry
    {
        IcebergRESTCatalogWarehousePtr warehouse;
        /// Last modification of the root node the object was built from.
        Int64 mzxid = 0;
    };

    const ContextPtr context;
    const IcebergRESTCatalogConfig catalog_config;
    const LoggerPtr log;

    mutable std::mutex mutex;
    mutable std::map<String, Entry> warehouses TSA_GUARDED_BY(mutex);
};

using IcebergRESTCatalogWarehouseRegistryPtr = std::shared_ptr<const IcebergRESTCatalogWarehouseRegistry>;

}
