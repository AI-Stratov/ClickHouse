#include <Interpreters/InterpreterDropWarehouseQuery.h>

#include <Access/ContextAccess.h>
#include <Interpreters/Context.h>
#include <Interpreters/InterpreterFactory.h>
#include <Parsers/ASTDropWarehouseQuery.h>
#include <Server/IcebergRESTCatalog/IcebergRESTCatalogWarehouseKeeperUtils.h>

namespace DB
{

namespace ErrorCodes
{
    extern const int WAREHOUSE_DOESNT_EXIST;
    extern const int WAREHOUSE_NOT_EMPTY;
}

BlockIO InterpreterDropWarehouseQuery::execute()
{
    const auto & query = query_ptr->as<const ASTDropWarehouseQuery &>();
    auto context = getContext();

    context->checkAccess(AccessType::DROP_WAREHOUSE);

    const auto catalog_config = getIcebergRESTCatalogConfig(*context);
    const auto result = IcebergRESTCatalog::dropWarehouseFromKeeper(*context, catalog_config, query.warehouse_name);
    if (result == IcebergRESTCatalog::DropWarehouseResult::NotEmpty)
        throw Exception(ErrorCodes::WAREHOUSE_NOT_EMPTY, "Warehouse '{}' is not empty. Drop its namespaces first", query.warehouse_name);
    if (result == IcebergRESTCatalog::DropWarehouseResult::Missing && !query.if_exists)
        throw Exception(ErrorCodes::WAREHOUSE_DOESNT_EXIST, "Warehouse '{}' does not exist", query.warehouse_name);

    return {};
}

void registerInterpreterDropWarehouseQuery(InterpreterFactory & factory);
void registerInterpreterDropWarehouseQuery(InterpreterFactory & factory)
{
    auto create_fn = [] (const InterpreterFactory::Arguments & args)
    {
        return std::make_unique<InterpreterDropWarehouseQuery>(args.query, args.context);
    };
    factory.registerInterpreter("InterpreterDropWarehouseQuery", create_fn);
}

}
