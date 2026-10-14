#pragma once

#include <base/types.h>

#include <optional>

namespace DB::IcebergRESTCatalog
{

/// The server accepts only `s3://` locations.
bool hasS3Scheme(const String & location);

String stripTrailingSlashes(String location);

/// `s3://<bucket>/<key>` split into its parts. The key may be empty.
struct S3Location
{
    String bucket;
    String key;
};

/// Returns nullopt if `location` has no `s3://` scheme.
std::optional<S3Location> parseS3Location(const String & location);

}
