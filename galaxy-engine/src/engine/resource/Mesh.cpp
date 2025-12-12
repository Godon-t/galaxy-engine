#include "Mesh.hpp"

#include "ResourceManager.hpp"
#include "core/Log.hpp"
#include "project/Project.hpp"
#include "types/Math.hpp"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

#include <filesystem>

namespace Galaxy {

bool Mesh::loadExtern(const std::string& filePath)
{
    m_gltfPath   = filePath;
    m_isInternal = false;

    const int meshIdx = 0;

    Assimp::Importer importer;

    const aiScene* scene = importer.ReadFile((Project::getProjectRootPath() + filePath).c_str(),
        aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_SortByPType | aiProcess_FlipUVs);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        GLX_CORE_ERROR("Assimp error: {0}", importer.GetErrorString());
        return false;
    }

    m_subMeshes.resize(scene->mNumMeshes);
    aiNode* sceneNode = scene->mRootNode;
    m_subMeshTree     = buildTree(sceneNode);

    for (int i = 0; i < scene->mNumMeshes; i++) {
        extractSubMesh(scene, i);
    }
    return true;
}

bool Mesh::save(bool recursive)
{
    return ResourceSerializer::serialize(*this);
}

bool Mesh::load(YAML::Node& data)
{
    if (!data["Type"] || data["Type"].as<std::string>() != std::string("Mesh")) {
        GLX_CORE_ERROR("File unsupported");
        return false;
    }

    if (data["ExternalFile"]) {
        if (!loadExtern(data["ExternalFile"].as<std::string>()))
            return false;
    }

    if (data["SubMeshes"]) {
        for (int i = 0; i < data["SubMeshes"].size(); i++) {
            auto subMesh               = data["SubMeshes"][i];
            m_subMeshes[i].hasMaterial = subMesh["HasMaterial"].as<bool>();
            if (hasMaterial(i)) {
                uuid materialID         = subMesh["MaterialID"].as<uint64_t>();
                m_subMeshes[i].material = ResourceManager::getInstance().load<Material>(Project::getPath(ProjectPathTypes::RESOURCE, materialID));
            }
        }
    }
    return true;
}

SubMeshTree& Mesh::getMeshTree(int surfaceIdx)
{
    return *findCorrespondingTree(&m_subMeshTree, surfaceIdx);
}

SubMeshTree& Mesh::getRootTree()
{
    return m_subMeshTree;
}

void Mesh::extractSubMesh(const aiScene* scene, int surface)
{
    std::vector<math::vec3> indexed_vertices;
    std::vector<math::vec3> normals;
    std::vector<math::vec2> tex_coords;

    aiMesh* mesh            = scene->mMeshes[surface];
    SubMesh& currentSubMesh = m_subMeshes[surface];

    // Add indices
    for (unsigned int faceIdx = 0; faceIdx < mesh->mNumFaces; ++faceIdx) {
        aiFace& face = mesh->mFaces[faceIdx];
        for (unsigned int k = 0; k < face.mNumIndices; ++k) {
            currentSubMesh.indices.push_back(face.mIndices[k]);
        }
    }

    // Add vertex coords
    for (unsigned int vertexIdx = 0; vertexIdx < mesh->mNumVertices; ++vertexIdx) {
        aiVector3D vertex = mesh->mVertices[vertexIdx];
        indexed_vertices.push_back(vec3(vertex.x, vertex.y, vertex.z));
    }

    // Add texture coords
    bool hasTexCoords = mesh->HasTextureCoords(0);
    if (hasTexCoords) {
        for (unsigned int j = 0; j < mesh->mNumVertices; ++j) {
            aiVector3D tex_coord = mesh->mTextureCoords[0][j];
            tex_coords.push_back(vec2(tex_coord.x, tex_coord.y));
        }
    }

    // Add normals
    if (mesh->HasNormals()) {
        for (unsigned int j = 0; j < mesh->mNumVertices; ++j) {
            aiVector3D normal = mesh->mNormals[j];
            normals.push_back(vec3(normal.x, normal.y, normal.z));
        }
    } else {
        normals.resize(mesh->mNumVertices, vec3(0.0f, 0.0f, 0.0f));

        for (unsigned int faceIdx = 0; faceIdx < mesh->mNumFaces; ++faceIdx) {
            aiFace& face = mesh->mFaces[faceIdx];

            if (face.mNumIndices == 3) {
                unsigned int idx0 = face.mIndices[0];
                unsigned int idx1 = face.mIndices[1];
                unsigned int idx2 = face.mIndices[2];

                vec3 v0 = indexed_vertices[idx0];
                vec3 v1 = indexed_vertices[idx1];
                vec3 v2 = indexed_vertices[idx2];

                vec3 edge1       = v1 - v0;
                vec3 edge2       = v2 - v0;
                vec3 face_normal = cross(edge1, edge2);

                normals[idx0] += face_normal;
                normals[idx1] += face_normal;
                normals[idx2] += face_normal;
            }
        }

        for (unsigned int j = 0; j < normals.size(); ++j) {
            normals[j] = normalize(normals[j]);
        }
    }

    currentSubMesh.vertices.resize(indexed_vertices.size());

    for (int i = 0; i < indexed_vertices.size(); ++i) {
        Vertex v;
        v.position = indexed_vertices[i];
        v.texCoord = hasTexCoords ? tex_coords[i] : vec2(0);
        v.normal   = normals[i];

        currentSubMesh.vertices[i] = v;
    }
}

SubMeshTree Mesh::buildTree(const aiNode* node)
{
    aiMatrix4x4 nodeTransform = node->mTransformation;
    aiVector3D scaling, translation;
    aiQuaternion rotation;
    nodeTransform.Decompose(scaling, rotation, translation);

    SubMeshTree tree;
    tree.rotation    = quat(rotation.w, rotation.x, rotation.y, rotation.z);
    tree.translation = vec3(translation.x, translation.y, translation.z);
    tree.scale       = vec3(scaling.x, scaling.y, scaling.z);

    tree.subMeshes.resize(node->mNumMeshes);
    for (int i = 0; i < node->mNumMeshes; i++) {
        tree.subMeshes[i] = node->mMeshes[i];
    }

    tree.childs.reserve(node->mNumChildren);
    for (int i = 0; i < node->mNumChildren; i++) {
        tree.childs.push_back(buildTree(node->mChildren[i]));
    }

    return tree;
}

SubMeshTree* Mesh::findCorrespondingTree(SubMeshTree* currentTree, int surfaceIdx)
{
    for (auto& meshIdx : currentTree->subMeshes) {
        if (meshIdx == surfaceIdx) {
            return currentTree;
        }
    }

    for (auto& child : currentTree->childs) {
        SubMeshTree* res = findCorrespondingTree(&child, surfaceIdx);
        if (res != nullptr)
            return res;
    }
    return nullptr;
}
} // namespace Galaxy