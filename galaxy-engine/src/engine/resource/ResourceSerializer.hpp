#pragma once

namespace Galaxy {
class Image;
class Mesh;
class BaseMaterial;
class PBRMaterial;
class TextureMaterial;
class CubemapResource;

class ResourceSerializer {
public:
    static bool serialize(Image& image);
    static bool serialize(CubemapResource& cubemap);
    static bool serialize(BaseMaterial& material);
    static bool serialize(PBRMaterial& material);
    static bool serialize(TextureMaterial& material);
    static bool serialize(Mesh& mesh);
};
} // namespace Galaxy
