#include <Interpreters/InterpreterCreateWarehouseQuery.h>

#include <Access/ContextAccess.h>
#include <Disks/DiskObjectStorage/ObjectStorages/IObjectStorage.h>
#include <Interpreters/Context.h>
#include <Interpreters/InterpreterFactory.h>
#include <Parsers/ASTCreateWarehouseQuery.h>
#include <Parsers/ASTSetQuery.h>
#include <Server/IcebergRESTCatalog/IcebergRESTCatalogWarehouse.h>
#include <Server/IcebergRESTCatalog/IcebergRESTCatalogWarehouseKeeperUtils.h>

namespace DB
{

namespace ErrorCodes
{
    extern const int WAREHOUSE_ALREADY_EXISTS;
}

BlockIO InterpreterCreateWarehouseQuery::execute()
{
    const auto & query = query_ptr->as<const ASTCreateWarehouseQuery &>();
    auto context = getContext();

    context->checkAccess(AccessType::CREATE_WAREHOUSE);

    validateIcebergRESTCatalogWarehouseName(query.warehouse_name);
    const auto metadata = IcebergRESTCatalogWarehouseMetadata::fromSettings(query.settings->as<const ASTSetQuery &>().changes);
    const auto catalog_config = getIcebergRESTCatalogConfig(*context);

    /// Probe the bucket now, so a wrong named collection or location fails here and not on the first REST request.
    const auto warehouse = createIcebergRESTCatalogWarehouse(context, query.warehouse_name, metadata, catalog_config);
    RelativePathsWithMetadata children;
    warehouse->object_storage->listObjects(warehouse->objectKey(metadata.base_location), children, /*max_keys*/ 1);

    if (!IcebergRESTCatalog::createWarehouseInKeeper(*context, catalog_config, query.warehouse_name, metadata) && !query.if_not_exists)
        throw Exception(ErrorCodes::WAREHOUSE_ALREADY_EXISTS, "Warehouse '{}' already exists", query.warehouse_name);

    return {};
}

void registerInterpreterCreateWarehouseQuery(InterpreterFactory & factory);
void registerInterpreterCreateWarehouseQuery(InterpreterFactory & factory)
{
    auto create_fn = [] (const InterpreterFactory::Arguments & args)
    {
        return std::make_unique<InterpreterCreateWarehouseQuery>(args.query, args.context);
    };
    factory.registerInterpreter("InterpreterCreateWarehouseQuery", create_fn);
}

}
