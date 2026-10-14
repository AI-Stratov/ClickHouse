#include <Server/IcebergRESTCatalog/IcebergRESTCatalogWarehouseKeeperUtils.h>

#include <Common/Exception.h>
#include <Common/ZooKeeper/KeeperException.h>
#include <Common/ZooKeeper/ZooKeeper.h>
#include <Common/ZooKeeper/ZooKeeperCommon.h>
#include <Common/escapeForFileName.h>

#include <algorithm>

namespace DB
{

namespace IcebergRESTCatalog
{

bool createWarehouseInKeeper(
    const Context & context, const IcebergRESTCatalogConfig & catalog_config, const String & name, const IcebergRESTCatalogWarehouseMetadata & metadata)
{
    auto component_guard = Coordination::setCurrentComponent("createWarehouseInKeeper");
    auto zookeeper = catalog_config.getZooKeeper(context);
    const auto path = catalog_config.warehousePath(name);
    zookeeper->createAncestors(path);

    Coordination::Requests ops;
    ops.emplace_back(zkutil::makeCreateRequest(path, metadata.toKeeperData(), zkutil::CreateMode::Persistent));
    ops.emplace_back(zkutil::makeCreateRequest(path + "/namespaces", "", zkutil::CreateMode::Persistent));
    Coordination::Responses responses;
    const auto code = zookeeper->tryMulti(ops, responses);
    if (code == Coordination::Error::ZNODEEXISTS)
        return false;
    zkutil::KeeperMultiException::check(code, ops, responses);
    return true;
}

DropWarehouseResult dropWarehouseFromKeeper(const Context & context, const IcebergRESTCatalogConfig & catalog_config, const String & name)
{
    auto component_guard = Coordination::setCurrentComponent("dropWarehouseFromKeeper");
    auto zookeeper = catalog_config.getZooKeeper(context);
    const auto path = catalog_config.warehousePath(name);

    /// Keeper refuses to remove a node with children, so a warehouse with namespaces is left untouched.
    Coordination::Requests ops;
    ops.emplace_back(zkutil::makeRemoveRequest(path + "/namespaces", -1));
    ops.emplace_back(zkutil::makeRemoveRequest(path, -1));
    Coordination::Responses responses;
    const auto code = zookeeper->tryMulti(ops, responses);
    if (code == Coordination::Error::ZNONODE)
        return DropWarehouseResult::Missing;
    if (code == Coordination::Error::ZNOTEMPTY)
        return DropWarehouseResult::NotEmpty;
    zkutil::KeeperMultiException::check(code, ops, responses);
    return DropWarehouseResult::Dropped;
}

std::vector<IcebergRESTCatalogWarehouseInfo> listWarehousesInKeeper(const Context & context, const IcebergRESTCatalogConfig & catalog_config)
{
    auto component_guard = Coordination::setCurrentComponent("listWarehousesInKeeper");
    auto zookeeper = catalog_config.getZooKeeper(context);

    /// The root is created by the first `CREATE WAREHOUSE`.
    Strings children;
    if (zookeeper->tryGetChildren(catalog_config.zookeeper_path, children) == Coordination::Error::ZNONODE)
        return {};
    std::sort(children.begin(), children.end());

    Strings paths;
    paths.reserve(children.size());
    for (const auto & child : children)
        paths.push_back(catalog_config.zookeeper_path + "/" + child);
    auto responses = zookeeper->tryGet(paths);

    std::vector<IcebergRESTCatalogWarehouseInfo> result;
    for (size_t i = 0; i < children.size(); ++i)
    {
        const auto & response = responses[i];
        /// Dropped between the list and this read.
        if (response.error == Coordination::Error::ZNONODE)
            continue;
        result.push_back(
            {.name = unescapeForFileName(children[i]),
             .metadata = IcebergRESTCatalogWarehouseMetadata::fromKeeperData(response.data, paths[i])});
    }
    return result;
}

}

}
