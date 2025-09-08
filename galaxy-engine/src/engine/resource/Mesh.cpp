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

bool Mesh::loadGres(const std::string& file)
{
    return ResourceDeserializer::deserialize(*this, file);
}

bool Mesh::import(const std::string& file)
{
    GLX_CORE_ASSERT(loadExtern(file), "Failed to import");

    std::string path, extension;
    Project::extractExtension(file, path, extension);
    path += std::string(".gres");

    m_resourcePath = path;
    m_gltfPath     = file;

    if (save()) {
        // Succesfully created resource file
        Project::deletePath(ProjectPathTypes::RESOURCE, m_resourceID);
        m_resourceID = Project::registerNewPath(ProjectPathTypes::RESOURCE, m_resourcePath);
        return true;
    } else {
        return false;
    }
}

bool Mesh::load(const unsigned char* data, size_t size)
{
    GLX_CORE_ASSERT(false, "Not implemented");
    return false;
}

bool Mesh::save()
{
    return ResourceSerializer::serialize(*this, m_resourcePath);
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