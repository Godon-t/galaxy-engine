#pragma once

namespace Galaxy {
class Image;
class Mesh;

class ResourceSerializer {
public:
    static bool serialize(Image& image);
    static bool serialize(Mesh& mesh);
};
} // namespace Galaxy
