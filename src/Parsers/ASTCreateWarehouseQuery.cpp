#include <Parsers/ASTCreateWarehouseQuery.h>

#include <Common/quoteString.h>
#include <IO/Operators.h>

namespace DB
{

ASTPtr ASTCreateWarehouseQuery::clone() const
{
    auto res = make_intrusive<ASTCreateWarehouseQuery>(*this);
    res->children.clear();
    if (settings)
        res->set(res->settings, settings->clone());
    return res;
}

void ASTCreateWarehouseQuery::formatImpl(WriteBuffer & ostr, const IAST::FormatSettings & format, IAST::FormatState & state, IAST::FormatStateStacked frame) const
{
    ostr << "CREATE WAREHOUSE ";
    if (if_not_exists)
        ostr << "IF NOT EXISTS ";
    ostr << backQuoteIfNeed(warehouse_name) << " SETTINGS ";
    settings->format(ostr, format, state, frame);
}

}
