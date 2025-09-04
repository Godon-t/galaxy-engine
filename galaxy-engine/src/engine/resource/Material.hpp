#pragma once

#include "Resource.hpp"

namespace Galaxy {
void extractMaterial(Material& mat, aiMesh* mesh, const aiScene* scene)
{
    if (mesh->mMaterialIndex >= 0) {
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

        auto albedoMaps = loadMaterialTextures(material, aiTextureType_BASE_COLOR, scene);
        if (!albedoMaps.empty()) {
            mat.albedoTex          = albedoMaps[0];
            mat.albedoTex->visible = true;
        } else
            mat.albedoTex = Texture::emptyTexture;

        auto specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, scene);
        if (!specularMaps.empty()) {
            mat.metallicTex          = specularMaps[0];
            mat.metallicTex->visible = true;
        } else
            mat.metallicTex = Texture::emptyTexture;

        auto normalMaps = loadMaterialTextures(material, aiTextureType_NORMALS, scene);
        if (normalMaps.empty())
            normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, scene);
        if (!normalMaps.empty()) {
            mat.normalTex          = normalMaps[0];
            mat.normalTex->visible = true;
        } else
            mat.normalTex = Texture::emptyTexture;

        auto roughnessMaps = loadMaterialTextures(material, aiTextureType_SHININESS, scene);
        if (!roughnessMaps.empty()) {
            mat.roughnessTex          = roughnessMaps[0];
            mat.roughnessTex->visible = true;
        } else
            mat.roughnessTex = Texture::emptyTexture;

        auto aoMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, scene);
        if (!aoMaps.empty()) {
            mat.aoTex          = aoMaps[0];
            mat.aoTex->visible = true;
        } else
            mat.aoTex = Texture::emptyTexture;
    }
}

struct subMat {
};

class Material : ResourceBase {
public:
    bool load(const std::string& file) override;

private:
    bool readGltf(const std::string& filePath);
    std::string m_gltfPath;

    std::vector<SubMesh> m_subMeshes;
};
} // namespace Galaxy
