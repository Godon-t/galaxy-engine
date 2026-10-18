#pragma once

namespace Galaxy {
class Image;
class Mesh;
class BaseMaterial;
class Environment;

class ResourceSerializer {
public:
    static bool serialize(Image& image);
    static bool serialize(Environment& env);
    static bool serialize(BaseMaterial& material);
    static bool serialize(Mesh& mesh);
};
} // namespace Galaxy
