#include "App.hpp"
#include "Unpacker.hpp"

#include <filesystem>
#include <spdlog/spdlog.h>
// For users on laptops we need to enable their dedicated GPUs
// otherwise we'll run our graphics on the integrated chip
#include <SFML/GpuPreference.hpp>

int main(int argc, char* argv[])
{
    (void)(argc);
    (void)(argv);

    App app;
    app.run();
}
