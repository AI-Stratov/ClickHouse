#pragma once

#include <base/types.h>
#include <Interpreters/Context_fwd.h>
#include <Server/IcebergRESTCatalog/IcebergRESTCatalogConfig.h>
#include <Server/IcebergRESTCatalog/IcebergRESTCatalogWarehouseMetadata.h>

#include <vector>

namespace DB
{

struct IcebergRESTCatalogWarehouseInfo
{
    String name;
    IcebergRESTCatalogWarehouseMetadata metadata;
};

/// Create, drop and list of the warehouse nodes under the catalog root.
/// The layout below a warehouse node is described in `KeeperIcebergRESTCatalogStore.h`.
namespace IcebergRESTCatalog
{

/// Returns false if warehouse already exists.
bool createWarehouseInKeeper(
    const Context & context, const IcebergRESTCatalogConfig & catalog_config, const String & name, const IcebergRESTCatalogWarehouseMetadata & metadata);

enum class DropWarehouseResult
{
    Dropped,
    Missing,
    NotEmpty,
};

/// Removes the warehouse only if it has no namespaces.
DropWarehouseResult dropWarehouseFromKeeper(const Context & context, const IcebergRESTCatalogConfig & catalog_config, const String & name);

std::vector<IcebergRESTCatalogWarehouseInfo> listWarehousesInKeeper(const Context & context, const IcebergRESTCatalogConfig & catalog_config);

}

}
