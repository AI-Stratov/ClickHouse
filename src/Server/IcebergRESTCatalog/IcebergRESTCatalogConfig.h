#pragma once

#include <base/types.h>
#include <Common/ZooKeeper/Common.h>
#include <Interpreters/Context_fwd.h>

#include <optional>

namespace DB
{

/// Info from `iceberg_rest_catalog` section of server config.
struct IcebergRESTCatalogConfig
{
    /// Typically `default`. `aux` for `zookeeper_path` = `aux:/path`.
    String zookeeper_name;
    /// Parent node of all warehouse nodes.
    String zookeeper_path;

    String warehousePath(const String & warehouse_name) const;

    /// The Keeper session that holds the catalog state.
    zkutil::ZooKeeperPtr getZooKeeper(const Context & context) const;
};

std::optional<IcebergRESTCatalogConfig> tryGetIcebergRESTCatalogConfig(const Context & context);
IcebergRESTCatalogConfig getIcebergRESTCatalogConfig(const Context & context);

}
