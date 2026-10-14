#include <Server/IcebergRESTCatalog/IcebergRESTCatalogWarehouseMetadata.h>

#include <Common/Exception.h>
#include <Server/IcebergRESTCatalog/IcebergRESTCatalogJSON.h>
#include <Server/IcebergRESTCatalog/IcebergRESTCatalogLocation.h>

#include <cctype>

namespace DB
{

namespace ErrorCodes
{
    extern const int BAD_ARGUMENTS;
    extern const int INCORRECT_DATA;
}

namespace
{

constexpr auto SETTING_BASE_LOCATION = "base_location";
constexpr auto SETTING_STORAGE_NAMED_COLLECTION = "storage_named_collection";

/// Bump on any layout change so old data is refused, not misread.
constexpr auto FORMAT = "IcebergRESTCatalog\nformat_version: 2";

}

IcebergRESTCatalogWarehouseMetadata IcebergRESTCatalogWarehouseMetadata::fromSettings(const SettingsChanges & changes)
{
    IcebergRESTCatalogWarehouseMetadata metadata;
    for (const auto & change : changes)
    {
        if (change.value.getType() != Field::Types::String)
            throw Exception(ErrorCodes::BAD_ARGUMENTS, "Warehouse setting '{}' must be a string", change.name);
        const auto & value = change.value.safeGet<String>();

        if (change.name == SETTING_BASE_LOCATION)
            metadata.base_location = IcebergRESTCatalog::stripTrailingSlashes(value);
        else if (change.name == SETTING_STORAGE_NAMED_COLLECTION)
            metadata.storage_named_collection = value;
        else
            throw Exception(
                ErrorCodes::BAD_ARGUMENTS,
                "Unknown warehouse setting '{}'. Supported settings: {}, {}",
                change.name,
                SETTING_BASE_LOCATION,
                SETTING_STORAGE_NAMED_COLLECTION);
    }

    if (metadata.base_location.empty())
        throw Exception(ErrorCodes::BAD_ARGUMENTS, "Warehouse setting '{}' is required", SETTING_BASE_LOCATION);
    if (!IcebergRESTCatalog::hasS3Scheme(metadata.base_location))
        throw Exception(ErrorCodes::BAD_ARGUMENTS, "Warehouse setting '{}' must start with s3://", SETTING_BASE_LOCATION);
    if (metadata.storage_named_collection.empty())
        throw Exception(ErrorCodes::BAD_ARGUMENTS, "Warehouse setting '{}' is required", SETTING_STORAGE_NAMED_COLLECTION);
    return metadata;
}

String IcebergRESTCatalogWarehouseMetadata::toKeeperData() const
{
    Poco::JSON::Object json;
    json.set("format", FORMAT);
    json.set("settings", stringMapToJSON(toSettingsMap()));
    return toJSONString(json);
}

IcebergRESTCatalogWarehouseMetadata IcebergRESTCatalogWarehouseMetadata::fromKeeperData(const String & data, const String & path)
{
    const auto what = fmt::format("Warehouse metadata at {}", path);
    const auto json = parseJSONObject(data, what);

    const auto format = json->optValue<String>("format", "");
    if (format != FORMAT)
        throw Exception(ErrorCodes::INCORRECT_DATA, "{} has an unsupported format: expected {}, found {}", what, FORMAT, format);

    const auto settings = json->getObject("settings");
    if (!settings)
        throw Exception(ErrorCodes::INCORRECT_DATA, "{} has no 'settings' object", what);

    SettingsChanges changes;
    for (const auto & [key, value] : stringMapFromJSON(*settings, what))
        changes.emplace_back(key, value);
    return fromSettings(changes);
}

std::map<String, String> IcebergRESTCatalogWarehouseMetadata::toSettingsMap() const
{
    return {{SETTING_BASE_LOCATION, base_location}, {SETTING_STORAGE_NAMED_COLLECTION, storage_named_collection}};
}

/// A warehouse name is a path segment of the REST catalog URLs.
/// Letters, digits, '_' and '-' need no percent-encoding in URLs.
/// `escapeForFileName` also keeps them unchanged, so the Keeper node name equals the warehouse name.
void validateIcebergRESTCatalogWarehouseName(const String & name)
{
    constexpr size_t max_length = 64;
    if (name.empty() || name.size() > max_length)
        throw Exception(ErrorCodes::BAD_ARGUMENTS, "Warehouse name must be 1 to {} characters long", max_length);

    for (const char c : name)
    {
        if (!std::isalnum(static_cast<unsigned char>(c)) && c != '_' && c != '-')
            throw Exception(
                ErrorCodes::BAD_ARGUMENTS, "Warehouse name '{}' is invalid. Allowed characters: letters, digits, '_' and '-'", name);
    }
}

}
