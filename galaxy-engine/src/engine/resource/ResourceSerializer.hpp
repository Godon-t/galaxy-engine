#pragma once

namespace Galaxy {
class Image;
class Mesh;
class BaseMaterial;
class PBRMaterial;
class TextureMaterial;
class Environment;

class ResourceSerializer {
public:
    static bool serialize(Image& image);
    static bool serialize(Environment& env);
    static bool serialize(BaseMaterial& material);
    static bool serialize(PBRMaterial& material);
    static bool serialize(TextureMaterial& material);
    static bool serialize(Mesh& mesh);
};
} // namespace Galaxy
