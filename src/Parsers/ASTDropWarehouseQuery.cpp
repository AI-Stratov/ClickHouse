#include <Parsers/ASTDropWarehouseQuery.h>

#include <Common/quoteString.h>
#include <IO/Operators.h>

namespace DB
{

ASTPtr ASTDropWarehouseQuery::clone() const
{
    return make_intrusive<ASTDropWarehouseQuery>(*this);
}

void ASTDropWarehouseQuery::formatImpl(WriteBuffer & ostr, const IAST::FormatSettings &, IAST::FormatState &, IAST::FormatStateStacked) const
{
    ostr << "DROP WAREHOUSE ";
    if (if_exists)
        ostr << "IF EXISTS ";
    ostr << backQuoteIfNeed(warehouse_name);
}

}
