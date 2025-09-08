#include "Mesh.hpp"

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
    for (int i = 0; i < scene->mNumMeshes; i++) {
        extractSubMesh(scene, i);
    }
    return true;
}

bool Mesh::initGres(const std::string& path, uuid resourceID, const std::string& externalPath)
{
    m_resourceID   = resourceID;
    m_resourcePath = path;

    loadExtern(externalPath);

    return false;
}

bool Mesh::save()
{
    return ResourceSerializer::serialize(*this);
}

bool Mesh::load(YAML::Node& data)
{
    return ResourceDeserializer::deserialize(*this, data);
}

void Mesh::extractSubMesh(const aiScene* scene, int surface)
{
    std::vector<math::vec3> indexed_vertices;
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
        indexed_vertices.push_back(glm::vec3(vertex.x, vertex.y, vertex.z));
    }

    // Add texture coords
    if (mesh->HasTextureCoords(0)) {
        for (unsigned int j = 0; j < mesh->mNumVertices; ++j) {
            aiVector3D tex_coord = mesh->mTextureCoords[0][j];
            tex_coords.push_back(glm::vec2(tex_coord.x, tex_coord.y)); // Ajoute la coordonnée de texture
        }
    }

    currentSubMesh.vertices.resize(indexed_vertices.size());

    for (int i = 0; i < indexed_vertices.size(); ++i) {
        Vertex v;
        v.position = indexed_vertices[i];
        v.texCoord = tex_coords[i];

        currentSubMesh.vertices[i] = v;
    }
}

} // namespace Galaxy