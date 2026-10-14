#pragma once

#include <Parsers/IAST.h>

namespace DB
{

/// CREATE WAREHOUSE [IF NOT EXISTS] name SETTINGS key = value, ...
class ASTCreateWarehouseQuery : public IAST
{
public:
    String warehouse_name;
    bool if_not_exists = false;
    /// `ASTSetQuery` with the `SETTINGS` pairs.
    ASTPtr settings;

    String getID(char) const override { return "CreateWarehouseQuery"; }

    ASTPtr clone() const override;

    QueryKind getQueryKind() const override { return QueryKind::Create; }

protected:
    void formatImpl(WriteBuffer & ostr, const FormatSettings & settings, FormatState & state, FormatStateStacked frame) const override;
};

}
