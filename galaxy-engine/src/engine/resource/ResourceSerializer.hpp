#pragma once

namespace Galaxy {
class Image;
class Mesh;
class Material;
class Environment;

class ResourceSerializer {
public:
    static bool serialize(Image& image);
    static bool serialize(Environment& env);
    static bool serialize(Material& material);
    static bool serialize(Mesh& mesh);
};
} // namespace Galaxy
