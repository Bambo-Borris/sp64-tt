#include <SFML/Graphics.hpp>
#include <array>
#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <spdlog/fmt/fmt.h>
#include <spdlog/spdlog.h>

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

struct SeparatorPair {
    size_t current { 0 };
    size_t next { 0 };
};

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

std::vector<uint16_t> getImageData(const std::vector<char>& byteData, const SeparatorPair& separator)
{
    assert(separator.current + SEPARATOR_LENGTH < byteData.size());
    assert(separator.next < byteData.size());

    std::vector<uint16_t> imageData;

    for (size_t i { separator.current + SEPARATOR_LENGTH }; i < separator.next - 1; i += 2) {
        uint16_t pixel;
        memcpy(&pixel, &byteData[i], 2 * sizeof(char));
        imageData.push_back(pixel);
    }
    return imageData;
}

void saveToDisk(std::string_view filename, const std::vector<uint16_t>& pixelData)
{
    spdlog::debug("We have {} pixels ", pixelData.size());

    std::vector<uint8_t> convertedPixelData;
    for (auto p : pixelData) {
        const auto pixel = convertARGB16ToRGBA32(p);
        for (auto c : pixel)
            convertedPixelData.push_back(c);
    }

    sf::Image img;
    // For now we hardcode to 64x64
    img.create({ 64, 64 }, convertedPixelData.data());
    if (!img.saveToFile(filename)) {
        spdlog::error("Unable to save to file!");
        assert(false);
    }
}

int main(int argc, char* argv[])
{
    // Before we do anything we'll configure out log output to
    // debug (release mode we'll probably alter this to reduce)
    // output verbosity
    spdlog::set_level(spdlog::level::debug);

    // Erm did the user pass a file path as an arg? No? Well then
    // we'll give up and exit
    if (argc < 2) {
        spdlog::error("No path to asset pack provided");
        return EXIT_FAILURE;
    }

    // Okay they did pass us a file path, but... does it exist?!
    // No? Okay well I also say we give up!
    const std::string packedAssetPath = argv[1];
    if (!std::filesystem::exists(packedAssetPath)) {
        spdlog::error("File not found {}", packedAssetPath);
        return EXIT_FAILURE;
    }

    // Is this file empty cause if so I don't want to read it, there's
    // no point in continuing.
    const auto FileSize { std::filesystem::file_size(packedAssetPath) };
    if (FileSize == 0) {
        spdlog::error("Asset pack {} is empty", packedAssetPath);
        return EXIT_FAILURE;
    }

    // Hurray let's create a input stream to use to read the file
    std::ifstream packedTexFile;
    packedTexFile.open(packedAssetPath, std::ios::in | std::ios::binary);

    // Ugh, we failed to load... See you later folks
    if (packedTexFile.fail()) {
        spdlog::error("Failed to open texture at path {}", packedAssetPath);
        return EXIT_FAILURE;
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
    std::vector<SeparatorPair> separatorUniform64Images; // Separator pairs that denote a 64x64 image data block
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

    // Okay we now know the offsets in the raw file data to each separator, great right? Well not quite...
    // For now we just want 64x64 images, and we also need to pair separators since
    // the offset_start + separator_size = image data start, offset_end = image data end.

    // TODO handle verifying size of final separator since it has no partner
    for (size_t i { 0 }; i < allSeparatorLocations.size(); ++i) {
        if (i > allSeparatorLocations.size() - 2)
            break;

        const auto distance { (allSeparatorLocations[i + 1] - allSeparatorLocations[i]) - SEPARATOR_LENGTH };
        // for now we just wanna see the 64x64 images
        if (distance == 8192) {
            spdlog::debug("Distance {} for separator {:x} & {:x} therefore 64 x 64 image found",
                          distance,
                          allSeparatorLocations[i],
                          allSeparatorLocations[i + 1]);
            separatorUniform64Images.push_back({ allSeparatorLocations[i], allSeparatorLocations[i + 1] });
        }
    }

    int id = 0;
    for (auto& pair : separatorUniform64Images) {
        UNUSED_VAR(pair);
        imageData.clear();
        imageData = getImageData(rawByteData, pair);
        saveToDisk(fmt::format("Test_{}.{}", id, OUTPUT_FILE_EXTENSION), imageData);
        ++id;
    }
}