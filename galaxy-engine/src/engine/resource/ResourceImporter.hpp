#pragma once

#include "ResourceManager.hpp"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

namespace Galaxy {
struct ResourceImporter {
    static std::string convertExternPathToGres(const std::string& externPath)
    {
        std::string path, extension;
        Project::extractExtension(externPath, path, extension);
        path += std::string(".gres");
        return path;
    }
    static uuid importImage(const std::string& externPath)
    {
        auto makerIt = ResourceManager::getInstance().m_makers.find(typeid(Image).hash_code());
        GLX_CORE_ASSERT(makerIt != ResourceManager::getInstance().m_makers.end(), "No resource maker for file: '{0}'", externPath);

        std::shared_ptr<Image> resource = std::static_pointer_cast<Image>(makerIt->second->createResourcePtr());

        auto resPath = convertExternPathToGres(externPath);
        if (Project::doesPathExist(resPath)) {
            return Project::getPathId(resPath);
        }
        uuid resourceID = Project::registerNewPath(ProjectPathTypes::RESOURCE, resPath);

        resource->m_resourceID   = resourceID;
        resource->m_resourcePath = resPath;
        resource->loadExtern(externPath);

        if (resource->save(false)) {
            return resourceID;
        } else {
            return 0;
        }
    }

    static std::string getTexturePath(aiMaterial* material, aiTextureType textureType)
    {
        std::string res = "";
        if (material->GetTextureCount(textureType) > 0) {
            aiString str;
            material->GetTexture(textureType, 0, &str);
            res = str.C_Str();
        }
        return res;
    }

    static uuid importMaterial(const std::string& externPath, int idx)
    {
        auto& rmInstance = ResourceManager::getInstance();
        auto makerIt     = rmInstance.m_makers.find(typeid(Material).hash_code());
        GLX_CORE_ASSERT(makerIt != rmInstance.m_makers.end(), "No resource maker for file: '{0}'", externPath);

        std::string gltfPath, extension;
        Project::extractExtension(externPath, gltfPath, extension);
        std::string resPath = gltfPath + "_material_" + std::to_string(idx) + ".gres";

        if (Project::doesPathExist(resPath)) {
            return Project::getPathId(resPath);
        }

        uuid resourceID = Project::registerNewPath(ProjectPathTypes::RESOURCE, resPath);

        std::shared_ptr<Material> resource = std::static_pointer_cast<Material>(makerIt->second->createResourcePtr());
        resource->m_resourceID             = resourceID;
        resource->m_resourcePath           = resPath;

        Assimp::Importer importer;

        const aiScene* scene = importer.ReadFile((Project::getProjectRootPath() + externPath).c_str(),
            aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_SortByPType | aiProcess_FlipUVs);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            GLX_CORE_ERROR("Assimp error: {0}", importer.GetErrorString());
            return false;
        }

        aiMaterial* material = scene->mMaterials[idx];
        auto tryImport       = [&resource, &material, &gltfPath, &rmInstance](aiTextureType aiType, TextureType glxType) {
            std::string texturePath = getTexturePath(material, aiType);
            if (texturePath[0] != '*' && texturePath != "") {
                uuid imageID                  = importImage(gltfPath + "/" + texturePath);
                resource->m_images[glxType]   = rmInstance.load<Image>(Project::getPath(ProjectPathTypes::RESOURCE, imageID));
                resource->m_useImage[glxType] = true;
            }
        };

        tryImport(aiTextureType_BASE_COLOR, ALBEDO);
        aiColor3D color(0.f, 0.f, 0.f);
        if (material->Get(AI_MATKEY_COLOR_DIFFUSE, color) == AI_SUCCESS) {
            resource->m_albedo = vec3(color.r, color.g, color.b);
        }

        float value = 0.f;
        tryImport(aiTextureType_SPECULAR, METALLIC);
        if (material->Get(AI_MATKEY_METALLIC_FACTOR, value) == AI_SUCCESS) {
            resource->m_metallic = value;
        }

        tryImport(aiTextureType_NORMALS, NORMAL);

        tryImport(aiTextureType_SHININESS, ROUGHNESS);
        if (material->Get(AI_MATKEY_ROUGHNESS_FACTOR, value) == AI_SUCCESS) {
            resource->m_roughness = value;
        }

        tryImport(aiTextureType_AMBIENT, AO);

        if (resource->save(false)) {
            return resourceID;
        } else {
            return 0;
        }
    }

    static uuid importGltf(const std::string& externPath)
    {
        auto& rmInstance = ResourceManager::getInstance();
        auto makerIt     = rmInstance.m_makers.find(typeid(Mesh).hash_code());
        GLX_CORE_ASSERT(makerIt != rmInstance.m_makers.end(), "No resource maker for file: '{0}'", externPath);

        std::shared_ptr<Mesh> resource = std::static_pointer_cast<Mesh>(makerIt->second->createResourcePtr());

        auto resPath = convertExternPathToGres(externPath);

        if (Project::doesPathExist(resPath)) {
            return Project::getPathId(resPath);
        }

        uuid resourceID = Project::registerNewPath(ProjectPathTypes::RESOURCE, resPath);

        resource->m_resourceID   = resourceID;
        resource->m_resourcePath = resPath;

        // loadExtern(externalPath);

        resource->m_gltfPath   = externPath;
        resource->m_isInternal = false;

        const int meshIdx = 0;

        Assimp::Importer importer;

        const aiScene* scene = importer.ReadFile((Project::getProjectRootPath() + externPath).c_str(),
            aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_SortByPType | aiProcess_FlipUVs);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            GLX_CORE_ERROR("Assimp error: {0}", importer.GetErrorString());
            return false;
        }

        std::vector<int> meshMaterialIdx;
        meshMaterialIdx.reserve(scene->mNumMeshes);
        resource->m_subMeshes.resize(scene->mNumMeshes);
        for (int i = 0; i < scene->mNumMeshes; i++) {
            resource->extractSubMesh(scene, i);
            uuid materialID = importMaterial(externPath, scene->mMeshes[i]->mMaterialIndex);
            if (materialID != 0) {
                resource->m_subMeshes[i].hasMaterial = true;
                resource->m_subMeshes[i].material    = rmInstance.load<Material>(Project::getPath(ProjectPathTypes::RESOURCE, materialID));
            }
        }

        if (resource->save(false)) {
            return resourceID;
        } else {
            return 0;
        }
    }

    static bool import(const std::string& file)
    {
        std::string path, extension;
        Project::extractExtension(file, path, extension);

        if (extension == ".jpg" || extension == ".png") {
            importImage(file);
        }
        if (extension == ".gltf") {
            importGltf(file);
        }
        Project::savePaths();
        return true;
    }
};
}