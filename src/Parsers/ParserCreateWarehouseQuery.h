#pragma once

#include <Parsers/IParserBase.h>

namespace DB
{

/// CREATE WAREHOUSE [IF NOT EXISTS] name SETTINGS key = value, ...
/// TODO: consider CREATE ICEBERG WAREHOUSE syntax instead.
class ParserCreateWarehouseQuery : public IParserBase
{
protected:
    const char * getName() const override { return "CREATE WAREHOUSE query"; }
    bool parseImpl(Pos & pos, ASTPtr & node, Expected & expected) override;
};

}
