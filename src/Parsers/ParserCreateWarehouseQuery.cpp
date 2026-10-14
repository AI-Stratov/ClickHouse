#include <Parsers/ParserCreateWarehouseQuery.h>

#include <Parsers/ASTCreateWarehouseQuery.h>
#include <Parsers/ASTIdentifier.h>
#include <Parsers/CommonParsers.h>
#include <Parsers/ExpressionElementParsers.h>
#include <Parsers/ParserSetQuery.h>

namespace DB
{

bool ParserCreateWarehouseQuery::parseImpl(IParser::Pos & pos, ASTPtr & node, Expected & expected)
{
    ParserKeyword s_create(Keyword::CREATE);
    ParserKeyword s_warehouse(Keyword::WAREHOUSE);
    ParserKeyword s_if_not_exists(Keyword::IF_NOT_EXISTS);
    ParserKeyword s_settings(Keyword::SETTINGS);
    ParserIdentifier name_p;
    ParserSetQuery settings_p(/* parse_only_internals_ = */ true);

    if (!s_create.ignore(pos, expected))
        return false;
    if (!s_warehouse.ignore(pos, expected))
        return false;

    bool if_not_exists = s_if_not_exists.ignore(pos, expected);

    ASTPtr name;
    if (!name_p.parse(pos, name, expected))
        return false;

    if (!s_settings.ignore(pos, expected))
        return false;

    ASTPtr settings;
    if (!settings_p.parse(pos, settings, expected))
        return false;

    auto query = make_intrusive<ASTCreateWarehouseQuery>();
    tryGetIdentifierNameInto(name, query->warehouse_name);
    query->if_not_exists = if_not_exists;
    query->set(query->settings, settings);

    node = query;
    return true;
}

}
