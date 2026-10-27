#pragma once

#include "ResourceManager.hpp"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

namespace Galaxy {
struct ResourceImporter {
    static std::string convertExternPathToGres(const std::string& externPath, const std::string& suffix = "")
    {
        std::string path, filename, extension;
        Project::extractExtension(externPath, path, filename, extension);
        return ".res/" + path + "/" + filename + suffix + ".gres";
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

    static uuid importMaterial(const std::string& externPath, int idx)
    {
        auto& rmInstance = ResourceManager::getInstance();
        auto makerIt     = rmInstance.m_makers.find(typeid(PBRMaterial).hash_code());
        GLX_CORE_ASSERT(makerIt != rmInstance.m_makers.end(), "No resource maker for file: '{0}'", externPath);

        std::string resPath = convertExternPathToGres(externPath, "_material_" + std::to_string(idx));

        if (Project::doesPathExist(resPath)) {
            return Project::getPathId(resPath);
        }

        uuid resourceID = Project::registerNewPath(ProjectPathTypes::RESOURCE, resPath);

        std::shared_ptr<PBRMaterial> resource = std::static_pointer_cast<PBRMaterial>(makerIt->second->createResourcePtr());
        resource->m_resourceID             = resourceID;
        resource->m_resourcePath           = resPath;

        Assimp::Importer importer;

        const aiScene* scene = importer.ReadFile((Project::getProjectRootPath() + externPath).c_str(),
            aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_SortByPType | aiProcess_FlipUVs);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            GLX_CORE_ERROR("Assimp error: {0}", importer.GetErrorString());
            return false;
        }

        std::string folderPath = Project::getFolderPath(externPath);
        aiMaterial* material   = scene->mMaterials[idx];
        auto tryImport         = [&resource, &material, &folderPath, &rmInstance](aiTextureType aiType, TextureType glxType) {
            std::string texturePath = getTexturePath(material, aiType);
            if (texturePath[0] != '*' && texturePath != "") {
                uuid imageID                  = importImage(folderPath + "/" + texturePath);
                resource->setImage(glxType, rmInstance.load<Image>(Project::getPath(ProjectPathTypes::RESOURCE, imageID)));
                return true;
            } else {
                return false;
            }
        };

        if (!tryImport(aiTextureType_BASE_COLOR, ALBEDO)) {
            tryImport(aiTextureType_AMBIENT, ALBEDO);
        } else {
            tryImport(aiTextureType_AMBIENT, AO);
        }

        if (resource->canUseImage(ALBEDO))
            resource->getImage(ALBEDO).getResource().onLoaded([resource] {
                resource->setUseTransparency(resource->getImage(ALBEDO).getResource().hasTransparency());
                resource->save();
            });

        aiColor3D color(0.f, 0.f, 0.f);
        if (material->Get(AI_MATKEY_COLOR_DIFFUSE, color) == AI_SUCCESS) {
            resource->setAlbedo(vec3(color.r, color.g, color.b));
        }

        float value = 0.f;
        tryImport(aiTextureType_SPECULAR, METALLIC);
        if (material->Get(AI_MATKEY_METALLIC_FACTOR, value) == AI_SUCCESS) {
            resource->setMetallic(value);
        }

        tryImport(aiTextureType_NORMALS, NORMAL);

        tryImport(aiTextureType_SHININESS, ROUGHNESS);
        if (material->Get(AI_MATKEY_ROUGHNESS_FACTOR, value) == AI_SUCCESS) {
            resource->setRoughness(value);
        }

        // TODO: Height map not imported

        float opacity;
        material->Get(AI_MATKEY_OPACITY, opacity);
        resource->setTransparency(opacity);
        material->Get(AI_MATKEY_TRANSPARENCYFACTOR, opacity);
        if (opacity < resource->getTransparency())
            resource->setTransparency(opacity);

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
                resource->m_subMeshes[i].material    = rmInstance.load<BaseMaterial>(Project::getPath(ProjectPathTypes::RESOURCE, materialID), true);
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
        std::string path, filename, extension;
        Project::extractExtension(file, path, filename, extension);

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
