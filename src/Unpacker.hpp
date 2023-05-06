#pragma once

#include <filesystem>

namespace tt {
[[nodiscard]] bool UnpackTextures(const std::filesystem::path& packedAssetPath);
}
