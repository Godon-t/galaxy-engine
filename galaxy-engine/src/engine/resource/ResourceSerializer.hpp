#pragma once

namespace Galaxy {
class Image;
class Mesh;

class ResourceSerializer {
public:
    static bool serialize(Image& image, std::string& outputPath);
    static bool serialize(Mesh& mesh, std::string& outputPath);
};
} // namespace Galaxy
