#include "Mesh.hpp"

#include "engine/core/Log.hpp"
#include "engine/types/Math.hpp"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

#include <filesystem>

namespace Galaxy {

bool Mesh::load(const std::string& file)
{
    std::filesystem::path filePath(file.c_str());

    std::string fileExtension = filePath.extension().string();
    if (fileExtension == std::string(".gltf")) {
        return readGltf(file);
    } else if (fileExtension == std::string(".gres")) {
        // Resource file already exist, we can retrieve settings
        GLX_CORE_ERROR("gres loading not implemented yet!");
        return false;
    } else {
        GLX_CORE_ERROR("Unknown file format '{0}' for mesh loading", fileExtension);
        return false;
    }

    return true;
}

bool Mesh::readGltf(const std::string& filePath)
{
    const int meshIdx = 0;

    Assimp::Importer importer;

    const aiScene* scene = importer.ReadFile(filePath.c_str(),
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
            currentSubMesh.m_indices.push_back(face.mIndices[k]);
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

    currentSubMesh.m_vertices.resize(indexed_vertices.size());

    for (int i = 0; i < indexed_vertices.size(); ++i) {
        Vertex v;
        v.position = indexed_vertices[i];
        v.texCoord = tex_coords[i];

        currentSubMesh.m_vertices[i] = v;
    }
}

} // namespace Galaxy