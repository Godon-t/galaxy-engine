#pragma once

#include "Image.hpp"

namespace Galaxy {
class ResourceDeSerializer {
    static bool deserialize(Image& image, std::string& filePath);
}
} // namespace Galaxy
