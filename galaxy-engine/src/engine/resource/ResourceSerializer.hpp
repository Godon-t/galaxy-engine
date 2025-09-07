#pragma once

#include "Image.hpp"

namespace Galaxy {
class ResourceSerializer {
    static bool serialize(Image& image, std::string& outputPath);
}
} // namespace Galaxy
