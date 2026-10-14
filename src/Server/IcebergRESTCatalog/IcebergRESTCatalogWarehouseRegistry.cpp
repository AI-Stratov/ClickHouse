#include <Server/IcebergRESTCatalog/IcebergRESTCatalogWarehouseRegistry.h>

#include <Common/ZooKeeper/ZooKeeper.h>
#include <Common/ZooKeeper/ZooKeeperCommon.h>
#include <Common/logger_useful.h>
#include <Interpreters/Context.h>

namespace DB
{

IcebergRESTCatalogWarehouseRegistry::IcebergRESTCatalogWarehouseRegistry(ContextPtr context_, IcebergRESTCatalogConfig catalog_config_)
    : context(std::move(context_)), catalog_config(std::move(catalog_config_)), log(getLogger("IcebergRESTCatalogWarehouseRegistry"))
{
}

IcebergRESTCatalogWarehousePtr IcebergRESTCatalogWarehouseRegistry::tryGet(const String & name) const
{
    auto component_guard = Coordination::setCurrentComponent("IcebergRESTCatalogWarehouseRegistry::tryGet");
    const auto path = catalog_config.warehousePath(name);
    String data;
    Coordination::Stat stat;
    const bool exists = catalog_config.getZooKeeper(*context)->tryGet(path, data, &stat);

    std::lock_guard lock(mutex);
    auto it = warehouses.find(name);
    if (it != warehouses.end())
    {
        /// The cached object is still current, use it.
        if (exists && it->second.mzxid == stat.mzxid)
            return it->second.warehouse;

        /// Otherwise, remove from cache.
        warehouses.erase(it);
        LOG_INFO(log, "Removed warehouse {}", name);
    }
    if (!exists)
        return nullptr;

    const auto metadata = IcebergRESTCatalogWarehouseMetadata::fromKeeperData(data, path);
    auto warehouse = createIcebergRESTCatalogWarehouse(context, name, metadata, catalog_config);
    warehouses.emplace(name, Entry{warehouse, stat.mzxid});
    LOG_INFO(log, "Added warehouse {}", name);
    return warehouse;
}

}
