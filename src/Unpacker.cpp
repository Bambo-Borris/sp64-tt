#include "Unpacker.hpp"

#include <SFML/Graphics.hpp>
#include <array>
#include <cassert>
#include <spdlog/fmt/fmt.h>
#include <fstream>
#include <iostream>
#include <spdlog/spdlog.h>
#include <unordered_map>

// BMP - ARGB 16-bit
// Where R = 5, G = 5, B = 5, A = 1

// For debugging purposes only!
#define UNUSED_VAR(var) (void)var;

// constexpr auto IGNORE_HEADER_OFFSET { 0x1680 };
constexpr auto SEPARATOR_LENGTH { 0x48 };
// clang-format off
constexpr auto SEQUENCE = std::array<char, 9> {
    0x00, 0x00, 0x00, 0x00, 0x08,0x00, 0x00, 0x00, 0x03 // We need to identify the sequence from the separator which is:
                                                        // 00 00 00 00 08 00 00 00 03
};
// clang-format on
constexpr auto OUTPUT_FILE_EXTENSION { "png" };

struct ImageFileInfo {
    size_t separatorStartIndex { 0 };
    sf::Vector2u bounds;
    size_t byteOffset;
};

// This is a custom has function we use so we can hash the
// sf::Vector2u structure (a 2D vector storing the x/y as
// unsigned integers)
namespace std {
template <>
struct hash<sf::Vector2u> {
    auto operator()(const sf::Vector2u& xyz) const -> size_t
    {
        const auto xHash = hash<size_t> {}(xyz.x);
        const auto yHash = hash<size_t> {}(xyz.y);
        return xHash ^ (yHash << 1);
    }
};
} // namespace std

// The bitmap colour format is 16-bit ARGB, whereas SFML will expect
// the colours to be in the format of 32-bit RGBA, so we need to
// convert the colours.
std::array<uint8_t, 4> convertARGB16ToRGBA32(uint16_t p)
{
    std::array<uint8_t, 4> pixel;
    pixel[3] = (p & 0x8000) ? 255 : 0; // A
    pixel[0] = uint8_t(((p & 0x7C00) >> 10) * 255 / 31); // R
    pixel[1] = uint8_t(((p & 0x3E0) >> 5) * 255 / 31); // G
    pixel[2] = uint8_t((p & 0x1F) * 255 / 31); // B
    return pixel;
}

std::vector<uint16_t> getImageData(const std::vector<char>& byteData, const ImageFileInfo& imageInfo)
{
    std::vector<uint16_t> imageData;
    const auto size { (16 * (imageInfo.bounds.x * imageInfo.bounds.y)) / 8 };
    const auto start { imageInfo.separatorStartIndex + SEPARATOR_LENGTH };
    for (size_t i { 0 }; i < (size - 1); i += 2) {
        uint16_t pixel;
        memcpy(&pixel, &byteData[start + i], 2 * sizeof(char));
        imageData.push_back(pixel);
    }
    return imageData;
}

void saveToDisk(std::string_view filename, const std::vector<uint16_t>& pixelData, const sf::Vector2u& size)
{
    // spdlog::debug("We have {} pixels ", pixelData.size());

    std::vector<uint8_t> convertedPixelData;
    for (auto p : pixelData) {
        const auto pixel = convertARGB16ToRGBA32(p);
        for (auto c : pixel)
            convertedPixelData.push_back(c);
    }

    sf::Image img;
    // For now we hardcode to 64x64
    img.resize(size, convertedPixelData.data());
    if (!img.saveToFile(filename)) {
        spdlog::error("Unable to save to file {} with size [{}, {}] to disk", filename, size.x, size.y);
        assert(false);
    }
}
namespace tt {
bool UnpackTextures(const std::filesystem::path& packedAssetPath)
{
    // Is this file empty cause if so I don't want to read it, there's
    // no point in continuing.
    const auto FileSize { std::filesystem::file_size(packedAssetPath) };
    if (FileSize == 0) {
        spdlog::error("Asset pack {} is empty", packedAssetPath.string());
        return false;
    }

    // Hurray let's create a input stream to use to read the file
    std::ifstream packedTexFile;
    packedTexFile.open(packedAssetPath, std::ios::in | std::ios::binary);

    // Ugh, we failed to load... See you later folks
    if (packedTexFile.fail()) {
        spdlog::error("Failed to open texture at path {}", packedAssetPath.string());
        return false;
    }
    // Now the juicy bit. We need a byte array to read the file contents into
    // so that we can perform operations on the contents. Here we make the
    // container, read the file into it, then close the file like a good
    // application.
    std::vector<char> rawByteData;
    rawByteData.resize(FileSize);
    packedTexFile.read(rawByteData.data(), static_cast<std::streamsize>(FileSize));
    packedTexFile.close();

    // TODO find all separator locations
    // Let's scan over the byte array and look for our magical separator pattern
    // if we find it, we'll add the separators ID to an array which we use
    // afterwards to extract the images.
    std::vector<size_t> allSeparatorLocations;

    for (size_t i { 0 }; i < rawByteData.size(); ++i) {
        std::array<char, SEQUENCE.size()> buff;
        bool doesMatch = true;
        if (i + SEQUENCE.size() >= rawByteData.size())
            break;

        for (size_t j { 0 }; j < SEQUENCE.size(); ++j) {
            buff[j] = rawByteData[i + j];
            if (SEQUENCE[j] != rawByteData[i + j]) {
                doesMatch = false;
                break;
            }
        }
        if (doesMatch) {
            allSeparatorLocations.push_back(i);
        }
    };

    spdlog::debug("Found {} separator locations", allSeparatorLocations.size());

    std::vector<uint16_t> imageData; // here we treat the pixel data as unsigned 16-bit integers

    // Okay we now know the locations of all separators in the packed file, let's identify the width
    // and height of each image and create a list of all images (organised by their dimension) that
    // we need to extract
    using ImageFileInfoList = std::vector<ImageFileInfo>;
    std::unordered_map<sf::Vector2u, ImageFileInfoList> entriesPerSize;

    for (size_t i { 0 }; i < allSeparatorLocations.size(); ++i) {
        if (i > allSeparatorLocations.size() - 2)
            break;

        uint16_t width = 0;
        uint16_t height = 0;
        const auto sepPos = allSeparatorLocations[i];

        // TODO swap to memcpy_s for safety...
        memcpy(&width, &rawByteData[sepPos + 0x2C], sizeof(uint16_t));
        memcpy(&height, &rawByteData[sepPos + 0x2C + sizeof(uint16_t)], sizeof(uint16_t));

        entriesPerSize[sf::Vector2u { width, height }].push_back(
            { allSeparatorLocations[i], { width, height }, sepPos });
    }

    for (auto& [k, v] : entriesPerSize) {
        int id = 0;
        for (const auto& pair : v) {
            UNUSED_VAR(pair);
            imageData.clear();
            imageData = getImageData(rawByteData, pair);
            // Name the file with the format width_height_id.format
            saveToDisk(fmt::format("{:x}_{}_{}.{}",
                                   pair.separatorStartIndex,
                                   fmt::format("{}x{}", k.x, k.y),
                                   id,
                                   OUTPUT_FILE_EXTENSION),
                       imageData,
                       k);
            ++id;
        }
    }
    return true;
}
}
