#include <SFML/Graphics.hpp>
#include <array>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <spdlog/spdlog.h>

// BMP - ARGB 16-bit
// Where R = 5, G = 5, B = 5, A = 1

int main(int argc, char* argv[])
{
    _CRT_UNUSED(argc);
    _CRT_UNUSED(argv);

    spdlog::set_level(spdlog::level::debug);

    if (!std::filesystem::exists("kennypixeldata")) {
        spdlog::error("File not found kennypixeldata");
        return EXIT_FAILURE;
    }

    std::ifstream packedTexFile;
    packedTexFile.open("kennypixeldata", std::ios::in | std::ios::binary);

    if (packedTexFile.fail()) {
        spdlog::error("Failed to open texture at path kennypixeldata");
        return EXIT_FAILURE;
    }

    const auto FileSize { std::filesystem::file_size("kennypixeldata") };
    std::cout << "File Size : " << FileSize << "\n";

    std::vector<std::uint16_t> imageData;

    while (!packedTexFile.eof()) {
        std::array<char, 2> buff;
        packedTexFile.read(buff.data(), 2);
        std::uint16_t pixel;
        memcpy(&pixel, buff.data(), buff.size() * sizeof(char));
        imageData.push_back(pixel);
    }
    spdlog::debug("We have {} pixels ", imageData.size());

    // std::vector<std::uint32_t> convertedImageData;
    // convertedImageData.resize(imageData.size());
    std::vector<std::uint8_t> convertedImageData;
    for (auto p : imageData) {
        std::array<std::uint8_t, 4> pixel;
        pixel[3] = (p & 0x3000) ? 255 : 0; // A
        pixel[0] = ((p & 0x7C00) >> 10) * 255 / 31; // R
        pixel[1] = std::uint8_t(((p & 0x3E0) >> 5) * 255 / 31); // G
        pixel[2] = ((p & 0x1F) >> 10) * 255 / 31; // B

        for (auto c : pixel)
            convertedImageData.push_back(c);
    }

    sf::Image img;
    img.create({ 64, 64 }, convertedImageData.data());

    if (!img.saveToFile("Test.bmp")) {
        spdlog::error("Unable to save to file!");
    }
    // sf::Image g;
    // auto window = sf::RenderWindow{{sf::Vector2u{640, 480}}};
    // window.setFramerateLimit(144);

    // while (window.isOpen())
    // {
    //     for (auto event = sf::Event{}; window.pollEvent(event);)
    //     {
    //         if (event.type == sf::Event::Closed)
    //         {
    //             window.close();
    //         }
    //     }

    //     window.clear();
    //     window.display();
    // }
}