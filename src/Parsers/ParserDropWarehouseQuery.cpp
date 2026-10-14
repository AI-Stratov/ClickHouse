#include <Parsers/ParserDropWarehouseQuery.h>

#include <Parsers/ASTDropWarehouseQuery.h>
#include <Parsers/ASTIdentifier.h>
#include <Parsers/CommonParsers.h>
#include <Parsers/ExpressionElementParsers.h>

namespace DB
{

bool ParserDropWarehouseQuery::parseImpl(IParser::Pos & pos, ASTPtr & node, Expected & expected)
{
    ParserKeyword s_drop(Keyword::DROP);
    ParserKeyword s_warehouse(Keyword::WAREHOUSE);
    ParserKeyword s_if_exists(Keyword::IF_EXISTS);
    ParserIdentifier name_p;

    if (!s_drop.ignore(pos, expected))
        return false;
    if (!s_warehouse.ignore(pos, expected))
        return false;

    bool if_exists = s_if_exists.ignore(pos, expected);

    ASTPtr name;
    if (!name_p.parse(pos, name, expected))
        return false;

    auto query = make_intrusive<ASTDropWarehouseQuery>();
    tryGetIdentifierNameInto(name, query->warehouse_name);
    query->if_exists = if_exists;

    node = query;
    return true;
}

}
