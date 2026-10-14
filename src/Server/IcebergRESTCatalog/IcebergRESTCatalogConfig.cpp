#include <Server/IcebergRESTCatalog/IcebergRESTCatalogConfig.h>

#include <Common/Exception.h>
#include <Common/ZooKeeper/ZooKeeper.h>
#include <Common/ZooKeeper/ZooKeeperPathUtils.h>
#include <Common/escapeForFileName.h>
#include <Interpreters/Context.h>

#include <Poco/Util/AbstractConfiguration.h>

#include <filesystem>

namespace DB
{

namespace ErrorCodes
{
    extern const int INVALID_CONFIG_PARAMETER;
    extern const int SUPPORT_IS_DISABLED;
}

namespace
{

constexpr auto CONFIG_SECTION = "iceberg_rest_catalog";

}

String IcebergRESTCatalogConfig::warehousePath(const String & warehouse_name) const
{
    return (std::filesystem::path(zookeeper_path) / escapeForFileName(warehouse_name)).string();
}

zkutil::ZooKeeperPtr IcebergRESTCatalogConfig::getZooKeeper(const Context & context) const
{
    return context.getDefaultOrAuxiliaryZooKeeper(zookeeper_name);
}

std::optional<IcebergRESTCatalogConfig> tryGetIcebergRESTCatalogConfig(const Context & context)
{
    const auto & config = context.getConfigRef();
    if (!config.has(CONFIG_SECTION))
        return std::nullopt;

    const auto zookeeper_path_with_name
        = config.getString(fmt::format("{}.zookeeper_path", CONFIG_SECTION), "/clickhouse/iceberg_rest_catalog");

    IcebergRESTCatalogConfig result;
    result.zookeeper_name = zkutil::extractZooKeeperName(zookeeper_path_with_name);
    result.zookeeper_path = zkutil::extractZooKeeperPath(zookeeper_path_with_name, /*check_starts_with_slash*/ true);

    if (result.zookeeper_name == zkutil::DEFAULT_ZOOKEEPER_NAME)
    {
        if (!context.hasZooKeeper())
            throw Exception(ErrorCodes::INVALID_CONFIG_PARAMETER, "The catalog state is stored in Keeper, but no <zookeeper> section is configured");
    }
    else if (!context.hasAuxiliaryZooKeeper(result.zookeeper_name))
    {
        throw Exception(
            ErrorCodes::INVALID_CONFIG_PARAMETER,
            "The catalog state is stored in auxiliary Keeper '{}', but it is not configured in <auxiliary_zookeepers>",
            result.zookeeper_name);
    }
    return result;
}

IcebergRESTCatalogConfig getIcebergRESTCatalogConfig(const Context & context)
{
    auto result = tryGetIcebergRESTCatalogConfig(context);
    if (!result)
        throw Exception(ErrorCodes::SUPPORT_IS_DISABLED, "The Iceberg REST catalog is not configured on this server");
    return *result;
}

}
