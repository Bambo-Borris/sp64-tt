#include "Unpacker.hpp"

#include <filesystem>
#include <spdlog/spdlog.h>

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

    if (!tt::UnpackTextures(packedAssetPath)) {
        spdlog::error("Unable to unpack textures, exiting...");
        return EXIT_FAILURE;
    }
}
