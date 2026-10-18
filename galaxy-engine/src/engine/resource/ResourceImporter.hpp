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

            for (auto& c : res) {
                if (c == '\\')
                    c = '/';
            }
        }
        return res;
    }

    static uuid importMaterial(const std::string&, int)
    {
        // Material import depends on the temporarily disabled serialization.
        return 0;
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
        if (extension == ".gltf" || extension == ".obj") {
            importGltf(file);
        }
        Project::savePaths();
        return true;
    }
};
}