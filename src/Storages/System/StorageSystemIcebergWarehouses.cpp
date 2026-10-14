#include <Storages/System/StorageSystemIcebergWarehouses.h>
#include <Storages/System/SystemTableSourceRegistry.h>

#include <Access/Common/AccessType.h>
#include <Access/ContextAccess.h>
#include <DataTypes/DataTypeMap.h>
#include <DataTypes/DataTypeString.h>
#include <Interpreters/Context.h>
#include <Server/IcebergRESTCatalog/IcebergRESTCatalogWarehouseKeeperUtils.h>

namespace DB
{

ColumnsDescription StorageSystemIcebergWarehouses::getColumnsDescription()
{
    return ColumnsDescription
    {
        {"name", std::make_shared<DataTypeString>(), "Name of the warehouse."},
        {"settings", std::make_shared<DataTypeMap>(std::make_shared<DataTypeString>(), std::make_shared<DataTypeString>()), "Settings given to CREATE WAREHOUSE."},
    };
}

StorageSystemIcebergWarehouses::StorageSystemIcebergWarehouses(const StorageID & table_id_)
    : IStorageSystemOneBlock(table_id_, getColumnsDescription())
{
}

void StorageSystemIcebergWarehouses::fillData(MutableColumns & res_columns, ContextPtr context, const ActionsDAG::Node *, std::vector<UInt8>) const
{
    if (!context->getAccess()->isGranted(AccessType::SHOW_WAREHOUSES))
        return;

    /// Keeper is the source of truth, so the table is the same on every server of the catalog.
    const auto catalog_config = tryGetIcebergRESTCatalogConfig(*context);
    if (!catalog_config)
        return;

    for (const auto & warehouse : IcebergRESTCatalog::listWarehousesInKeeper(*context, *catalog_config))
    {
        Map settings;
        for (const auto & [key, value] : warehouse.metadata.toSettingsMap())
            settings.push_back(Tuple{key, value});

        size_t col = 0;
        res_columns[col++]->insert(warehouse.name);
        res_columns[col++]->insert(settings);
    }
}

}

/// Register the source file of this system table for `system.documentation`.
namespace DB { REGISTER_SYSTEM_TABLE_SOURCE(StorageSystemIcebergWarehouses) }
