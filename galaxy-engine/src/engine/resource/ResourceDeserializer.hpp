#pragma once

namespace Galaxy {
class Image;
class Mesh;

class ResourceDeserializer {
public:
    static bool deserialize(Image& image, const std::string& filePath);
    static bool deserialize(Mesh& image, const std::string& filePath);
};
} // namespace Galaxy
