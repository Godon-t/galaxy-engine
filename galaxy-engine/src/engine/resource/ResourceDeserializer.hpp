#pragma once

namespace Galaxy {
class Image;

class ResourceDeserializer {
public:
    static bool deserialize(Image& image, const std::string& filePath);
};
} // namespace Galaxy
