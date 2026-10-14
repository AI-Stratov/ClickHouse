#pragma once

#include <Parsers/IAST.h>

namespace DB
{

/// DROP WAREHOUSE [IF EXISTS] name
class ASTDropWarehouseQuery : public IAST
{
public:
    String warehouse_name;
    bool if_exists = false;

    String getID(char) const override { return "DropWarehouseQuery"; }

    ASTPtr clone() const override;

    QueryKind getQueryKind() const override { return QueryKind::Drop; }

protected:
    void formatImpl(WriteBuffer & ostr, const FormatSettings & settings, FormatState & state, FormatStateStacked frame) const override;
};

}
