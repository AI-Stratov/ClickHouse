#include <Server/IcebergRESTCatalog/IcebergRESTCatalogLocation.h>

namespace DB::IcebergRESTCatalog
{

namespace
{

constexpr std::string_view S3_SCHEME = "s3://";

}

bool hasS3Scheme(const String & location)
{
    return location.starts_with(S3_SCHEME);
}

String stripTrailingSlashes(String location)
{
    while (location.ends_with('/'))
        location.pop_back();
    return location;
}

std::optional<S3Location> parseS3Location(const String & location)
{
    /// TODO: support s3a:// and s3n://
    if (!location.starts_with(S3_SCHEME))
        return std::nullopt;
    const auto slash = location.find('/', S3_SCHEME.size());
    if (slash == String::npos)
        return S3Location{.bucket = location.substr(S3_SCHEME.size()), .key = ""};
    return S3Location{.bucket = location.substr(S3_SCHEME.size(), slash - S3_SCHEME.size()), .key = location.substr(slash + 1)};
}

}
