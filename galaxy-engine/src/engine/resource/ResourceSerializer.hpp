#pragma once

namespace Galaxy {
class Image;

class ResourceSerializer {
public:
    static bool serialize(Image& image, std::string& outputPath);
};
} // namespace Galaxy
